#ifndef TG_TIME_H
#define TG_TIME_H

#include <time.h>
#include<sys/time.h>

typedef struct timeval tg_utime_t;
typedef struct {
#define TIMER_STOPPED 0
#define TIMER_RUNNING 1
    int status;
    tg_utime_t start_time;
    tg_utime_t end_time;
    tg_utime_t duration;
}tg_timer_t;

char* tg_time_stamp(void);
char* tg_high_precision_time_stamp(void);

tg_utime_t tg_get_utime(void);
tg_timer_t* tg_timer_create(tg_utime_t duration);
void tg_timer_destroy(tg_timer_t* timer);

int tg_timer_start(tg_timer_t* timer);
int tg_timer_stop(tg_timer_t* timer);
int tg_timer_reset(tg_timer_t* timer);

bool tg_timer_is_expired(tg_timer_t* timer);

#endif