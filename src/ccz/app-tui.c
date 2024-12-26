#include <string.h>
#include <ncurses.h>

#include "app-tui.h"
#include "../../lib/data_handle.h"

scroll_t *tk_scroll = NULL;

int app_tui_init(void)
{
    tui_init();
    tk_tui_t* tui = &(get_app_context()->tui);

    getmaxyx(stdscr, tui->main.scr_line, tui->main.scr_col);
    if(tui->main.scr_line < 16 || tui->main.scr_col < 70){
        printw("screen size(line:%d, at least 16; column:%d, at least 70) too small, press any key to exit...", tui->main.scr_line, tui->main.scr_col);
        getch();
        return TG_ERROR;
    }
    tui->main.active = true;
    tui->main.w = stdscr;
    if(stdscr == NULL){
        tui->main.active = false;
        log_error("Failed to inti stdscr");
        return TG_ERROR;
    }
    strncpy(tui->main.store_path, "main_window.scr", sizeof(tui->main.store_path));

    tui->battle_report.active = false;
    tui->battler_info.active = false;
    tui->battle.active = false;
    strncpy(tui->battle_report.store_path, "battle_report.scr", sizeof(tui->main.store_path));
    strncpy(tui->battler_info.store_path, "battler_info.scr", sizeof(tui->main.store_path));
    strncpy(tui->battle.store_path, "battle.scr", sizeof(tui->main.store_path));

    return TG_OK;
}

int app_tui_final(void)
{
    del_scroll_obj(tk_scroll);
    delwin(stdscr);
    tui_final();

    return TG_OK;
}

int game_window_draw(void)
{
    tk_window_t* main = &(get_app_context()->tui.main);
    tk_window_t* battler_info = &(get_app_context()->tui.battler_info);
    tk_window_t* battle_report = &(get_app_context()->tui.battle_report);
    tk_window_t* battle = &(get_app_context()->tui.battle);
    int maxline = main->scr_line-KEY_INFO_LINE_NUM; //预留几行显示按键说明
    int maxcol = main->scr_col;
    int battle_col = get_app_context()->battle_window_width;
    if(main->active == false){
        log_error("ERROR: tui is not initialized");
        return TG_ERROR;
    }

    mvaddstr(main->scr_line/2, main->scr_col/2 - 12, "Press any key to start...");
    getch();
    scr_dump(main->store_path);
 
    battle->scr_line = maxline;
    battle->scr_col = battle_col; 
    battle->w = newwin(battle->scr_line, battle->scr_col, 0, 0);
    box(battle->w, 0, 0);
    wrefresh(battle->w);
    battle->active = true;

    battler_info->scr_line = maxline/2;
    battler_info->scr_col = maxcol - battle->scr_col; 
    battler_info->w = newwin(battler_info->scr_line, battler_info->scr_col, 0, battle->scr_col);
    box(battler_info->w, 0, 0);
    wrefresh(battler_info->w);
    battler_info->active = true;

    battle_report->scr_line = maxline - battler_info->scr_line;
    battle_report->scr_col = maxcol - battle->scr_col;
    battle_report->w = newwin(battle_report->scr_line, battle_report->scr_col, battler_info->scr_line, battle->scr_col);
    box(battle_report->w, 0, 0);
    wrefresh(battle_report->w);
    battle_report->active = true;

    log_debug("battle line %d, col %d, enemy line %d, col %d, self line %d, col %d", battle->scr_line, battle->scr_col, battler_info->scr_line, battler_info->scr_col, battle_report->scr_line, battle_report->scr_col);
    
    tk_scroll = new_scroll_obj(battle_report->w, battle_report->scr_col, battle_report->scr_line, true);
    if(tk_scroll == NULL){
        log_error("Failed to create scroll obj");
        return TG_ERROR;
    }

    return TG_OK;
}

