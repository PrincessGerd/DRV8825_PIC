#include "../system.h"
#include "../core/hw/inc/timer0_hw.h"
#include "../core/hw/inc/timer1_hw.h"

#include <stdint.h>
struct timer1_hw* system_timer;
struct timer0_hw* fast_tick_timer;

static timer0_config_t fast_tick_config = {
    .clk_src = TMR0_CLK_FOSC4,
    .prescaler = 0,
    .postscaler = 0,
    .async = true
};

static tmr1_config_t system_tick_config = {
    .clk_src = TMR1_CLK_FOSC4,
    .prescaler = TMR1_PRESCALER_1,
    .gate = {0},
    .gate_enable = false,
    .clk_sync = true
};

void system_init(void){
    timer0_create(&fast_tick_timer, true);
    timer1_create(&system_timer);
    timer1_init(system_timer, &system_tick_config);
    timer0_init(fast_tick_timer, &fast_tick_config);
}

void systick_config(uint32_t freq, system_timer_e timer){
    // loop over every prescaler value that timer2x can have
    const uint8_t ps_max = timer == FAST_TICK_TIMER ? 0xF : 0x4;
    for(uint8_t i = 0; i < ps_max; i++){
        // find the period required to achive the desired frequency
        uint8_t ps = (i << 1u);
        uint32_t period = SYSTEM_CLOCK_HZ / (ps * freq);
        if(period == 0){continue;} 
        // if it fits within the 16bit period register, then return 
        if((period-1) <= 0xFF){
            if(timer == FAST_TICK_TIMER){
                timer0_set_prescaler(fast_tick_timer, ps);
                timer0_set_counter(fast_tick_timer, (uint16_t)(0xFF - (period-1)));
            }else{
                timer1_set_prescaler(system_timer, ps);
                timer1_set_counter(system_timer, (uint16_t)(0xFF - (period-1)));
            }
        }
    }
}