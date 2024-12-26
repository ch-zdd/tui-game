#ifndef TK_APP_TUI_H
#define TK_APP_TUI_H

#include "app-context.h"
#include "../../lib/common.h"
#include "../../lib/tui.h"

#define MAX_SELF_BATTLER_NUM 3
#define MAX_ENEMY_BATTLER_NUM 3
#define MAX_BATTLER_NUM MAX_SELF_BATTLER_NUM+MAX_ENEMY_BATTLER_NUM
#define KEY_INFO_LINE_NUM 2

#define INFO_TYPE_ATTR 1
#define INFO_TYPE_INNATE 2

#define battle_report(format, ...) scroll_print(tk_scroll, format, ##__VA_ARGS__)
#define roll_battle_report(direction) scroll_direction(tk_scroll, direction)

extern scroll_t *tk_scroll;

int app_tui_init(void);
int app_tui_final(void);

int game_window_draw(void);
void draw_battle_info(void);

void show_battler(void);
int show_attr_info(battle_type_t battler_type, int battler_position);
int show_innate_info(battle_type_t battler_type, int battler_position);

int battler_highlight(battle_type_t battler_type, int battler_position, attr_t color_attr, short color_index);
int revert_battler_info_screen(void);

int battler_blink_delay(battle_type_t battler_type, int battler_position, int blink_times, short font_color);

int select_enemy_tui(int self_idx);

tk_position_state_t get_battler_position_state(battle_type_t battler_type, int position);
int generate_position(battle_type_t battler_type);

extern battler_t* get_battlers(void);
extern int get_battlers_num(void);

#endif