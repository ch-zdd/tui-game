#ifndef TK_APP_TUI_H
#define TK_APP_TUI_H

#include "app-context.h"
#include "../../lib/common.h"
#include "../../lib/tui.h"

#define MAX_SELF_BATTLER_NUM 3
#define MAX_ENEMY_BATTLER_NUM 3
#define MAX_BATTLER_NUM MAX_SELF_BATTLER_NUM+MAX_ENEMY_BATTLER_NUM
#define KEY_INFO_LINE_NUM 2

#define INFO_TYPE_ATTR 1
#define INFO_TYPE_INNATE 2

#define battle_report(format, ...) scroll_print(tk_scroll, format, ##__VA_ARGS__)
#define roll_battle_report(direction) scroll_direction(tk_scroll, direction)

typedef struct{
    int scr_line;
    int scr_col;
    WINDOW* w;
    char store_path[MAX_PATH_LEN];
    bool active;
}tetris_window_para_t;

typedef struct{
    tetris_window_para_t main;
    tetris_window_para_t game;
    tetris_window_para_t info;
    tetris_window_para_t statistics;

}tetris_window_t;

typedef struct{
    int tetromino_index;
    int x;
    int y;
    int rotate;
}tui_tetromino_t;

typedef struct{
uint8_t* screen;
int width;
int height;
}tui_game_screen_t;

extern scroll_t *tk_scroll;

int app_tui_init(void);
int app_tui_final(void);

int game_window_draw(void);

#endif