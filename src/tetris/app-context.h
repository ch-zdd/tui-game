
#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "../../lib/common.h"
#include <ncurses.h>

#define MAX_TETROMINOES_NAME_LEN 16
#define MAX_TETROMINOES_NUM 16

enum{
    TETROMINO_TYPE_NORMAL = 0,
    TETROMINO_TYPE_PENETRATE,
    TETROMINO_TYPE_BOMB,
    TETROMINO_TYPE_NONE,
};

typedef struct{
    char name[MAX_TETROMINOES_NAME_LEN];
    int type;
    int map;
    int map_width;
    int map_height;
    int color_index;
#define MAX_SYMBOL_LEN 3
    char symbol[MAX_SYMBOL_LEN+1];
    char background[MAX_SYMBOL_LEN+1];
}tetromino_t;

typedef struct{
    char cfg_path[MAX_PATH_LEN];
    char game_cfg_path[MAX_PATH_LEN];
    int game_window_width;
    int game_window_height;

    int symbol_uniform_width;
    tetromino_t tetromino[MAX_TETROMINOES_NUM];
    int tetrominoes_num;
}tetris_context_t;

void init_app_context(void);
tetris_context_t* get_app_context(void);

int set_cfg_path(const char* path);
int parse_app_cfg(const char* cfg_path);

int set_game_cfg_path(const char* path);
int parse_game_cfg(void);

const char* tetromino_type_to_string(int type);
int tetromino_type_to_int(const char* type);
char* tetromino_to_string(tetromino_t tetromino);

#endif