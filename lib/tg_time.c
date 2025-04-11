#include "common.h"
#include "tg_time.h"
#include "data_handle.h"

/*
功能： 获取当前日期和时间，用于表格的时间戳
*/
char* tg_time_stamp(void)
{
    time_t rawtime;
    struct tm *info;
    static char date_time[25];

    time( &rawtime );
    info = localtime( &rawtime );
 
    strftime(date_time, 25, "%H:%M:%S", info);
    return date_time;
}

// 获取高精度时间戳的函数
char* tg_high_precision_time_stamp(void)
{
    static char date_time[100];
    struct timespec ts;
    struct tm *info;

    // 获取当前时间戳
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        perror("clock_gettime");
        return NULL;
    }

    // 将时间戳转换为本地时间
    info = localtime(&ts.tv_sec);

    // 格式化时间为字符串，包括纳秒部分
    snprintf(date_time, sizeof(date_time), "%04d-%02d-%02d %02d:%02d:%02d.%09ld",
             info->tm_year + 1900, info->tm_mon + 1, info->tm_mday,
             info->tm_hour, info->tm_min, info->tm_sec, ts.tv_nsec);

    return date_time;
}

tg_utime_t tg_get_utime(void)
{
    struct timeval utime;
    tg_utime_t res_utime;
    gettimeofday(&utime, NULL);

    res_utime.tv_sec = utime.tv_sec;
    res_utime.tv_usec = utime.tv_usec;

    return res_utime;
}

tg_timer_t* tg_timer_create(tg_utime_t duration, const char* timer_name)
{

    tg_timer_t* timer = tg_malloc(sizeof(tg_timer_t));
    if (timer == NULL) {
        log_error("Failed to allocate memory for timer %s.", timer_name == NULL ? NULL: timer_name);
        return NULL;
    }

    if(timer_name != NULL && strlen(timer_name) != 0){
        timer->name = tg_malloc(sizeof(timer_name));
        if(timer->name == NULL) {
            log_warn("Failed to allocate memory for timer's name %s.", timer_name == NULL ? NULL: timer_name);   
        }else{
            strcpy(timer->name, timer_name);
        }
    }else{
        timer->name = tg_malloc(25-2);
        if(timer->name == NULL) {
            log_warn("Failed to allocate memory for timer's default name %s.", timer_name == NULL ? NULL: timer_name);   
        }else{
            snprintf(timer->name, 25-2, "NO.%s", trim_chars(tg_time_stamp(), ":"));
        }
    }
    
    timer->status = TIMER_STOPPED;
    timer->duration.tv_usec = duration.tv_usec%1000000;
    timer->duration.tv_sec = duration.tv_sec + duration.tv_usec/1000000;

    return timer;
}

void tg_timer_destroy(tg_timer_t* timer)
{
    if(timer == NULL){
        return;
    }

    tg_free(timer);

    if(timer->name != NULL){
        tg_free(timer->name);
    }
}

int tg_timer_start(tg_timer_t* timer)
{
    if(timer == NULL){
        return TG_ERROR;
    }

    timer->status = TIMER_RUNNING;
    timer->start_time = tg_get_utime();
    timer->end_time.tv_sec = timer->start_time.tv_sec + timer->duration.tv_sec;
    timer->end_time.tv_usec = timer->start_time.tv_usec + timer->duration.tv_usec;

    return TG_OK;
}

int tg_timer_stop(tg_timer_t* timer)
{
    if(timer == NULL){
        return TG_ERROR;
    }

    timer->status = TIMER_STOPPED;
    timer->end_time = tg_get_utime();
    return TG_OK;
}

int tg_timer_reset(tg_timer_t* timer)
{
    if(timer == NULL){
        return TG_ERROR;
    }

    if(timer->status == TIMER_STOPPED){
        log_debug("Timer %s is stopped");
        return TG_ERROR;
    }

    timer->start_time = tg_get_utime();
    timer->end_time.tv_sec = timer->start_time.tv_sec + timer->duration.tv_sec;
    timer->end_time.tv_usec = timer->start_time.tv_usec + timer->duration.tv_usec;

    return TG_OK;
}

bool tg_timer_is_expired(tg_timer_t* timer)
{
    if(timer == NULL){
        return false;
    }

    if(timer->status == TIMER_STOPPED){
        log_debug("Timer is stopped");
        return false;
    }

    tg_utime_t current_time = tg_get_utime();
    return current_time.tv_sec > timer->end_time.tv_sec ||
           (current_time.tv_sec == timer->end_time.tv_sec &&
            current_time.tv_usec >= timer->end_time.tv_usec);
}
