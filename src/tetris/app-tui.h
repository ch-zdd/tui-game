#ifndef TK_APP_TUI_H
#define TK_APP_TUI_H

#include "app-context.h"
#include "app.h"
#include "../../lib/common.h"
#include "../../lib/tui.h"

#define DEFAULT_WINDOW_HEIGHT 24
#define DEFAULT_WINDOW_WIDTH 48

#define FOREGGROUND_DEFAULT get_cell_symbol(1)
#define BACKGROUND_DEFAULT get_cell_symbol(0)

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

#define MAX_TETROMINOES_NAME_LEN 16
#define MAX_TETROMINOES_NUM 16

enum{
    SHAPE_TYPE_NORMAL = 0,
    SHAPE_TYPE_PENETRATE,
    SHAPE_TYPE_BOMB,
    SHAPE_TYPE_NONE,
};

#define MAX_SYMBOL_LEN 3
typedef struct{
    char name[MAX_TETROMINOES_NAME_LEN];
    int type;
    int map;
    int map_width;
    int map_height;
    int color_index;

    int symbol_index;
    bool symbol_use;
}shape_t;

typedef struct{
    tetris_window_t window;
    int game_window_height;
    int game_window_width;

    int cell_width;
    char background_index;
    shape_t shape[MAX_TETROMINOES_NUM];
    int shape_num;
}tui_context_t;

extern scroll_t *tk_scroll;

tui_context_t* get_tui_context(void);

int app_tui_init(void);
int app_tui_final(void);

void set_game_win_size(int h, int w);
str_t* get_cell_symbol(int index);
int add_cell_symbol(const char* symbol);

int game_window_draw(void);

int show_shape_for_debug(shape_t* shape, int num);
char* shape_to_string(shape_t shape);

int coord_to_game(int scr_x, int scr_y, int* x, int* y);
int coord_to_scr(int x, int y, int* scr_x, int* scr_y);
int draw_border(game_board_t* board);
int draw_board(game_board_t* board);
int draw_tetromino(tetromino_t tetromino, bool is_clear);

#endif