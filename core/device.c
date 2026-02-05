#include "device.h"

void system_init(void){
    uint8_t idx = 0;
    device_t dev = init_table[idx];
    while(dev != 0 && idx < MAX_INIT_ENTRIES){
        idx++;
        dev->init(dev);
        dev = init_table[idx];
    }
}