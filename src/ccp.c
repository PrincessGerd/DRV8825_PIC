
#include <xc.h>
#include "ccp.h"


void ccp_init(void* self) {
    ccp_instance_t* ccp = (ccp_instance_t*)self;
    CLEAR_BIT(CCPXCON(ccp->module_num), CCP_EN_SHIFT);
    SET_MODE(ccp->config.mode, ccp->module_num);
    if(ccp->config.mode == CCP_MODE_PWM){
        SET_ALIGNMENT(ccp->config.align, ccp->module_num);
    }
    //SET_TRIGGER(ccp->config.trigger, ccp->module_num); excluded for now
}

void ccp_enable(void* self) {
    ccp_instance_t* _self = (ccp_instance_t*)self;
    SET_BIT(CCPXCON(_self->module_num),CCP_EN_SHIFT);
}

void ccp_disable(void* self) {
    ccp_instance_t* _self = (ccp_instance_t*)self;
    CLEAR_BIT(CCPXCON(_self->module_num),CCP_EN_SHIFT);
}

void ccp_enable_interrupt(void* self) {PIE4bits.CCP1IE = 1;}
void ccp_disable_interrupt(void* self) {PIE4bits.CCP1IE = 0;}
void ccp_set_callback(void* self,ccp_callback_t callback_func) {
    ccp_instance_t* ccp = (ccp_instance_t*)self;
    if(callback_func != NULL) {
        ccp_callbacks[ccp->module_num] = callback_func;
    }
}

void ccp_set_register(void* self, uint16_t value){
    ccp_instance_t* _self = (ccp_instance_t*)self;
    *(CCPRXH(_self->module_num)) = (value >> 8) & 0xFF;
    *(CCPRXL(_self->module_num)) = value & 0xFF;
}

//void __interrupt() ccp1_isr(void){
//    if(PIR4bits.CCP1IF == 1){
//       PIR4bits.CCP1IF = 0;
//       TRISCbits.TRISC4 = 1; // enable PWM pin LAST
//       CLEAR_BIT(CCPXCON(0),CCP_EN_SHIFT);
//       if(ccp_callbacks[0] != 0){
//           ccp_callbacks[0]();
//       }
//    }
//}
