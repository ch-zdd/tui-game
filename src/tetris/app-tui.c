#include <string.h>
#include <ncurses.h>

#include "app-tui.h"
#include "../../lib/data_handle.h"
#include "app.h"

#define CELL_SYMBOL_MAX_LEN 3

static tui_context_t tui_context;

typedef struct{
    int max_length;
    int number;
    #define MAX_SYMBOL_NUM 16
    str_t symbol[MAX_SYMBOL_NUM];
}cell_symbol_pool_t;

static cell_symbol_pool_t cell_symbol_pool;

tui_context_t* get_tui_context(void)
{
    return &tui_context;
}

void set_game_win_size(int h, int w)
{
    get_tui_context()->game_window_height = h;
    get_tui_context()->game_window_width = w;
}

// 初始化默认符号，main函数前执行防止意外错误
__attribute__((constructor)) void init_cell_symbol_pool(void)
{
    memset(&cell_symbol_pool, 0, sizeof(cell_symbol_pool));
    cell_symbol_pool.max_length = CELL_SYMBOL_MAX_LEN;
    cell_symbol_pool.number = 0;

    // 固定第一个是默认背景色，第二个为默认前景色，
    // 即index = 0 是背景，与某些变量初始化为0有关系，可以修改符号但不能随意索引值
    add_cell_symbol("  ");
    add_cell_symbol("()");
}

int add_cell_symbol(const char* symbol)
{
    // 首先去重
    int i;
    for(i = 0; i< cell_symbol_pool.number; i++){
        if(strcmp(symbol, cell_symbol_pool.symbol[i].str) == 0){
            log_debug("Symbol[%s] already exists", symbol);
            return i;
        }
    }

    size_t len = strlen(symbol);

    if(cell_symbol_pool.max_length < len){
        log_warn("Invalid symbol[%s], use default symbol", symbol);
        return 0;
    }

    str_t* cell_symbol = &cell_symbol_pool.symbol[cell_symbol_pool.number];
    if(chars_to_string(symbol, cell_symbol) == TG_ERROR){
        free_string(cell_symbol);
        return TG_ERROR;
    }

    cell_symbol_pool.number++;

    return cell_symbol_pool.number-1;
}

str_t* get_cell_symbol(int index)
{
    if(index < 0 || index >= cell_symbol_pool.number){
        log_warn("Invalid index[%d], use default symbol", index);
        // 防止返回NULL
        return &cell_symbol_pool.symbol[0];
    }

    return &cell_symbol_pool.symbol[index];
}

int app_tui_init(void)
{
    tui_init();

    get_tui_context()->cell_width = CELL_SYMBOL_MAX_LEN;
    tetris_window_t* tui = &get_tui_context()->window;

    if(get_tui_context()->game_window_height == 0){
        get_tui_context()->game_window_height = DEFAULT_WINDOW_HEIGHT;
    }
    if(get_tui_context()->game_window_width == 0){
        get_tui_context()->game_window_width = DEFAULT_WINDOW_WIDTH;
    }

    getmaxyx(stdscr, tui->main.scr_line, tui->main.scr_col);
    if(tui->main.scr_line < 16 || tui->main.scr_col < 70){
        printw("screen size(line:%d, at least 16; column:%d, at least 70) too small, press any key to exit...", tui->main.scr_line, tui->main.scr_col);
        getch();
        return TG_ERROR;
    }
    
    if(tui->main.scr_col < get_tui_context()->game_window_width*2){
        printw("screen size(line:%d, at least 16; column:%d, at least twice the width of the game window %d) too small, press any key to exit...", 
            tui->main.scr_line, tui->main.scr_col, get_tui_context()->game_window_width*2);
        getch();
        return TG_ERROR;
    }

    tui->main.active = true;
    tui->main.w = stdscr;
    if(stdscr == NULL){
        tui->main.active = false;
        log_error("Failed to init stdscr");
        return TG_ERROR;
    }
    strncpy(tui->main.store_path, "bk_main.scr", sizeof(tui->main.store_path));

    tui->info.active = false;
    tui->statistics.active = false;
    tui->game.active = false;
    strncpy(tui->game.store_path, "bk_game.scr", sizeof(tui->main.store_path));
    strncpy(tui->info.store_path, "bk_info.scr", sizeof(tui->main.store_path));
    strncpy(tui->statistics.store_path, "bk_statics.scr", sizeof(tui->main.store_path));

    return TG_OK;
}

