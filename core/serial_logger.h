#ifndef SERIAL_LOGGER
#define SERIAL_LOGGER

#include "../core/hw/inc/uart_hw.h"
#include <stdbool.h>

    bool init_loggger();
    void trace_impl(const char* file, int line, const char* format, ...);
    void assert_trace(void);

    //#define BREAKPOINT __debug_break();
    //only  %i, %d, %x, %c, %s  is supported, and all numerical formats is represented as hex for simplicity
    #define SLOG_ASSERT(x, ...) { if(!(x)) { assert_trace(); /*BREAKPOINT*/} }
    #define SLOG_TRACE(...) trace_impl(__FILE__, __LINE__, __VA_ARGS__)
#endif