#ifndef COMMON_H
#define COMMON_H

#include <bits/stdint-uintn.h>
#include <bits/stdint-intn.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#include "log.h"

#define COLOR_BOLD_YELLOW  "\033[1;33m"    /* Bold Yellow */
#define PRINT_COLOR_GREEEN "\033[1;40;32m" /* green */
#define COLOR_BOLD_RED     "\033[1;40;31m" 
#define COLOR_OFF          "\033[0m"       /* all off */


#define CHECK_FMT(a, b)	__attribute__((format(printf, a, b)))

#define TK_ABORT() \
    do{\
        log_error("The current error prevents the program from starting, abort");\
        exit(1);\
    }while(0)
    

#define PTR_CONVERT_TYPE(p, type) (*((type*)(p)))
#define TK_CHANCE_APPEAR(chance) ( ((float)(rand()%100)) < chance*100)
#define TK_MAX(a,b) (a>b?a:b)
#define TK_MIN(a,b) (a<b?a:b)
#define IS_CH_NUM(ch) (ch>='0' && ch<='9')

#define MAX_PATH_LEN 256

#define TK_NOT_FOUND -2
#define TK_OK 0
#define TK_ERROR -1

typedef enum{
    type_int,
    type_int64,
    type_float,
    type_char,
    type_bool,
    type_string,
    type_array,
    type_none
}value_type_t;

#define MAX_ELEMENTS_SIZE 1024
#define MAX_ELEMENTS_NUM 1024

typedef struct{
    char* str;
    uint32_t len;
}str_t;

#endif