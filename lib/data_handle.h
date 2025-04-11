#ifndef _DATA_HANDLE_H_
#define _DATA_HANDLE_H_

#include <stdbool.h>
#include "common.h"

#define MAX_ELEMENTS_LEN 1024

#define FILL_LINE_SPACES(str, n_spaces) \
    do{                                 \
        str[n_spaces] = '\0';           \
        memset(str, ' ', n_spaces);     \
    }while(0)                           \

typedef struct{
    int max_length;
    int number;
}element_para_t;

int count_char(const char* str, char ch);
int count_digits(int number);

char* file_to_string(const char* file_name);

char* trim_chars(char* str, const char* chars);
char* trim(char *str);
void remove_quotation_marks(char* str);
bool is_all_digits(const char *str);
bool is_float_num(const char *str);

element_para_t get_elements_para(char* str, char delimiter);
char* pasrse_array(const char* data, char* ele, size_t ele_max_size, const char* delimiter);
int parse_key_value(const char* data, const char* key, char* const value, size_t value_max_len);
const char*  parse_cfg_label(const char* data, const char* attr_name, char* attr_context);

void comment_remove(char* cfg_context);
char* strchr2(const char* str, char c);

#endif