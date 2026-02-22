#include "../system.h"
#include "../core/hw/inc/timer0_hw.h"
#include "../core/hw/inc/timer1_hw.h"
#include "../task_manager.h"
#include "../core/interrupts.h"
#include "../core/gpio.h"
#include <stdint.h>
#include <xc.h>

struct timer1_hw* system_timer;
struct timer0_hw* fast_tick_timer;
static uint16_t fast_tick_period = 0xFFFF-1;
static uint16_t sys_tick_period = 0xFFFF-1;

static timer0_config_t fast_tick_config = {
    .clk_src = TMR0_CLK_FOSC4,
    .prescaler = 0,
    .postscaler = 0,
    .async = false
};

static tmr1_config_t system_tick_config = {
    .clk_src = TMR1_CLK_FOSC4,
    .prescaler = TMR1_PRESCALER_1,
    .gate = {0},
    .gate_enable = false,
    .clk_sync = true
};

static void clock_initialize(void){
    OSCCON1 = (0 << _OSCCON1_NDIV_POSN)     // NDIV 1
        | (6 << _OSCCON1_NOSC_POSN);        // NOSC HFINTOSC
    OSCCON3 = (1 << _OSCCON3_SOSCPWR_POSN)  // SOSCPWR High power
        | (0 << _OSCCON3_CSWHOLD_POSN);     // CSWHOLD may proceed
    OSCEN = (0 << _OSCEN_EXTOEN_POSN)       // EXTOEN disabled
        | (0 << _OSCEN_HFOEN_POSN)          // HFOEN disabled
        | (0 << _OSCEN_MFOEN_POSN)          // MFOEN disabled
        | (0 << _OSCEN_LFOEN_POSN)          // LFOEN disabled
        | (1 << _OSCEN_SOSCEN_POSN)         // SOSCEN enabled
        | (0 << _OSCEN_ADOEN_POSN)          // ADOEN disabled
        | (0 << _OSCEN_PLLEN_POSN);         // PLLEN disabled
    OSCFRQ = (8 << _OSCFRQ_FRQ_POSN);       // FRQ 64_MHz
    OSCTUNE = (0 << _OSCTUNE_TUN_POSN);     // TUN 0x0
    ACTCON = (0 << _ACTCON_ACTEN_POSN)      // ACTEN disabled
        | (0 << _ACTCON_ACTUD_POSN);        // ACTUD enabled
    FSCMCON = (0 << _FSCMCON_FSCMFEV_POSN)  // FSCMFEV detected
        | (0 << _FSCMCON_FSCMFFI_POSN)      // FSCMFFI enabled
        | (0 << _FSCMCON_FSCMPEV_POSN)      // FSCMPEV detected
        | (0 << _FSCMCON_FSCMPFI_POSN)      // FSCMPFI enabled
        | (0 << _FSCMCON_FSCMSEV_POSN)      // FSCMSEV detected
        | (0 << _FSCMCON_FSCMSFI_POSN);     // FSCMSFI enabled
}

void system_init(void){
    timer0_create(&fast_tick_timer, false);
    timer1_create(&system_timer);
    timer1_init(system_timer, &system_tick_config);
    timer0_init(fast_tick_timer, &fast_tick_config);
}


void systick_config(uint32_t freq, system_timer_e timer){
    // loop over every prescaler value that timer2x can have
    const uint8_t ps_max = timer == FAST_TICK_TIMER ? 0xF : 0x4;
    for(uint8_t i = 0; i < ps_max; i++){
        // find the period required to achive the desired frequency
        uint8_t ps = (i << 1U);
        uint32_t period = SYSTEM_CLOCK_HZ / (ps * freq);
        if(period == 0){continue;} 
        // if it fits within the 16bit period register, then return 
        if((period-1) <= 0xFFFF){
            if(timer == FAST_TICK_TIMER){
                fast_tick_period = period;
                timer0_set_prescaler(fast_tick_timer, ps);
                timer0_set_counter(fast_tick_timer, (uint16_t)(0xFFFF - (period-1)));
                interrupt_enable(0x1D);
                interrupt_set_priority(0x1D,true);
                timer0_enable(fast_tick_timer);
                interrupt_enable_priority();
                enable_global_interrupts();
            }else{
                sys_tick_period = period;
                timer1_set_prescaler(system_timer, ps);
                timer1_set_counter(system_timer, (uint16_t)(0xFFFF - (period-1)));
            }
        }
    }
}

void __interrupt(high_priority) isr(void){
    if(interrupt_flag(0x1D)){
        interrupt_clear(0x1D);
        timer0_set_counter(fast_tick_timer, (uint16_t)(0xFFFF - (fast_tick_period-1)));
        fast_tick_handler();
    }
}

//TODO:
    // Add onStart
    // Add systic handler, and fast_systtick handler
    // find a place for the initialisation event for each module to be placed;