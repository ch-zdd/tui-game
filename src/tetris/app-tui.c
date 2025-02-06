#include <string.h>
#include <ncurses.h>

#include "app-tui.h"
#include "../../lib/data_handle.h"

tui_game_screen_t Game_screen;
tetris_window_t Windows;

tetris_window_t* get_windows_para(void)
{
    return &Windows;
}

tui_game_screen_t* get_game_screen(void)
{
    return &Game_screen;
}

int game_screen_create(int width, int height)
{
    Game_screen.width = width;
    Game_screen.x = 0;
    Game_screen.height = height;
    Game_screen.y = 0;
    Game_screen.screen = (uint8_t*)tg_malloc(Game_screen.width*Game_screen.height*sizeof(uint8_t));
    log_debug("Allocate memory for game screen, %d bytes", Game_screen.width*Game_screen.height*sizeof(uint8_t));
    if(Game_screen.screen == NULL){
        log_error("Failed to allocate memory for game screen, %s", strerror(errno));
        return TG_ERROR;
    }
    memset(Game_screen.screen, 0, Game_screen.width*Game_screen.height*sizeof(uint8_t));

    return TG_OK;
}

int game_screen_destroy(void)
{
    if(Game_screen.screen != NULL){
        tg_free(Game_screen.screen);
        Game_screen.screen = NULL;
    }

    return TG_OK;
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
    game_screen_destroy();
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

    getch();
    mvwprintw(game->w, game->scr_line/2, game->scr_col/2, "Game start...");
    wrefresh(game->w);

    return TG_OK;
}

int tetromino_draw(tui_tetromino_t tetromino)
{
    //int x = Game_screen.x + tetromino.x;
    //int y = Game_screen.y + tetromino.y;
    //int i = 0;
    
    return TG_OK;
}