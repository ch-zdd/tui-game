#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "log.h"

#define DEFAULT_LOG_PATH "/var/log/tk.log"
#define MAX_LOG_SIZE 1024

static FILE* G_log_file = NULL;
static log_levle_t G_log_level = LOG_LEVEL_MAX;

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
void log_with_info(const char *file, int line, int level, const char *format, ...) 
{
    char buffer[MAX_LOG_SIZE];
    char* print_buf = buffer;
    int level_str_len = 30;
    int pos_info_len = 256;
    int info_len = MAX_LOG_SIZE-pos_info_len-level_str_len;
    FILE* log_file = G_log_file;
    va_list args;

    if(log_file == NULL){
        log_file = stderr;
    }

    if(level > G_log_level){
        return;
    }
    
    if(level == LOG_LEVEL_TEXT){
        va_start(args, format);  
        // 首先，我们打印出日志的主要内容（采用vfprintf函数处理可变参数）  
        print_buf += vsnprintf(print_buf, MAX_LOG_SIZE, format, args);  
        va_end(args);
    }else{
        print_buf += snprintf(print_buf, level_str_len, "%s <%5s> | ", tk_time_stamp(), log_level_to_string(level));

        va_start(args, format);  
        // 首先，我们打印出日志的主要内容（采用vfprintf函数处理可变参数）  
        print_buf += vsnprintf(print_buf, info_len, format, args);  
        va_end(args);

        // 然后，在同一行末尾追加函数名和行号信息  
        // 注意：为了保持格式整洁，我们在函数名和行号前添加了分隔符，并进行了换行  
        snprintf(print_buf, pos_info_len, " [%s:%d]\n", file, line);
    }

    fprintf(log_file, "%s", buffer);
}

void log_init(void)
{
    if(G_log_level == LOG_LEVEL_MAX){
        G_log_level = LOG_LEVEL_INFO;
    }

    if(G_log_file == NULL){
        if(TK_OK != set_log_file(DEFAULT_LOG_PATH)){
            G_log_file = stderr;
        }
    }
}

void log_final(void)
{
    fclose(G_log_file);
}

int set_log_file(const char* path)
{
    FILE* log_file = fopen(path, "w");
    if(log_file == NULL){
        log_error("open log file [%s] failed!, %s", path, strerror(errno));
        return TK_ERROR;
    }

    G_log_file = log_file;
    return TK_OK;
}

int set_log_level(log_levle_t level)
{
    if(level >= LOG_LEVEL_MAX){
        log_error("invalid log level %d", level);
        return TK_ERROR;
    }

    G_log_level = level;
    return TK_OK;
}

const char* log_level_to_string(log_levle_t level)
{
    switch(level){
        case LOG_LEVEL_TEXT:
            return "TEXT";
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
    if(strcmp(level, "TEXT") == 0){
        return LOG_LEVEL_TEXT;
    }else if(strcmp(level, "DEBUG") == 0){
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

