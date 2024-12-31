#include <string.h>
#include <ncurses.h>

#include "app.h"
#include "../../lib/common.h"
#include "app-context.h"
#include "app-tui.h"

int show_tetrominoe_for_debug(tetromino_t* tetromino, int num);
tui_tetromino_t generate_tetrominoe(void);


int app_run(void)
{
    tetris_context_t* ctx = get_app_context();
    show_tetrominoe_for_debug(ctx->tetromino, ctx->tetrominoes_num);

    if(TG_OK != game_window_draw()){
        return TG_ERROR;
    }

    //game_loop();

    return TG_OK;
}

int show_tetrominoe_for_debug(tetromino_t* tetromino, int num)
{
    log_debug("Load tetromino num: %d", num);
    for(int i = 0; i< num; i++){
        log_text("Tetromino name[%s] map[%d] type[%s] symbol[%s] bk[%s] color[%s]", 
            tetromino[i].name, tetromino[i].map, tetromino_type_to_string(tetromino[i].type), tetromino[i].symbol, tetromino[i].background, color_to_string(tetromino[i].color_index));
        log_text("%s", tetromino_to_string(tetromino[i]));
    }

    return TG_OK;
}

void game_loop(void)
{
    tui_tetromino_t next_tetromino_index;
    tui_tetromino_t current_tui_tetromino;

    next_tetromino_index = generate_tetrominoe();
    while(1){
        current_tui_tetromino = next_tetromino_index;
        next_tetromino_index = generate_tetrominoe();
    }

}

tui_tetromino_t generate_tetrominoe(void)
{
    tui_tetromino_t tetromino;
    tetris_context_t* ctx = get_app_context();
    int index = rand() % ctx->tetrominoes_num;
    tetromino.tetromino_index = index;
    tetromino.x = ctx->game_window_width/2;
    tetromino.y = 0;
    tetromino.rotate = 0;
    log_debug("Generate tetrominoe index: %d", index);

    return tetromino;
}