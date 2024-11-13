#ifndef TK_APP_INIT_H
#define TK_APP_INIT_H
#include "app-context.h"

int app_init(void);
int app_final(void);

int set_game_cfg_path(const char* path);

#endif