void show_battler()
{
    tk_tui_t* tui = &(get_app_context()->tui);
    int battler_idx = 0;
    int enemy_idx = 0;
    int self_idx = 0;
    int line = 0;
    int col = 0;
    int battler_width = (tui->battle.scr_col-2)/MAX_ENEMY_BATTLER_NUM;
    int battler_position = 0;
    int battlers_num = get_battlers_num();
    battler_t* battlers = get_battlers();

    WINDOW* battle_w = tui->battle.w;

    for(battler_idx = 0; battler_idx<battlers_num && battler_idx < MAX_BATTLER_NUM; battler_idx++){
        battler_position = battlers[battler_idx].position;
        col = (battler_position-1)*battler_width+1;
       if(battler_position <=0){
            log_error("Invalid battler position %d", battler_position);
            return;
        }

        if(battlers[battler_idx].type == BATTLE_TYPE_ENEMY){
            line = 1;
            enemy_idx++;
        }else if (battlers[battler_idx].type == BATTLE_TYPE_SELF ){
            line = tui->battle.scr_line - 3;
            self_idx++;
        }else{
            log_error("Invalid battler type %d", battlers[battler_idx].type);
            return;
        }

        if(enemy_idx > MAX_ENEMY_BATTLER_NUM || self_idx > MAX_SELF_BATTLER_NUM){
            log_error("Max 3 self battler or enemy battler, enemy = %d, self = %d", enemy_idx, self_idx);
            return;
        }

        mvwprintw(battle_w, line, col, "[%s]", battlers[battler_idx].name);
        mvwprintw(battle_w, line+1, col, "[%d]", battlers[battler_idx].hp);
    }
    wrefresh(battle_w);
}

int show_attr_info(battle_type_t battler_type, int battler_position)
{
    int battlers_num = get_battlers_num();
    battler_t* battlers = get_battlers();
    WINDOW* info_w = NULL;
    int i = 0;

    for(i = 0; i < battlers_num; i++){
        if(battlers[i].type == battler_type && battlers[i].position == battler_position){
            break;
        }
    }
    if(i == battlers_num){
        log_debug("Battler not found, %d ,%d", battler_type, battler_position);
        return TG_ERROR;
    }

    info_w = get_app_context()->tui.battler_info.w;

    wclrctx(info_w);
    mvwprintw(info_w, 1, 1, "%s hp%d/%d mp%d/%d", battlers[i].name, battlers[i].hp, battlers[i].hp_max, battlers[i].mp, battlers[i].mp_max);
    mvwprintw(info_w, 2, 1, "武[%d/%d] 智[%d/%d] 防[%d/%d]",
        battlers[i].force.base_value, battlers[i].force.current_value,
        battlers[i].intelligence.base_value, battlers[i].intelligence.current_value, 
        battlers[i].defense.base_value, battlers[i].defense.current_value);
    
    mvwprintw(info_w, 3, 1, "爆[%d/%d] 士[%d/%d]  速[%d/%d] ",
        battlers[i].agile.base_value, battlers[i].agile.current_value, 
        battlers[i].morale.base_value, battlers[i].morale.current_value, 
        battlers[i].speed.base_value, battlers[i].speed.current_value);

    wrefresh(info_w);

    return TG_OK;
}

int show_innate_info(battle_type_t battler_type, int battler_position)
{
    WINDOW* info_w = NULL;
    int i = 0;
    int j = 0;
    int battlers_num = get_battlers_num();
    battler_t* battlers = get_battlers();

    for(i = 0; i < battlers_num; i++){
        if(battlers[i].type == battler_type && battlers[i].position == battler_position){
            break;
        }
    }
    if(i == battlers_num){
        return TG_ERROR;
    }

    info_w = get_app_context()->tui.battler_info.w;

    tk_loaded_innate_t innate = battlers[i].loaded_innate;
    wclrctx(info_w);
    mvwprintw(info_w, 1, 1, "%s hp%d/%d mp%d/%d", battlers[i].name, battlers[i].hp, battlers[i].hp_max, battlers[i].mp, battlers[i].mp_max);
    for(i = 1;  i< SKILL_MAX; i++){
        if(!((innate.innate_mask >>i) & 0x01)){
            continue;
        }
        j++;
        if(strlen(innate.innate_skill[i].name) != 0){
            mvwprintw(info_w, j+1, 1, "innate %d: %s", j, innate.innate_skill[i].name);
        }
    }
    wrefresh(info_w);
    return TG_OK;
}

int revert_battler_info_screen(void)
{
    tk_window_t* bw = &(get_app_context()->tui.battle);

    mvwchgat(bw->w, 1, 1, bw->scr_col-2, A_NORMAL, 0, NULL);
    mvwchgat(bw->w, 2, 1, bw->scr_col-2, A_NORMAL, 0, NULL);
    mvwchgat(bw->w, bw->scr_line - 3, 1, bw->scr_col-2, A_NORMAL, 0, NULL);
    mvwchgat(bw->w, bw->scr_line - 2, 1, bw->scr_col-2, A_NORMAL, 0, NULL);

    wrefresh(bw->w);
    return TG_OK;
}

