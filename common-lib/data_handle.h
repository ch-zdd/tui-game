#ifndef DATA_HANDLE_H
#define DATA_HANDLE_H
 
#ifdef __cplusplus
extern "C"
{
#endif

#include <bits/stdint-uintn.h>
#include <bits/stdint-intn.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <complex.h>
#include <errno.h>

#define COLOR_BOLD_YELLOW  "\033[1;33m"    /* Bold Yellow */
#define COLOR_OFF              "\033[0m"       /* all off */

#define fm_info(format, ...) fprintf(stderr, COLOR_BOLD_YELLOW "<info> " COLOR_OFF "[%s:%u]" format "\n", __func__, __LINE__,  ##__VA_ARGS__)
#define fm_debug(format, ...) fprintf(stderr, COLOR_BOLD_YELLOW "<debug> " COLOR_OFF "[%s:%u]" format "\n", __func__, __LINE__,  ##__VA_ARGS__)
#define fm_warn(format, ...) fprintf(stderr, COLOR_BOLD_YELLOW "<warn> " COLOR_OFF "[%s:%u]" format "\n", __func__, __LINE__,  ##__VA_ARGS__)
#define fm_error(format, ...) fprintf(stderr, COLOR_BOLD_YELLOW "<error> " COLOR_OFF "[%s:%u]" format "\n", __func__, __LINE__,  ##__VA_ARGS__)

#define FILL_HEAD_SPACES(str, n_spaces) \
    do{                                 \
        str[n_spaces] = '\0';           \
        memset(str, ' ', n_spaces);     \
    }while(0)                           \

#ifndef cf_t
// cf_t definition
typedef _Complex float cf_t;
#endif

#ifndef cd_t
// cf_t definition
typedef _Complex double cd_t;
#endif

#define FM_OK 0
#define FM_ERROR -1
#define FM_TRUE 1
#define FM_FALSE 0
#define FM_FILLER_BIT 254 /*!< \brief Identifies a filler bit. */

#define FM_NORM(x) (creal(x)*creal(x) + cimag(x)*cimag(x))

typedef struct{
    void* data;
    uint32_t len;
}fm_data_t;

char* read_file_to_str(const char* file_name);

cd_t convert_complex_float_to_double(cf_t cf);
cf_t convert_complex_double_to_float(cf_t cf);

void print_int8(FILE* stream, int8_t* data, size_t len, int n_spaces);
void print_bit_uint8(FILE* stream, uint8_t* data, size_t len, int n_spaces);
void print_uint32(FILE* stream, uint32_t* data, size_t len, int n_spaces);
void print_float(FILE* stream, float* data, size_t len, int n_spaces);
void print_double(FILE* stream, double* data, size_t len, int n_spaces);
void print_hex(FILE* stream, uint8_t* data, size_t len, int n_spaces);
void print_complex(FILE* stream, cf_t* data, size_t len, int n_spaces);
void print_complex_double(FILE* stream, cd_t* data, size_t len, int n_spaces);
void print_llr(FILE* stream, int8_t* data, size_t len, int n_spaces);

void gen_bit_data(uint8_t* result, int result_num);
void gen_complex_data(cf_t* result, int result_num);
void gen_complex_double_data(cd_t* result, int result_num);

int parse_bit_data(const char* source, int source_is_file, uint8_t* result, int result_num);
int parse_complex_data(const char* source, int source_is_file, cf_t* result, int result_num);
int parse_complex_double_data(const char* source, int source_is_file, cd_t* result, int result_num);
int parse_float_data(const char* source, int source_is_file, float* result, int result_num);
int parse_double_data(const char* source, int source_is_file, double* result, int result_num);

uint32_t bit_diff(const uint8_t* x, const uint8_t* y, int nbits);

#ifdef __cplusplus
}
#endif

#endif