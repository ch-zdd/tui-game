#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "log.h"

#define DEFAULT_LOG_PATH "/var/log/tk.log"
#define MAX_LOG_SIZE 1024

static log_context_t log_context = {
    .fp = NULL,
    .level = LOG_LEVEL_MAX
};

/*
功能： 获取当前日期和时间，用于表格的时间戳
*/
char* tk_time_stamp(void)
{
    time_t rawtime;
    struct tm *info;
    static char date_time[25];

    time( &rawtime );
    info = localtime( &rawtime );
 
    strftime(date_time, 25, "%H:%M:%S", info);
    return date_time;
}

// 定义一个辅助函数，用于实现带函数名和行号的日志记录  
void tk_log(const char *file, int line, int level, const char *format, ...) 
{
    char buffer[MAX_LOG_SIZE];
    char* print_buf = buffer;
    int level_str_len = 30;
    int pos_info_len = 256;
    int info_len = MAX_LOG_SIZE-pos_info_len-level_str_len;
    FILE* log_file = log_context.fp;
    va_list args;

    if(log_file == NULL){
        log_file = stderr;
    }

    if(level > log_context.level){
        return;
    }
    
    print_buf += snprintf(print_buf, level_str_len, "%s <%-5s> | ", tk_time_stamp(), log_level_to_string(level));

    va_start(args, format);  
    // 首先，我们打印出日志的主要内容（采用vfprintf函数处理可变参数）  
    print_buf += vsnprintf(print_buf, info_len, format, args);  
    va_end(args);

    // 然后，在同一行末尾追加函数名和行号信息  
    // 注意：为了保持格式整洁，我们在函数名和行号前添加了分隔符，并进行了换行  
    snprintf(print_buf, pos_info_len, " (%s:%d)\n", file, line);

    fprintf(log_file, "%s", buffer);
    fflush(log_file);
}

void tk_log_text(const char* format, ...)
{
    FILE* log_file = log_context.fp;
    va_list args;

    if(log_file == NULL){
        log_file = stderr;
    }

    if(log_context.level < LOG_LEVEL_INFO){
        return;
    }
    
    va_start(args, format);  
        // 首先，我们打印出日志的主要内容（采用vfprintf函数处理可变参数）  
    vfprintf(log_file, format, args);  
    va_end(args);

    fflush(log_file);
}

int log_init(void)
{

    if(log_context.level == LOG_LEVEL_MAX){
        log_context.level = LOG_LEVEL_INFO;
    }

    if(log_context.fp == NULL){
        if(TK_OK != set_log_file(DEFAULT_LOG_PATH)){
            log_context.fp = stderr;
        }
    }

    return TK_OK;
}

void log_final(void)
{
    if(log_context.fp == NULL || log_context.fp == stderr){
        return;
    }

    fflush(log_context.fp);
    fclose(log_context.fp);
}

int set_log_file(const char* path)
{
    FILE* log_file = fopen(path, "w");
    if(log_file == NULL){
        log_error("open log file [%s] failed!, %s", path, strerror(errno));
        return TK_ERROR;
    }

    log_context.fp = log_file;
    return TK_OK;
}

int set_log_level(log_levle_t level)
{
    if(level >= LOG_LEVEL_MAX){
        log_error("invalid log level %d", level);
        return TK_ERROR;
    }

    log_context.level = level;
    return TK_OK;
}

const char* log_level_to_string(log_levle_t level)
{
    switch(level){
        case LOG_LEVEL_DEBUG:
            return "DEBUG";
        case LOG_LEVEL_ERROR:
            return "ERROR";
        case LOG_LEVEL_INFO:
            return "INFO";
        case LOG_LEVEL_WARN:
            return "WARN";
        case LOG_LEVEL_ALL:
            return "ALL";
        case LOG_LEVEL_NONE:
            return "NONE";
        default:
            return "UNKNOWN";
    }
}

log_levle_t log_string_to_level(const char* level)
{
    if(strcmp(level, "DEBUG") == 0){
        return LOG_LEVEL_DEBUG;
    }else if(strcmp(level, "ERROR") == 0){
        return LOG_LEVEL_ERROR;
    }else if(strcmp(level, "INFO") == 0){
        return LOG_LEVEL_INFO;
    }else if(strcmp(level, "WARN") == 0){
        return LOG_LEVEL_WARN;
    }else if(strcmp(level, "ALL") == 0){
        return LOG_LEVEL_ALL;
    }else if(strcmp(level, "NONE") == 0){
        return LOG_LEVEL_NONE;
    }else{
        log_warn("unknown log level %s", level);
        return LOG_LEVEL_NONE;
    }

    return LOG_LEVEL_NONE;
}

void print_array_int(int* data, size_t len, int n_spaces)
{
    int i;
    char format[100] = {};

    if(len == 0){
        log_warn("Parameter len is 0 \n");
        return;
    }

    if(data == NULL){
        log_warn("Cannot input NULL ptr\n");
        return;
    }

    if(n_spaces >= 100){
        log_warn("Parameter n_spaces = %d is out of range\n", n_spaces);
        return;
    }

    TK_INDETION(format, n_spaces);

    log_text("%s(", format);
    for(i = 0; i<(len -1); i++){
        log_text("%d, ", data[i]);
        if((i+1)%16 == 0){
            log_text("\n %s", format);
        }
    }
    log_text("%d)\n", data[i]);
}