int battler_highlight(battle_type_t battler_type, int battler_position, attr_t color_attr, short color_index)
{
    tk_window_t* bw = NULL;
    int line = 0;
    int col = 0;
    int width = 0;

    if(get_battler_position_state(battler_type, battler_position) == POSITION_STATE_INVALID){
        log_debug("battler_type %d, position %d invalid", battler_type, battler_position);
        return TG_NOT_FOUND;
    }

    bw = &(get_app_context()->tui.battle);
    width = (bw->scr_col-2)/MAX_ENEMY_BATTLER_NUM;
    col = (battler_position-1)*width+1;

    if(battler_type == BATTLE_TYPE_ENEMY){
        line = 1;
        
    }else if(battler_type == BATTLE_TYPE_SELF){
        line = bw->scr_line - 3;
    }else{
        log_error("Invalid battler type %d", battler_type);
        return TG_ERROR;
    }

    mvwchgat(bw->w, line, col, width-1, color_attr, color_index, NULL);
    mvwchgat(bw->w, line+1, col, width-1, color_attr, color_index, NULL);
    wrefresh(bw->w);

    return TG_OK;
}

int battler_blink_delay(battle_type_t battler_type, int battler_position, int blink_times, short font_color)
{
    tk_window_t* bw = NULL;
    int line = 0;
    int col = 0;
    int width = 0;
    int i = 0;

    if(get_battler_position_state(battler_type, battler_position) == POSITION_STATE_INVALID){
        log_debug("battler_type %d, position %d invalid", battler_type, battler_position);
        return TG_NOT_FOUND;
    }

    bw = &(get_app_context()->tui.battle);
    width = (bw->scr_col-2)/MAX_ENEMY_BATTLER_NUM;
    col = (battler_position-1)*width+1;

    if(battler_type == BATTLE_TYPE_ENEMY){
        line = 1;
        
    }else if(battler_type == BATTLE_TYPE_SELF){
        line = bw->scr_line - 3;
    }else{
        log_error("Invalid battler type %d", battler_type);
        return TG_ERROR;
    }

    while(i < blink_times){
        mvwchgat(bw->w, line, col, width-1, A_STANDOUT, 0, NULL);
        mvwchgat(bw->w, line+1, col, width-1, A_STANDOUT, 0, NULL);
        wrefresh(bw->w);
        usleep(500000);
        mvwchgat(bw->w, line, col, width-1, A_NORMAL, 0, NULL);
        mvwchgat(bw->w, line+1, col, width-1, A_NORMAL, 0, NULL);
        wrefresh(bw->w);
        usleep(500000);
        i++;
    }

    return TG_OK;
}

tk_position_state_t get_battler_position_state(battle_type_t battler_type, int position)
{
    int battlers_num = get_battlers_num();
    battler_t* battlers = get_battlers();
    int i = 0;

    for(i = 0; i < battlers_num; i++){
        if(battlers[i].type == battler_type && battlers[i].position == position){
            if(battlers[i].hp <= 0){
                return POSITION_STATE_DEAD;
            }else{
                return POSITION_STATE_ALIVE;
            }
        }
    }

    return POSITION_STATE_INVALID;
}

