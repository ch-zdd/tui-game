#ifndef TG_LOG_H
#define TG_LOG_H

#include <stdio.h>

#define log_info(format, ...)  tg_log(__FILE__, __LINE__, LOG_LEVEL_INFO,  format, ##__VA_ARGS__)
#define log_debug(format, ...) tg_log(__FILE__, __LINE__, LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define log_warn(format, ...)  tg_log(__FILE__, __LINE__, LOG_LEVEL_WARN,  format, ##__VA_ARGS__)
#define log_error(format, ...) tg_log(__FILE__, __LINE__, LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define log_text(format, ...)  tg_log_text(format"\n", ##__VA_ARGS__)
#define tg_print(format, ...)  fprintf(stderr, format"\n", ##__VA_ARGS__)

#define TG_INDETION(str, n_spaces) \
    do{                                 \
        str[n_spaces] = '\0';           \
        memset(str, ' ', n_spaces);     \
    }while(0)                           \

typedef enum{
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_ALL,
    LOG_LEVEL_NONE,
    LOG_LEVEL_MAX
}log_levle_t;

typedef struct{
    int level;
    FILE* fp;
    FILE* default_fp; //必须存在
}log_context_t;

void tg_log(const char* func_name, int line_number, int level, const char* format, ...);
void tg_log_text(const char* format, ...);
int log_init(void);
void log_final(void);
int set_log_file(const char* path);
int set_log_level(log_levle_t level);
const char* log_level_to_string(log_levle_t level);
log_levle_t log_string_to_level(const char* level);

void print_array_int(int* data, size_t len, int n_spaces);
#endif