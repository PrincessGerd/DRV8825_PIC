#ifndef GCODE_PARSE_H
#define GCODE_PARSE_H
#include <stdint.h>
#include <stdbool.h>
//https://tsapps.nist.gov/publication/get_pdf.cfm?pub_id=823374

//3.4 Modal Groups [nist p20]
typedef enum {
    MODAL_GROUP_1 = 1,
    MODAL_GROUP_2 = 2,
    MODAL_GROUP_3 = 3,
    MODAL_GROUP_4 = 4,
    MODAL_GROUP_5 = 5,
    MODAL_GROUP_6 = 6,
    MODAL_GROUP_7 = 7,
    MODAL_GROUP_8 = 8,
    MODAL_GROUP_9 = 9,
    MODAL_GROUP_10 = 10,
    MODAL_GROUP_11 = 11,
    MODAL_GROUP_12 = 12,
    MODAL_GROUP_13 = 13,
} modal_grups_e;


#endif