int select_enemy_tui(int self_idx)
{
    bool loop_flag_1 = true;
    bool loop_flag_2 = true;
    int battlers_num = get_battlers_num();
    battler_t* battlers = get_battlers();

    if(self_idx < 0 || self_idx >= battlers_num){
        return TG_ERROR;
    }

    int defender_position = 1; //默认选择的敌人
    int battler_info_idx = battlers[self_idx].position;
    int key = 0;
    int current_info_type = INFO_TYPE_ATTR;
    int maxline = get_app_context()->tui.main.scr_line;
    int maxcol = get_app_context()->tui.main.scr_col;

    battle_type_t current_battler_type = BATTLE_TYPE_SELF;
    clrzone(KEY_INFO_LINE_NUM, maxcol, maxline-KEY_INFO_LINE_NUM, 0);
    mvprintw(maxline-KEY_INFO_LINE_NUM, 0, "Current state: select defender");
    mvprintw(maxline-KEY_INFO_LINE_NUM+1, 0, "KEY: ballter:1;2;3 , q:quit, j:attack, c:info, w:scroll up, s:scroll down");
    refresh();
    battler_highlight(BATTLE_TYPE_ENEMY, defender_position, A_NORMAL, 2); //默认选择的敌人
    while(loop_flag_1){
        switch(key){
            case 'j':loop_flag_1 = false; continue; //退出当前页面
            case 'w':
            case 's':{
                roll_battle_report(key);
                break;
            }
            case '1':
            case '2':
            case '3':{
                if(get_battler_position_state(BATTLE_TYPE_ENEMY, key-'0') == POSITION_STATE_ALIVE){
                    battler_highlight(BATTLE_TYPE_ENEMY, defender_position, A_NORMAL, 0);
                    defender_position = key-'0';
                    battler_highlight(BATTLE_TYPE_ENEMY, defender_position, A_NORMAL, 2);
                }
                break;
            }
            case 'q': exit(EXIT_SUCCESS); //退出游戏
            case 'c':{
                clrzone(KEY_INFO_LINE_NUM, maxcol, maxline-KEY_INFO_LINE_NUM, 0);
                mvprintw(maxline-KEY_INFO_LINE_NUM, 0, "Current state: show %s info", current_battler_type == BATTLE_TYPE_SELF?"self":"enemy");
                mvprintw(maxline-KEY_INFO_LINE_NUM+1, 0, "KEY: ballter:1;2;3 , q:quit, l:return, c:self/enemy, k:attr/innate");
                refresh();
                key = ' ';
                //下级页面
                while(loop_flag_2){
                    switch(key){
                        case 'l': loop_flag_2 = false; continue;
                        case 'q': exit(EXIT_SUCCESS); //退出游戏
                        case 'c': {
                            //切换角色类型显示
                            if(current_battler_type == BATTLE_TYPE_ENEMY){
                                current_battler_type = BATTLE_TYPE_SELF;
                            }else if(current_battler_type == BATTLE_TYPE_SELF){
                                current_battler_type = BATTLE_TYPE_ENEMY;
                            }
                            battler_info_idx = generate_position(current_battler_type);
                            break;
                        }
                        case 'k':{
                            //切换查看角色天赋
                            if(current_info_type == INFO_TYPE_ATTR){
                                current_info_type = INFO_TYPE_INNATE;
                            }else if(current_info_type == INFO_TYPE_INNATE){
                                current_info_type = INFO_TYPE_ATTR;
                            }
                            break;
                        }
                        case '1':
                        case '2':
                        case '3':{
                            battler_info_idx = key-'0';
                            break;
                        }
                        default: break;
                    }

                    //查看角色天赋/属性
                    if(current_info_type == INFO_TYPE_ATTR){
                        show_attr_info(current_battler_type, battler_info_idx);
                    }else if(current_info_type == INFO_TYPE_INNATE){
                        show_innate_info(current_battler_type, battler_info_idx);
                    }else{
                        log_warn("Invalid info type");
                    }

                    key = getch();
                }

                clrzone(KEY_INFO_LINE_NUM, maxcol, maxline-KEY_INFO_LINE_NUM, 0);
                mvprintw(maxline-KEY_INFO_LINE_NUM, 0, "Current state: select defender");
                mvprintw(maxline-KEY_INFO_LINE_NUM+1, 0, "KEY: ballter:1;2;3 , q:quit, j:attack, c:info, w:scroll up, s:scroll down");
                refresh();
                break;
            }
            default:
                break;
        }

        key = getch();
    }

    clrzone(KEY_INFO_LINE_NUM, maxcol, maxline-KEY_INFO_LINE_NUM, 0);
    refresh();

    return defender_position;
}

int generate_position(battle_type_t battler_type)
{
    int battlers_num = get_battlers_num();
    battler_t* battlers = get_battlers();
    int i = 0;

    for(i = 0; i < battlers_num; i++){
        if(battlers[i].type != battler_type || battlers[i].hp <= 0 || battlers[i].position <= 0){
            continue;
        }
        
        if(battler_type == BATTLE_TYPE_SELF && battlers[i].position >= MAX_SELF_BATTLER_NUM){
            continue;
        }

        if(battler_type == BATTLE_TYPE_ENEMY && battlers[i].position >= MAX_ENEMY_BATTLER_NUM){
            continue;
        }

        return battlers[i].position;
    }

    return TG_NOT_FOUND;
}

