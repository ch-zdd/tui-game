#ifndef TK_APP_TUI_H
#define TK_APP_TUI_H

#include "app-context.h"
#include "../../lib/common.h"
#include "../../lib/tui.h"

#define DEFAULT_WINDOW_HEIGHT 24
#define DEFAULT_WINDOW_WIDTH 48


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
    int x;
    int height;
    int y;
}tui_game_screen_t;

extern scroll_t *tk_scroll;

tetris_window_t* get_windows_para(void);
tui_game_screen_t* get_game_screen(void);
int game_screen_create(int width, int height);
int game_screen_destroy(void);

int app_tui_init(void);
int app_tui_final(void);

int game_window_draw(void);
int tetromino_draw(tui_tetromino_t tetromino);

#endif