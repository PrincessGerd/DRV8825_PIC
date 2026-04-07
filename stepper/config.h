#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>


struct config{
    uint16_t steps_per_mm;
    uint8_t micro_steps;
    uint16_t pulse_us;
};

static struct config gconfig = {0};

#endif