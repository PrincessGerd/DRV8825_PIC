#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "../serial_logger.h"
#include "../core/hw/inc/uart_hw.h"
#include "../core/interrupts.h"

#define MAX_STRING_LEN 255
#define ASSERT_STRING_MAX_LEN (24u + 6u + 1u)


static const uart_hw_config_t config = {
    .baud = 9600,
    .mode = UART_7BIT_MODE
};

bool init_loggger(){
    uart_hw_iginit(&config);
}

//static inline __attribute__((always_inline)) void get_program_counter(uint24_t* pc_out){
//    // The program counter isn't accessable, so using the assembly instruction
//    // CALL pushes the program counter onto the stack where it is acceseble
//    uint8_t pcl, pch, pcu; 
//    __asm (
//        "call __get_pc_label \n"
//        "__get_pc_label:     \n"
//        "movff TOSL, %0      \n"
//        "movff TOSH, %1      \n"
//        "movff TOSU, %2      \n"
//        : "=r"(pcl), "=r"(pch), "=r"(pcu)
//    );
//    *pc_out = (((uint24_t)pcu << 16) | ((uint16_t)pch << 8) | pcl);
//}


typedef enum {
    STATE_DEFAULT = 0,
    STATE_FORMAT,
    STATE_DONE
} vsnprintf_state_t;

static void _outch(char *buffer, unsigned int *currlen, unsigned int maxlen, char c) {
    if (*currlen < maxlen) {
            buffer[(*currlen)] = c;
    }
    (*currlen)++;
}

static void _outstr(char *buffer, unsigned int *currlen, unsigned int maxlen, const char *src, unsigned int len) {
    if (*currlen < maxlen) {
        unsigned int remaining = maxlen - *currlen;
        if (len > remaining) {
            len = remaining;
        }
        memcpy(buffer + *currlen, src, len);
    }
    *currlen += len;
}

static void format_hex(char* buffer, unsigned int *currlen, unsigned int maxlen, uint32_t value) {
    unsigned int uvalue = value;
    char convert[20];
    char* ptr = &convert[sizeof(convert)];
    do {
        *--ptr ="0123456789ABCDEF"[uvalue & 0x0F]; // fast mod of 16
        uvalue >>= 4; // divide by 16
    } while (uvalue > 0);
    unsigned int len = (&convert[sizeof(convert)] - ptr);
    _outstr(buffer, currlen, maxlen, ptr, len);
}

static unsigned int __vsnprintf(char *buffer, unsigned int maxlen, const char *format, va_list args) {
    char ch;
    char* strvalue;
    unsigned int value;
    
    vsnprintf_state_t state = STATE_DEFAULT;
    unsigned int currlen = 0;
    ch = *format++;

    while (state != STATE_DONE) {
            if (ch == '\0') { state = STATE_DONE; }
            switch(state) {
            case STATE_DEFAULT:
                    if (ch == '%'){
                        state = STATE_FORMAT;
                    } else {
                        _outch(buffer, &currlen, maxlen, ch);
                    }
                    ch = *format++;
                    break;
            case STATE_FORMAT:
                switch (ch) {
                    case 'i': case 'd': case 'x':
                        value = va_arg (args, unsigned int);
                        format_hex(buffer, &currlen, maxlen, value);
                        break;
                    case 'c':
                        _outch(buffer, &currlen, maxlen, va_arg (args, int));
                        break;
                    case 's':
                        strvalue = va_arg (args, char *);
                        unsigned int strln = 0;
                        for (;strvalue[strln]; ++strln) {}
                        _outstr(buffer, &currlen, maxlen, strvalue, strln);
                        break;
                    }
                ch = *format++;
                state = STATE_DEFAULT;
                break;
            case STATE_DONE:
                break;
            }
    }
    if (maxlen != 0) {
        if (currlen < maxlen - 1){
            buffer[currlen] = '\0';
        } else if (maxlen > 0) {
            buffer[maxlen - 1] = '\0';
        }
    }
    return currlen;
}


unsigned int string_format(char* dest, unsigned int strlen, const char* format, ...) {
    unsigned int strln = 0;
    if (dest) {
        va_list arg_ptr;
        va_start(arg_ptr, format);
        strln = __vsnprintf(dest, strlen, format, arg_ptr);
        va_end(arg_ptr);
    }
    return strln;
}

// polling trace for assertion. no message. sets breakpoint
void assert_trace(void){
    // disable interrupts to make sure this executes
    disable_global_interrupts();
    
    // construct the assert message
    char assert_string[ASSERT_STRING_MAX_LEN];
    //uint24_t pc = 0;
    //get_program_counter(&pc);
    //string_format(assert_string, ASSERT_STRING_MAX_LEN, "ASSERT 0x%x\n", pc);
    string_format(assert_string, ASSERT_STRING_MAX_LEN, "ASSERT %s:%d\n", __FILE__, __LINE__);
    uint8_t bytes_written = 0;

    // send message in blocking mode 
    uart_hw_TX_enable();
    uart_hw_write_polling((uint8_t*)assert_string, ASSERT_STRING_MAX_LEN, &bytes_written);
}

// simple trace, non blocking
void trace_impl(const char* file, int line, const char* format, ...) {
    char msg[MAX_STRING_LEN];
    char out[MAX_STRING_LEN];

    va_list args;
    va_start(args, format);
    __vsnprintf(msg, MAX_STRING_LEN, format, args);
    va_end(args);

    unsigned int strln = 0;
    strln = string_format(out, MAX_STRING_LEN, "%s:%d  %s\n", file, line, msg);
    // send message none blocking
    uint16_t bytes_written = 0;
    uart_hw_write(out, strln);
}