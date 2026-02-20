#include "hwlock.h"
#include "../interrupts.h"
#include "../task_manager.h"

bool hwlock_aquire(hwlock_t* lck, hw_owner_t owner){
    bool gie_state = INTCON0bits.GIE;
    disable_global_interrupts();
    if(lck->owner != HW_OWNER_NONE || lck->owner == HW_OWNER_INVALID){
        if(gie_state)
            enable_global_interrupts();
        return false;
    }
    lck->owner = owner;
    if(gie_state)
        enable_global_interrupts();
    return true;
}

bool hwlock_release(hwlock_t* lck, hw_owner_t owner){
    bool gie_state = INTCON0bits.GIE;
    disable_global_interrupts();
    if(lck->owner != owner || lck->owner == HW_OWNER_INVALID){
        return false;
    }
    lck->owner = HW_OWNER_NONE; 
    if(gie_state)
        enable_global_interrupts();
    return true;
}