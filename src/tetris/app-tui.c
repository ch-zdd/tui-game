#include <string.h>
#include <ncurses.h>

#include "app-tui.h"
#include "../../lib/data_handle.h"

tetris_window_t Windows;
tui_game_board_t game_board;

tetris_window_t* get_windows_para(void)
{
    return &Windows;
}

tui_game_board_t* get_game_board(void)
{
    return &game_board;
}


int app_tui_init(void)
{
    tui_init();
    tetris_window_t* tui = &Windows;

    getmaxyx(stdscr, tui->main.scr_line, tui->main.scr_col);
    if(tui->main.scr_line < 16 || tui->main.scr_col < 70){
        printw("screen size(line:%d, at least 16; column:%d, at least 70) too small, press any key to exit...", tui->main.scr_line, tui->main.scr_col);
        getch();
        return TG_ERROR;
    }
    if(tui->main.scr_col < get_app_context()->game_window_width*2){
        printw("screen size(line:%d, at least 16; column:%d, at least twice the width of the game window %d) too small, press any key to exit...", 
            tui->main.scr_line, tui->main.scr_col, get_app_context()->game_window_width*2);
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
    strncpy(tui->main.store_path, "main_window.scr", sizeof(tui->main.store_path));

    tui->info.active = false;
    tui->statistics.active = false;
    tui->game.active = false;
    strncpy(tui->game.store_path, "battle_report.scr", sizeof(tui->main.store_path));
    strncpy(tui->info.store_path, "battler_info.scr", sizeof(tui->main.store_path));
    strncpy(tui->statistics.store_path, "battle.scr", sizeof(tui->main.store_path));

    return TG_OK;
}

int app_tui_final(void)
{
    delwin(stdscr);
    tui_final();

    return TG_OK;
}

int game_window_draw(void)
{
    tetris_window_para_t* main = &Windows.main;
    tetris_window_para_t* info = &Windows.info;
    tetris_window_para_t* stat = &Windows.statistics;
    tetris_window_para_t* game = &Windows.game;

    if(main->active == false){
        log_error("ERROR: tui is not initialized");
        return TG_ERROR;
    }

    game->scr_line = get_app_context()->game_window_height;
    game->scr_col = get_app_context()->game_window_width;

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
    tui_game_board_t * board = get_game_board();

    if(scr_x) *scr_x = x*board->cell_width;
    if(scr_y) *scr_y = y;

    return TG_OK;
}

int coord_to_game(int scr_x, int scr_y, int* x, int* y)
{
    tui_game_board_t * board = get_game_board();

    if(x) *x = scr_x/board->cell_width;
    if(y) *y = scr_y;

    return TG_OK;
}

int draw_border(void)
{
    int i, scr_x, scr_y;
    tui_game_board_t * board = get_game_board();
    tetris_window_para_t* game = &Windows.game;

    for(i = 0; i<board->height; i++){
        if(board->flags[i*board->width] != 1 || board->flags[i*board->width+board->width-1] != 1){
            log_warn("border of board error");
        }
        coord_to_scr(0, i, &scr_x, &scr_y);
        mvwprintw(game->w, scr_y, scr_x, "%s", "* ");
        coord_to_scr(board->width-1, i, &scr_x, &scr_y);
        mvwprintw(game->w, scr_y, scr_x, "%s", "* ");
    }
    for(i = 0; i<board->width; i++){
        if(board->flags[i] != 1 || board->flags[(board->height-1)*board->width+i] != 1){
            log_warn("border of board error");
        }
        coord_to_scr(i, 0, &scr_x, &scr_y);
        mvwprintw(game->w, scr_y, scr_x, "%s", "* ");
        coord_to_scr(i, board->height-1, &scr_x, &scr_y);
        mvwprintw(game->w, scr_y, scr_x, "%s", "* ");
    }

    return TG_OK;
}

int draw_board(void)
{
    int i, j, x, y;
    tui_game_board_t * board = get_game_board();
    tetris_window_para_t* game = &Windows.game;

    for(i = 1; i < board->height-1; i++){
        for(j = 1; j < board->width-1; j++){
            coord_to_scr(j, i, &x, &y);
            if(board->flags[i*board->width+j] == 1){
                mvwprintw(game->w, y, x, "%s", board->symbol);
            }else{
                mvwprintw(game->w, y, x, "%s", board->bkg_symbol);
            }
        }
    }
    wrefresh(game->w);

    return TG_OK;
}

int draw_tetromino(tui_tetromino_t tetromino, bool is_clear)
{
    tetromino_t shape = tetromino.shape;
    tetris_window_para_t* game = &Windows.game;
    int i = 0;
    int j = 0;
    int x = tetromino.x;
    int y = tetromino.y;
    int x_scr, y_scr;

    log_debug("draw tetromino");
    for(i = 0; i < shape.map_height; i++){
        // 边界外不显示
        if(tetromino.y+i <= 1){
            continue;
        }

        for(j = 0; j < shape.map_width; j++){           
            coord_to_scr(x+j, y+i, &x_scr, &y_scr);

            if((shape.map >> (i*shape.map_width+j)) & 0x01){
                if(is_clear){
                    // 用背景清除
                    mvwprintw(game->w, y_scr, x_scr, "%s", shape.background);
                }else{
                    mvwprintw(game->w, y_scr, x_scr, "%s", shape.symbol);
                }
            }else{
                // 游戏板背景统一
            }
        }
    }
    wrefresh(game->w);
    
    return TG_OK;
}