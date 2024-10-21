#ifndef TK_APP_INIT_H
#define TK_APP_INIT_H
#include "app-context.h"

int app_init(void);
int app_final(void);
int set_role_path(const char* path);
int read_role(void);
int set_role(void);

#endif