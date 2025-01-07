#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "common.h"
#include "tg_log.h"
#include "tg_time.h"

#define DEFAULT_LOG_PATH "/tmp/tui-game.log"
#define MAX_LOG_SIZE 1024

static log_context_t log_context = {
    .default_fp = NULL,
    .fp = NULL,
    .level = LOG_LEVEL_INFO
};

// 定义一个辅助函数，用于实现带函数名和行号的日志记录  
void tg_log(const char *file, int line, int level, const char *format, ...) 
{
    char buffer[MAX_LOG_SIZE];
    char* print_buf = buffer;
    int level_str_len = 30;
    int pos_info_len = 256;
    int info_len = MAX_LOG_SIZE-pos_info_len-level_str_len;
    va_list args;

    if(level > log_context.level){
        return;
    }
    
    print_buf += snprintf(print_buf, level_str_len, "%s <%-5s> | ", tg_time_stamp(), log_level_to_string(level));

    va_start(args, format);  
    // 首先，我们打印出日志的主要内容（采用vfprintf函数处理可变参数）  
    print_buf += vsnprintf(print_buf, info_len, format, args);  
    va_end(args);

    // 然后，在同一行末尾追加函数名和行号信息  
    // 注意：为了保持格式整洁，我们在函数名和行号前添加了分隔符，并进行了换行  
    snprintf(print_buf, pos_info_len, " (%s:%d)\n", file, line);

    fprintf(log_context.default_fp, "%s", buffer);
    //保存到用户自定义日志文件
    if(log_context.fp != NULL){
        fprintf(log_context.fp, "%s", buffer);
    }
    //因为没有性能要求，所以 这里直接实时刷新日志
    fflush(log_context.default_fp);
    fflush(log_context.fp);
}

void tg_log_text(const char* format, ...)
{
    FILE* log_file = log_context.fp;
    char buffer[MAX_LOG_SIZE];
    char* print_buf = buffer;
    int info_len = MAX_LOG_SIZE;
    va_list args;

    if(log_file == NULL){
        log_file = stderr;
        return;
    }

    if(log_context.level < LOG_LEVEL_INFO){
        return;
    }
    
    va_start(args, format);  
    // 首先，我们打印出日志的主要内容（采用vfprintf函数处理可变参数）  
    vsnprintf(print_buf, info_len, format, args);  
    va_end(args);

    fprintf(log_context.default_fp, "%s", print_buf);
    //保存到用户自定义日志文件
    if(log_context.fp != NULL){
        fprintf(log_context.fp, "%s", print_buf);
    }
    //因为没有性能要求，所以 这里直接实时刷新日志
    fflush(log_context.default_fp);
    fflush(log_context.fp);
}

// 初始化日志模块,日志必定打印在默认路径，但可以重定向到其他文件
int log_init(void)
{

    if(log_context.level == LOG_LEVEL_MAX){
        log_context.level = LOG_LEVEL_INFO;
    }

    FILE* default_log_file = fopen(DEFAULT_LOG_PATH, "w");
    if(default_log_file == NULL){
        tg_print("Open default log file [%s] failed !, %s", DEFAULT_LOG_PATH, strerror(errno));
        return TG_ERROR;
    }
    log_context.default_fp = default_log_file;

    return TG_OK;
}

void log_final(void)
{
    if(log_context.fp == NULL || log_context.fp == stderr){
        return;
    }

    fclose(log_context.fp);
    fclose(log_context.default_fp);
}

int set_log_file(const char* path)
{
    FILE* log_file = fopen(path, "w");
    if(log_file == NULL){
        log_error("open log file [%s] failed!, %s", path, strerror(errno));
        return TG_ERROR;
    }

    log_context.fp = log_file;
    return TG_OK;
}

int set_log_level(log_levle_t level)
{
    if(level >= LOG_LEVEL_MAX){
        log_error("invalid log level %d", level);
        return TG_ERROR;
    }

    log_context.level = level;
    return TG_OK;
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

    TG_INDETION(format, n_spaces);

    log_text("%s(", format);
    for(i = 0; i<(len -1); i++){
        log_text("%d, ", data[i]);
        if((i+1)%16 == 0){
            log_text("\n %s", format);
        }
    }
    log_text("%d)\n", data[i]);
}

