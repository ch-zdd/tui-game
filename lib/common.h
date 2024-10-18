#ifndef COMMON_H
#define COMMON_H

#include <bits/stdint-uintn.h>
#include <bits/stdint-intn.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include "log.h"

#define COLOR_BOLD_YELLOW  "\033[1;33m"    /* Bold Yellow */
#define PRINT_COLOR_GREEEN "\033[1;40;32m" /* green */
#define COLOR_OFF          "\033[0m"       /* all off */

#define TK_ABORT() \
    do{\
        log_error("The current error prevents the program from starting, abort");\
        exit(1);\
    }while(0)
    

#define PTR2TYPE(p, type) (*((type*)(p)))

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

typedef value_type_t data_type_t;

typedef struct{
    union{
        char* str; //字符串必须以 \0 结尾
        bool boolean;
        char ch;
        int32_t intger;
        int64_t intger64;
        float flt;
        char **array; //任何数组类型的元素都暂时视为字符串,必须以 \0 结尾
    };
    data_type_t type_t;
#define MAX_ELEMENTS_SIZE 1024
#define MAX_ELEMENTS_NUM 1024
    uint32_t elements_size; //元素大小，sizeof(类型)得出，str和array都类型都应该为数组类型
    uint32_t elements_num; //元素个数，除array外均为1，字符串数组类型为字符串个数
}data_t;

typedef struct{
    char* str;
    uint32_t len;
}str_t;

#endif