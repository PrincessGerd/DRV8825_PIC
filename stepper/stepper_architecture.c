#include "stepper.h"
#include "stepper_device.h"
// core
#include "../core/types.h"
#include "../core/task_manager.h"
#include "../core/gpio.h"
//
#include <stdint.h>
#include <stdbool.h>
#include <xc.h>

typedef enum{
    STEPPER_STATE_ACCELERATE,
    STEPPER_STATE_CONSTANT_SPEED,
    STEPPER_STATE_DEACCELERATE,
    STEPPER_STATE_IDLE
}motion_states_e;

struct stepper{
    task_t super;
    timed_event_t fte;
    event_t       evt;
    motion_states_e motion_state;
    stepper_device_t    device;

    uint16_t tick_freq;         // frequency of timer 
    uint16_t phase_acc;         // phase accumulator
    uint16_t phase_inc;         // phase increment

    uint16_t step_rate;         // current speed (steps/sec)
    uint16_t target_rate;       // desired speed (steps/sec)
    uint16_t accel_per_step;    // acceleration increase per step

    uint16_t decel_steps;       // steps required to decelarate
    uint32_t steps_remaining;   // steps to done
    uint8_t dir_state;          // current direction
    uint8_t step_pin;           // step pin
};

static void stepper_init(task_t* const super, event_t const* const ie);
static void stepper_dispatch(task_t* const super, event_t* const e);

void stepper_create(task_t** self){
    static struct stepper stepper_inst;
    create_stepper_device(&stepper_inst.device);
    task_create(&stepper_inst.super,
        (event_handler_t)&stepper_init,
        (event_handler_t)&stepper_dispatch);
    *self = &stepper_inst.super;
}

static void stepper_init(task_t* const super, event_t const* const ie){
    struct stepper* self = container_of(super, struct stepper, super); 
    struct stepper_initEvt* initial_event = (struct stepper_initEvt*)ie;
    (*(self->device))->init(self->device, &initial_event->pins);
    self->step_pin = initial_event->pins.step_pin;
}

static void stepper_dispatch(task_t* const super, event_t* const e){
    struct stepper* self = container_of(super, struct stepper, super); 
    switch (e->signal) {
    ////////////////////////////////////////////////////////
        case STEPPER_WORK_SIG:{
            stepper_workEvt_t* evt = (stepper_workEvt_t*)e;
            // set the given values for step and desired speed for this event
            uint32_t evt_steps = evt->steps;
            uint16_t evt_rate = evt->speed;
            uint16_t evt_accel = evt->accel;

            self->steps_remaining = 0xFFFFFF00;//evt_steps;
            self->target_rate = 0xFFFE;//evt_rate;     
            self->step_rate   = 0;

            // calculate the acceleration and deceleration steps and step-rate needed
            uint32_t temp = (uint32_t)self->target_rate * (uint32_t)self->target_rate;
            temp /= (uint32_t)(2u * evt_accel);
            uint16_t accel_steps = (uint16_t)temp;

            // set initial values for phase increment
            self->phase_acc = 0;
            uint32_t temp2 = (uint32_t)self->accel_per_step * 65536u;
            temp2 /= self->tick_freq;
            self->phase_inc = (uint16_t)temp2;

            // set state for the fast tick state machine, and post the event
            self->motion_state = STEPPER_STATE_ACCELERATE;
            fast_tick_event_create(&self->fte, super, FAST_TICK_TIMEOUT_SIG);
            fast_tick_event_arm(&self->fte, self->phase_acc, self->phase_inc);
        } break;
        ////////////////////////////////////////////////////////
        case FAST_TICK_TIMEOUT_SIG:{
            // each fast tick is the half period of the pulse
            //gpio_toggle(self->step_pin);
            if(self->steps_remaining == 0) {
                self->motion_state = STEPPER_STATE_IDLE;
                self->evt.signal = STEPPER_DONE_SIG;
                gpio_clear(self->step_pin);
                fast_tick_event_disarm(&self->fte);
                task_event_post(super, &self->evt);
                break;
            }
            gpio_toggle(RC_4);
            self->steps_remaining--;
            switch (self->motion_state) {
                // increment the step rate until target is reached or deceleration is needed
                case STEPPER_STATE_ACCELERATE:{
                    if(self->steps_remaining <= self->decel_steps){
                        self->motion_state = STEPPER_STATE_DEACCELERATE;
                    }
                    self->step_rate += self->accel_per_step;
                    if(self->step_rate >= self->target_rate){
                        self->step_rate = self->target_rate;
                        self->motion_state = STEPPER_STATE_CONSTANT_SPEED;
                    }
                } break;
                // continue with same step rate 
                case STEPPER_STATE_CONSTANT_SPEED:{
                    if(self->steps_remaining <= self->decel_steps){
                        self->motion_state = STEPPER_STATE_DEACCELERATE;
                    }
                }break;
                // decrement the step rate until stoped
                case STEPPER_STATE_DEACCELERATE:{
                    if(self->step_rate > self->accel_per_step){
                        self->step_rate -= self->accel_per_step;
                    }else{
                        self->step_rate = 0;
                    }
                }break;
                default:    // idle state 
                    break;
            }
            // update phase increment
            uint32_t temp2 = (uint32_t)self->accel_per_step * 65536u;
            temp2 /= self->tick_freq;
            self->phase_inc = (uint16_t)temp2;
            fast_tick_event_arm(&self->fte, self->phase_acc, self->phase_inc);
        } break;

        ////////////////////////////////////////////////////////
        case STEPPER_DONE_SIG:{
            gpio_clear(self->step_pin);
            self->phase_acc = 0;
            self->phase_inc = 0;
            self->target_rate = 0;
            self->decel_steps = 0;
            self->accel_per_step = 0;
            self->steps_remaining = 0;
        ////////////////////////////////////////////////////////
        }break;
        default:
            break;
    }
}