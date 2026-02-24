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
static uint16_t sys_tick_period = 0xFFFF-1;
static uint32_t sys_tick_frequency = 0;

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
    timer1_create(&system_timer);
    timer1_init(system_timer, &system_tick_config);
    interrupt_enable_priority();
    enable_global_interrupts();
}

void systick_config(uint32_t freq, system_timer_e timer){
    disable_global_interrupts();
    // loop over every prescaler value that timer2x can have
    const uint8_t ps_max = timer == FAST_TICK_TIMER ? 0xF : 0x4;
    for(uint8_t i = 0; i < ps_max; i++){
        // find the period required to achive the desired frequency
        uint8_t ps = (i << 1U);
        uint32_t period = SYSTEM_CLOCK_HZ / (ps * freq);
        if(period == 0){continue;} 
        
        // if it fits within the 16bit period register, then return 
        if((period-1) <= 0xFFFF){
            sys_tick_frequency = SYSTEM_CLOCK_HZ / (ps*period);
            sys_tick_period = (uint16_t)period;
            timer1_set_prescaler(system_timer, ps);
            timer1_set_counter(system_timer, (uint16_t)(0xFFFF - (period-1)));
        }
    }
    enable_global_interrupts();
}