#include "../inc/stepper.h"
#include "stepper_device.h"
// core
#include "../core/types.h"
#include "../core/task_manager.h"
#include "../core/gpio.h"
//
#include <stdint.h>
#include <stdbool.h>

typedef enum{
    STEPPER_STATE_ACCELERATE,
    STEPPER_STATE_CONSTANT_SPEED,
    STEPPER_STATE_DEACCELERATE,
    STEPPER_STATE_IDLE
}motion_states_e;

struct stepper{
    task_t super;
    fast_tickEvt_t fte;
    motion_states_e motion_state;
    stepper_device_t    device;

    uint32_t tick_freq;         // frequency of timer 
    uint32_t phase_acc;         // phase accumulator
    uint32_t phase_inc;         // phase increment

    uint32_t step_rate;         // current speed (steps/sec)
    uint32_t target_rate;       // desired speed (steps/sec)
    uint32_t accel_per_step;    // acceleration increase per step

    uint32_t decel_steps;       // steps required to decelarate
    uint32_t steps_remaining;   // steps to done
    uint8_t dir_state;          // current direction
    uint8_t step_pin;           // step pin
};

static void stepper_init(struct stepper* const self, event_t const* const ie);
static void stepper_dispatch(struct stepper* const self, event_t* const event);

void stepper_create(task_t const* self){
    static struct stepper stepper_inst;
    task_t * const stepper = &stepper_inst.super;
    create_stepper_device(&stepper_inst.device);
    task_create(stepper,
        (event_handler_t)&stepper_init,
        (event_handler_t)&stepper_dispatch);
    self = stepper;
}

static void stepper_init(struct stepper* const self, event_t const* const ie){
    (*(self->device))->init(self->device, 0);
}

static void stepper_dispatch(struct stepper* const self, event_t* const event){
    switch (event->signal) {
    ////////////////////////////////////////////////////////
        case STEPPER_WORK_SIG:{
            stepper_workEvt_t* evt = 
                container_of(event, struct stepper_workEvt, super);
            // set the given values for step and desired speed for this event
            self->steps_remaining = evt->steps; 
            self->target_rate = evt->speed;     
            self->step_rate   = 0;

            // calculate the acceleration and deceleration steps and step-rate needed
            uint32_t accel_steps = (self->target_rate * self->target_rate) / (2*evt->accel);
            self->decel_steps = accel_steps;
            self->accel_per_step =  self->target_rate / accel_steps;

            // set initial values for phase increment
            self->phase_acc = 0;
            self->phase_inc = (self->accel_per_step << 16) / self->tick_freq;
            
            // set state for the fast tick state machine, and post the event
            self->motion_state = STEPPER_STATE_ACCELERATE;
            fast_tick_event_create(&self->fte, &self->super, FAST_TICK_TIMEOUT_SIG);
            fast_tick_event_arm(&self->fte, self->phase_acc, self->phase_inc);

        } break;
    ////////////////////////////////////////////////////////
        case FAST_TICK_TIMEOUT_SIG:{
            self->phase_acc += self->phase_inc;
            // when the phase acumulator reaches 1.0 in fixed point q16  (should be changed, but even fixedpoint is easier)
            if(self->phase_acc >= (1UL <<16)){
                self->phase_acc = 0;
                // each fast tick is the half period of the pulse
                gpio_toggle(self->step_pin);
                if(self->steps_remaining == 0) {
                    self->motion_state = STEPPER_STATE_IDLE;
                    task_signal_post(&self->super, STEPPER_DONE_SIG);
                    break;
                }
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
                self->phase_inc = (self->step_rate << 16) / self->tick_freq;
            }
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