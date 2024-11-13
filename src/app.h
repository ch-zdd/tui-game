#ifndef TK_APP_H
#define TK_APP_H
#include "app-context.h"

#define BATTLER_ATTR_INIT(battler_attr, role_attr, level)                           \
    do{                                                                             \
        battler_attr.state = ATTR_NORMAL;                                           \
        battler_attr.duration = 0;                                                  \
        battler_attr.base_value = role_attr.base + role_attr.growth*level;          \
        battler_attr.current_value = battler_attr.base_value;                       \
    }while(0)                                                                       \

int app_run(void);
battler_t* get_battlers(void);
int get_battlers_num(void);

#define BUFF_DURATION_INF -1
#define BUFF_DURATION_NOR 2

#endif