int app_tui_final(void)
{
    tui_final();
    int i = 0;
    
    // 释放符号池
    for(i = 0; i<cell_symbol_pool.number; i++){
        free_string(&cell_symbol_pool.symbol[i]);
    }
    cell_symbol_pool.number = 0;

    return TG_OK;
}

int show_shape_for_debug(shape_t* shape, int num)
{
    log_debug("Load tetromino num: %d", num);
    for(int i = 0; i< num; i++){
        log_text("Tetromino name[%s] map[%d] type[%s] symbol[%s] color[%s]\n", 
            shape[i].name, shape[i].map, shape_type_to_string(shape[i].type), get_cell_symbol(shape[i].symbol_index)->str, color_to_string(shape[i].color_index));
        log_text("%s\n", shape_to_string(shape[i]));
    }

    return TG_OK;
}

char* shape_to_string(shape_t shape)
{
    static char buff[1024];
    
    memset(buff, 0, sizeof(buff));

    for(int i = 0; i < shape.map_height; i++){
        for(int j = 0; j < shape.map_width; j++){
            if((shape.map >> (i*shape.map_width+j)) & 0x01){
                snprintf(buff+strlen(buff), sizeof(buff)-strlen(buff), "%s", get_cell_symbol(shape.symbol_index)->str);
            }else{
                snprintf(buff+strlen(buff), sizeof(buff)-strlen(buff), "%s", BACKGROUND_DEFAULT->str);
            }
            
        }
        snprintf(buff+strlen(buff), sizeof(buff)-strlen(buff), "\n");
    }

    return buff;
}

int game_window_draw(void)
{
    tetris_window_t* tui = &get_tui_context()->window;
    tetris_window_para_t* main = &tui->main;
    tetris_window_para_t* info = &tui->info;
    tetris_window_para_t* stat = &tui->statistics;
    tetris_window_para_t* game = &tui->game;

    if(main->active == false){
        log_error("ERROR: tui is not initialized");
        return TG_ERROR;
    }

    if(get_tui_context()->game_window_height > main->scr_line){
        log_error("ERROR: screen size(%d) too small, at least %d", main->scr_line, get_tui_context()->game_window_height);
    }
    if(get_tui_context()->game_window_width*2 > main->scr_col){
        log_error("ERROR: screen size(%d) too small, at least %d", main->scr_col, get_tui_context()->game_window_width*2);
    }

    game->scr_line = get_tui_context()->game_window_height;
    game->scr_col = get_tui_context()->game_window_width;

    info->scr_line = main->scr_line/2;
    info->scr_col = main->scr_col-game->scr_col;

    stat->scr_line = main->scr_line-info->scr_line;
    stat->scr_col = main->scr_col-game->scr_col;

    mvaddstr(main->scr_line/2, main->scr_col/2 - 12, "Press any key to start...");
    getch();
    scr_dump(main->store_path);


    game->w = newwin(game->scr_line, game->scr_col, 0, 0);
    box(game->w, 0, 0);
    wrefresh(game->w);
    game->active = true;

    info->w = newwin(info->scr_line, info->scr_col, 0, game->scr_col);
    box(info->w, 0, 0);
    wrefresh(info->w);
    info->active = true;

    stat->w = newwin(stat->scr_line, stat->scr_col, info->scr_line, game->scr_col);
    box(stat->w, 0, 0);
    wrefresh(stat->w);
    stat->active = true;

    mvwprintw(game->w, game->scr_line/2, game->scr_col/2, "Pause...");
    wrefresh(game->w);
    getch();
    wclear(game->w);

    return TG_OK;
}

