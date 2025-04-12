
#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "../../lib/common.h"
#include <ncurses.h>

typedef struct{
    char cfg_path[MAX_PATH_LEN];
    char game_cfg_path[MAX_PATH_LEN];
}app_context_t;

void init_app_context(void);
app_context_t* get_app_context(void);

int set_cfg_path(const char* path);
int parse_app_cfg(const char* cfg_path);

int set_game_cfg_path(const char* path);
int parse_game_cfg(void);

const char* shape_type_to_string(int type);
int shape_type_to_int(const char* type);

#endif