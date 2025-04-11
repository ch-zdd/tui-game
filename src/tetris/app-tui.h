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
    tetromino_t shape;
    // 左上角为坐标原点，x和y为方块左上角的坐标
    int x;
    int y;
    int rotate;
}tui_tetromino_t;

typedef struct{
    uint8_t* flags;
    char symbol[MAX_SYMBOL_LEN+1];
    char bkg_symbol[MAX_SYMBOL_LEN+1];
    int cell_width;
    int width;
    int height;
}tui_game_board_t;

extern scroll_t *tk_scroll;

tetris_window_t* get_windows_para(void);
tui_game_board_t* get_game_board(void);

int app_tui_init(void);
int app_tui_final(void);

int game_window_draw(void);

int coord_to_game(int scr_x, int scr_y, int* x, int* y);
int coord_to_scr(int x, int y, int* scr_x, int* scr_y);
int draw_border(void);
int draw_board(void);
int draw_tetromino(tui_tetromino_t tetromino, bool is_clear);

#endif