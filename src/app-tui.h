#ifndef TK_APP_TUI_H
#define TK_APP_TUI_H

#include "app-context.h"
#include "../lib/common.h"

#define MAX_SELF_BATTLER_NUM 3
#define MAX_ENEMY_BATTLER_NUM 3
#define MAX_BATTLER_NUM MAX_SELF_BATTLER_NUM+MAX_ENEMY_BATTLER_NUM
#define KEY_INFO_LINE_NUM 2

#define INFO_TYPE_ATTR 1
#define INFO_TYPE_INNATE 2

#define wclrctx(w) do{werase(w); box(w, 0, 0);}while(0)

#define clrzone(lines, cols, y, x) wclrzone(stdscr, lines, cols, y, x)
#define wclrzone(w, lines, cols, y, x)          \
    do{                                         \
        char buffer[1024] = "";                 \
        memset(buffer, ' ', cols);              \
        for(int i = 0; i< lines; i++){          \
            mvwprintw(w, y+i, x, "%s", buffer); \
        }                                       \
    }while(0)

typedef struct{
    int column;
    int str_len;
    short color;
}color_mark_t;

int app_tui_init(void);
int app_tui_final(void);

int game_window_draw(void);
void draw_battle_info(void);

void show_battler(void);
int show_attr_info(battle_type_t battler_type, int battler_position);
int show_innate_info(battle_type_t battler_type, int battler_position);

int battler_highlight(battle_type_t battler_type, int battler_position, attr_t color_attr, short color_index);
int revert_battler_info_screen(void);

void wchar_blink(WINDOW* win, int line, int col, int n, int blink_times);
int battler_blink_delay(battle_type_t battler_type, int battler_position, int blink_times, short font_color);

int select_enemy_tui(int self_idx);

tk_position_state_t get_battler_position_state(battle_type_t battler_type, int position);
int generate_position(battle_type_t battler_type);

extern battler_t* get_battlers(void);
extern int get_battlers_num(void);

int print_battle_report_line(chtype* chstr, size_t len);
int battle_report(const char* format, ...) CHECK_FMT(1,2);
int roll_battle_report(int direction);

#endif