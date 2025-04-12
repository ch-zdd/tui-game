#ifndef TG_APP_H
#define TG_APP_H
#include "app-context.h"

typedef struct{
    int Foreground_index;
    int background_index;
    bool presence;
}board_cell_t;

typedef struct{
    board_cell_t* cell;
    int width;
    int height;
}game_board_t;

typedef struct{
    int shape_index;
    // 左上角为坐标原点，x和y为方块左上角的坐标
    int x;
    int y;
    int rotate;
}tetromino_t;

int app_run(void);
int app_stop(void);

#endif