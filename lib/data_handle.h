#ifndef _DATA_HANDLE_H_
#define _DATA_HANDLE_H_

#include <stdbool.h>
#include "common.h"

#define FILL_LINE_SPACES(str, n_spaces) \
    do{                                 \
        str[n_spaces] = '\0';           \
        memset(str, ' ', n_spaces);     \
    }while(0)                           \

typedef struct{
    int max_length;
    int number;
}element_para_t;

void* tk_malloc(size_t size);
void tk_free(void* ptr);

char* file_to_string(const char* file_name);
void trim(char *str);
bool is_all_digits(const char *str);

element_para_t get_elements_para(char* str, char delimiter);
int parse_array_to_string(const char* data, char** ele, int ele_max_size, char delimiter);
int parse_key_value(const char* data, const char* key, data_t* const value, value_type_t value_type);
const char*  parse_cfg_label(const char* data, const char* attr_name, char* attr_context);

void cfg_context_comment_remove(char* cfg_context);

#endif