int coord_to_scr(int x, int y, int* scr_x, int* scr_y)
{
    if(scr_x) *scr_x = x*get_tui_context()->cell_width;
    if(scr_y) *scr_y = y;

    return TG_OK;
}

int coord_to_game(int scr_x, int scr_y, int* x, int* y)
{
    if(x) *x = scr_x/get_tui_context()->cell_width;
    if(y) *y = scr_y;

    return TG_OK;
}

int draw_border(game_board_t* board)
{
    int i, scr_x, scr_y;
    tetris_window_para_t* game = &get_tui_context()->window.game;
    str_t* symbol = FOREGGROUND_DEFAULT;

    for(i = 0; i<board->height; i++){
        if(board->cell[i*board->width].presence != 1 || board->cell[i*board->width+board->width-1].presence != 1){
            log_warn("border of board error");
        }
        coord_to_scr(0, i, &scr_x, &scr_y);
        mvwprintw(game->w, scr_y, scr_x, "%s", symbol->str);
        coord_to_scr(board->width-1, i, &scr_x, &scr_y);
        mvwprintw(game->w, scr_y, scr_x, "%s", symbol->str);
    }
    for(i = 0; i<board->width; i++){
        if(board->cell[i].presence != 1 || board->cell[(board->height-1)*board->width+i].presence != 1){
            log_warn("border of board error");
        }
        coord_to_scr(i, 0, &scr_x, &scr_y);
        mvwprintw(game->w, scr_y, scr_x, "%s", symbol->str);
        coord_to_scr(i, board->height-1, &scr_x, &scr_y);
        mvwprintw(game->w, scr_y, scr_x, "%s", symbol->str);
    }

    return TG_OK;
}

int draw_board(game_board_t* board)
{
    int i, j, x, y;
    tetris_window_para_t* game = &get_tui_context()->window.game;
    board_cell_t cell;
    str_t* frg_symbol;
    str_t* bkg_symbol;

    //wclrzone(game->w, board->height-2, (board->width-2)* board->cell_width, 1, board->cell_width);
    //wrefresh(game->w);
    for(i = 1; i < board->height-1; i++){
        for(j = 1; j < board->width-1; j++){
            coord_to_scr(j, i, &x, &y);
            cell = board->cell[i*board->width+j];
            frg_symbol = get_cell_symbol(cell.Foreground_index);
            bkg_symbol = get_cell_symbol(cell.background_index);
            if(cell.presence == 1){
                mvwprintw(game->w, y, x, "%s", frg_symbol->str);
            }else{
                mvwprintw(game->w, y, x, "%s", bkg_symbol->str);
            }
        }
        log_text("\n");
    }
    wrefresh(game->w);

    return TG_OK;
}

int draw_tetromino(tetromino_t tetromino, bool is_clear)
{
    shape_t shape = get_tui_context()->shape[tetromino.shape_index];
    tetris_window_para_t* game = &get_tui_context()->window.game;
    int i = 0;
    int j = 0;
    int x = tetromino.x;
    int y = tetromino.y;
    int x_scr, y_scr;
    str_t* symbol = get_cell_symbol(shape.symbol_index);
    str_t* bkg_symbol = BACKGROUND_DEFAULT;

    log_debug("draw tetromino");
    for(i = 0; i < shape.map_height; i++){
        // 边界外不显示
        if(tetromino.y+i <= 0){
            continue;
        }

        for(j = 0; j < shape.map_width; j++){           
            coord_to_scr(x+j, y+i, &x_scr, &y_scr);

            if((shape.map >> (i*shape.map_width+j)) & 0x01){
                if(is_clear){
                    // 用背景清除
                    mvwprintw(game->w, y_scr, x_scr, "%s", bkg_symbol->str);
                }else{
                    mvwprintw(game->w, y_scr, x_scr, "%s", symbol->str);
                }
            }else{
                // 游戏板背景统一
            }
        }
    }
    wrefresh(game->w);
    
    return TG_OK;
}