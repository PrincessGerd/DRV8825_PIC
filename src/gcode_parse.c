#include "../inc/gcode_parse.h"
#include "../inc/math.h"
#include "../core/task_manager.h"
#include <string.h>

typedef enum{
    TOKEN_INTEGER = 0b01,
    TOKEN_FLOAT   = 0b10,
    TOKEN_UNKNOWN = 0b00,
    TOKEN_EOL,           // end of line
} TokenType_e;

typedef struct {
    TokenType_e type;
    int value;
    char letter;
} Token_t;


static inline bool is_letter_AZ(char* c){
    return ('A' <= *c && *c <='Z');
}

static inline bool is_digit(char *c){
    return ('0' <= c && c <= '9');
}


TokenType_e get_number_type(const char* c, uint8_t* num_len_o, uint8_t* len_to_dot_o){
    uint8_t i = 0;
    bool is_negative = false;
    bool has_dot = false;
    TokenType_e token = TOKEN_UNKNOWN;
    
    if(c[i] == '-'){
        is_negative = true; 
        i++;
    }

    char* ptr = &c[i];
    if(is_digit(ptr)){
        token |= TOKEN_INTEGER;
    }else{
        return token;
    }

    for(; (*ptr != '\0' || *ptr != ' '); ptr++){
        if(is_digit(ptr)){
            token |= TOKEN_INTEGER;
        }
        if(*ptr == '.'){
            token = has_dot 
                ? TOKEN_UNKNOWN 
                : (token | TOKEN_FLOAT);
            has_dot = true;
            *len_to_dot_o = is_negative 
                ? (uint8_t)(ptr - (c+1))
                : (uint8_t)(ptr - c);
        }
    }
    num_len_o = is_negative 
        ? (uint8_t)(ptr - (c+1))
        : (uint8_t)(ptr - c);
    return token;
}


static Token_t next_code(char* line, uint8_t* count, uint16_t* val_o, char* letter_o){
    if(line[*count] == '\0'){
        return false; // done
    }
    if(!is_letter_AZ(line[*count])){
        return false; // error, expected letter
    }
    *letter_o = line[*count];
    *count++;


    uint8_t num_len = 0;
    uint8_t len_to_dot = 0;
    char tmp_num[16] = {0};
    int16_t temp_val = 0;
    TokenType_e type = get_number_type(&line[*count], &num_len, &len_to_dot);
    switch (type) {
        case TOKEN_INTEGER:{
            char* tmp_num = 0;
            memmove(tmp_num, &line[*count], num_len);
            *val_o = (int16_t)(atoi(tmp_num));
        } break;
        case TOKEN_FLOAT:{
            memmove(tmp_num, &line[*count], len_to_dot);
            temp_val = (fp15_t)(atoi(tmp_num) << Q15_BITS);
            memmove(tmp_num, &line[*count+len_to_dot], (num_len - len_to_dot));
            //TODO: FIX this. wrong. only a estimate
            temp_val |= (fp15_t)((atoi(tmp_num) >> ((num_len - len_to_dot) - 1)));
            *val_o = temp_val;
        } break;
    default: // UNKNOWN_TOKEN
        return false;
    }
    return true;
}

void parse_line(char* line){    
    event_t next_evt = {0};
    uint8_t count = 0; 
    int16_t curr_val = 0;
    char curr_code;
    uint8_t modal_group = 0;
    while(next_code(line, count, &curr_val, &curr_code)){
        switch (curr_code) {
            case 'G':{
                switch(curr_val){
                    case 0: case 1: case 2: 
                }
            } break;
            case 'M':{
        
            } break;
            case 'F': {

            } break;
            default:
                break;
        }
    }
}