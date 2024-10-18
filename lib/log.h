#ifndef TK_LOG_H
#define TK_LOG_H

#include <stdio.h>

#define log_info(format, ...)  log_with_info(__FILE__, __LINE__, LOG_LEVEL_INFO,  format, ##__VA_ARGS__)
#define log_debug(format, ...) log_with_info(__FILE__, __LINE__, LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define log_warn(format, ...)  log_with_info(__FILE__, __LINE__, LOG_LEVEL_WARN,  format, ##__VA_ARGS__)
#define log_error(format, ...) log_with_info(__FILE__, __LINE__, LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define log_text(format, ...)  log_with_info(__FILE__, __LINE__, LOG_LEVEL_TEXT, format, ##__VA_ARGS__)
#define tk_print(format, ...)  fprintf(stderr, format"\n", ##__VA_ARGS__)

typedef enum{
    LOG_LEVEL_TEXT,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_ALL,
    LOG_LEVEL_NONE,
    LOG_LEVEL_MAX
}log_levle_t;

void log_with_info(const char* func_name, int line_number, int level, const char* format, ...);
void log_init(void);
void log_final(void);
int set_log_file(const char* path);
int set_log_level(log_levle_t level);
const char* log_level_to_string(log_levle_t level);
log_levle_t log_string_to_level(const char* level);
#endif