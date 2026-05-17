#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#define DP_S_DEFAULT 0
#define DP_S_CONV    6
#define DP_S_DONE    7


static void __dopr_outch(char *buffer, unsigned int *currlen, unsigned int maxlen, char c) {
        if (*currlen < maxlen) {
                buffer[(*currlen)] = c;
        }
        (*currlen)++;
}

static void fmtint(char* buffer, unsigned int *currlen, unsigned int maxlen, uint32_t value) {
    uint8_t place = 0;
    char convert[20];
    unsigned int uvalue = value;
    do{
        convert[place++] = "0123456789ABCDEF"[uvalue % 16];
        uvalue = uvalue  >> 4;
    } while(uvalue && (place < 20));
    if (place == 20) place--;
    convert[place] = 0;
}

static unsigned int __dopr(char *buffer, unsigned int maxlen, const char *format, va_list args) {
    char ch;
    int32_t value;
    uint8_t state = DP_S_DEFAULT;
    unsigned int currlen = 0;

    ch = *format++;
    
    while (state != DP_S_DONE) {
            if (ch == '\0') 
                    state = DP_S_DONE;
            switch(state) {
            case DP_S_DEFAULT:
                    if (ch == '%'){
                        state = DP_S_CONV;
                    } else {
                        __dopr_outch (buffer, &currlen, maxlen, ch);
                    }
                    ch = *format++;
                    break;
            case DP_S_CONV:
                    switch (ch) {
                    case 'd':
                    case 'i':
                    case 'x':
                        value = va_arg (args, uint32_t);
                        fmtint (buffer, &currlen, maxlen, value);
                        break;
                    case 'c':
                        __dopr_outch (buffer, &currlen, maxlen, va_arg (args, uint8_t));
                        break;
                    case 's':
                        __dopr_outch (buffer, &currlen, maxlen, va_arg (args, char *));
                        break;
                    case '%':
                        __dopr_outch (buffer, &currlen, maxlen, ch);
                        break;
                    default:
                        break;
                    }
                ch = *format++;
                state = DP_S_DEFAULT;
                break;
            case DP_S_DONE:
                    break;
            }
    }
    if (maxlen != 0) {
            if (currlen < maxlen - 1) 
                    buffer[currlen] = '\0';
            else if (maxlen > 0) 
                    buffer[maxlen - 1] = '\0';
    }
    return currlen;
}

int __vsnprintf (char *str, unsigned int count, const char *fmt, va_list args) {
    return __dopr(str, count, fmt, args);
}