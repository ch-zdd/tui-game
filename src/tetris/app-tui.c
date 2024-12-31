#include <string.h>
#include <ncurses.h>

#include "app-tui.h"
#include "../../lib/data_handle.h"

tui_game_screen_t Game_screen;
tetris_window_t Windows;

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

    tui->game.scr_line = tui->main.scr_line;
    tui->game.scr_col = get_app_context()->game_window_width;;

    tui->info.scr_line = tui->main.scr_line/2;
    tui->info.scr_col = tui->main.scr_col-tui->game.scr_col;

    tui->statistics.scr_line = tui->main.scr_line-tui->info.scr_line;
    tui->statistics.scr_col = tui->main.scr_col-tui->game.scr_col;

    Game_screen.width = tui->game.scr_col-2;
    Game_screen.height = tui->game.scr_line-2;
    Game_screen.screen = (uint8_t*)tg_malloc(Game_screen.width*Game_screen.height*sizeof(uint8_t));
    if(Game_screen.screen == NULL){
        log_error("Failed to allocate memory for game screen");
        return TG_ERROR;
    }
    memset(Game_screen.screen, 0, Game_screen.width*Game_screen.height*sizeof(uint8_t));

    return TG_OK;
}

int app_tui_final(void)
{
    if(Game_screen.screen != NULL){
        tg_free(Game_screen.screen);
        Game_screen.screen = NULL;
    }
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

    getch();

    return TG_OK;
}



int tetromino_draw(tui_tetromino_t* tetromino)
{
    return TG_OK;
}