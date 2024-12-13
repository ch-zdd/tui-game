#include <string.h>
#include <ncurses.h>
#include <locale.h>

#include "app-tui.h"
#include "../lib/data_handle.h"

struct {
    WINDOW* screen;
    chtype* str;
    int line_width;
    int max_display_lines;
    int max_display_cols;

    int total_str_lines;    
    int fisrt_display_line_idx;
} battle_report_screen;

int app_tui_init(void)
{
    tk_tui_t* tui = &(get_app_context()->tui);

    setlocale(LC_ALL, "");
    initscr();
    curs_set(0);          // 隐藏光标  
    cbreak();
    noecho();
    if (!has_colors()) {
        log_warn("Your terminal does not support color\n");
    }
    start_color();
    init_pair(0,COLOR_WHITE,COLOR_BLACK);
    init_pair(1,COLOR_RED,COLOR_BLACK);
    init_pair(2,COLOR_BLUE,COLOR_BLACK);
    init_pair(3,COLOR_YELLOW,COLOR_BLACK);

    getmaxyx(stdscr, tui->main.scr_line, tui->main.scr_col);
    if(tui->main.scr_line < 16 || tui->main.scr_col < 70){
        printw("screen size(line:%d, at least 16; column:%d, at least 70) too small, press any key to exit...", tui->main.scr_line, tui->main.scr_col);
        getch();
        return TK_ERROR;
    }
    tui->main.active = true;
    tui->main.w = stdscr;
    if(stdscr == NULL){
        tui->main.active = false;
        log_error("Failed to inti stdscr");
        return TK_ERROR;
    }
    strncpy(tui->main.store_path, "main_window.scr", sizeof(tui->main.store_path));

    tui->battle_report.active = false;
    tui->battler_info.active = false;
    tui->battle.active = false;
    strncpy(tui->battle_report.store_path, "battle_report.scr", sizeof(tui->main.store_path));
    strncpy(tui->battler_info.store_path, "battler_info.scr", sizeof(tui->main.store_path));
    strncpy(tui->battle.store_path, "battle.scr", sizeof(tui->main.store_path));

    return TK_OK;
}

int app_tui_final(void)
{
    tk_free(battle_report_screen.str);
    delwin(stdscr);
    endwin();

    return TK_OK;
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
        return TK_ERROR;
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

    memset(&battle_report_screen, 0, sizeof(battle_report_screen));
    battle_report_screen.max_display_lines = battle_report->scr_line-2;
    battle_report_screen.total_str_lines = 0;
    battle_report_screen.fisrt_display_line_idx = 0;
    battle_report_screen.screen = battle_report->w;
    battle_report_screen.max_display_cols = battle_report->scr_col-2;
    battle_report_screen.line_width = battle_report->scr_col-1;
    battle_report_screen.str = (chtype*)tk_malloc(sizeof(chtype)*battle_report_screen.max_display_lines*battle_report_screen.line_width);

    if(battle_report_screen.str == NULL){
        log_error("Failed to alloc memory for battle report screen");
        return TK_ERROR;
    }

    return TK_OK;
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
        return TK_ERROR;
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

    return TK_OK;
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
        return TK_ERROR;
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
    return TK_OK;
}

int revert_battler_info_screen(void)
{
    tk_window_t* bw = &(get_app_context()->tui.battle);

    mvwchgat(bw->w, 1, 1, bw->scr_col-2, A_NORMAL, 0, NULL);
    mvwchgat(bw->w, 2, 1, bw->scr_col-2, A_NORMAL, 0, NULL);
    mvwchgat(bw->w, bw->scr_line - 3, 1, bw->scr_col-2, A_NORMAL, 0, NULL);
    mvwchgat(bw->w, bw->scr_line - 2, 1, bw->scr_col-2, A_NORMAL, 0, NULL);

    wrefresh(bw->w);
    return TK_OK;
}

int battler_highlight(battle_type_t battler_type, int battler_position, attr_t color_attr, short color_index)
{
    tk_window_t* bw = NULL;
    int line = 0;
    int col = 0;
    int width = 0;

    if(get_battler_position_state(battler_type, battler_position) == POSITION_STATE_INVALID){
        log_debug("battler_type %d, position %d invalid", battler_type, battler_position);
        return TK_NOT_FOUND;
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
        return TK_ERROR;
    }

    mvwchgat(bw->w, line, col, width-1, color_attr, color_index, NULL);
    mvwchgat(bw->w, line+1, col, width-1, color_attr, color_index, NULL);
    wrefresh(bw->w);

    return TK_OK;
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
        return TK_NOT_FOUND;
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
        return TK_ERROR;
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

    return TK_OK;
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
        return TK_ERROR;
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

    return TK_NOT_FOUND;
}

void wchar_blink(WINDOW* win, int line, int col, int n, int blink_times)
{
    int i = 0;
    while(i< blink_times){
        mvwchgat(win, line, col, n, A_STANDOUT, 0, NULL);
        wrefresh(win);

        usleep(400000);

        mvwchgat(win, line, col, n, A_NORMAL, 0, NULL);
        wrefresh(win);

        usleep(400000);
        i++;
    }
}

/**
 * 打印战斗报告的一行信息
 * 
 * 此函数负责将一行战斗报告信息打印到指定的窗口中它需要确保信息的长度不超过窗口的最大列数，
 * 并且在必要时重新分配内存以容纳更多的信息，如果显示的行数达到窗口的最大行数，则会向上滚动一行屏幕内容
 * 
 * @param chstr 要打印的带属性的字符数组
 * @param len 要打印的字符数组的长度
 * 
 * @return 成功返回打印的字符数，失败返回TK_ERROR
 */
int print_battle_report_line(chtype* chstr, size_t len)
{
    WINDOW* win = NULL;
    int print_line_idx = 0;
    int displayed_lines = 0;
    chtype* print_str = NULL;
    int max_display_cols = 0;
    int max_display_lines = 0;
    int buffer_line_width = 0;


    win = battle_report_screen.screen;
    max_display_cols = battle_report_screen.max_display_cols;
    max_display_lines = battle_report_screen.max_display_lines;
    buffer_line_width = battle_report_screen.line_width;

    if(chstr == NULL || len > max_display_cols){
        return TK_ERROR;
    }

    //已申请内容使用完毕，申请一次容纳屏幕内容的内存
    if( battle_report_screen.total_str_lines % max_display_lines == 0 && battle_report_screen.total_str_lines > 0){
        int request_line_num = battle_report_screen.total_str_lines+max_display_lines;

        chtype* temp = (chtype*)tk_realloc(battle_report_screen.str, sizeof(chtype)*request_line_num*buffer_line_width);
        if(temp == NULL){
            endwin();
            log_error("fatal error: %s\n", strerror(errno));
            exit(1);
        }
        battle_report_screen.str = temp;
    }

    print_str = battle_report_screen.str + buffer_line_width*battle_report_screen.total_str_lines;
    memcpy(print_str, chstr, sizeof(chtype)*len);
    displayed_lines = battle_report_screen.total_str_lines - battle_report_screen.fisrt_display_line_idx;

    //将屏幕边框内的内容上移一行
    if(displayed_lines == max_display_lines){
        battle_report_screen.fisrt_display_line_idx++;
        werase(win);
        box(win, 0, 0);
        int fresh_line_idx = battle_report_screen.fisrt_display_line_idx;
        for(int i = 0; i< max_display_lines-1; i++, fresh_line_idx++){
            mvwaddchstr(win,i+1,1,battle_report_screen.str + buffer_line_width*fresh_line_idx);
        }
        wrefresh(win);
    }

    //打印到屏幕
    print_line_idx = battle_report_screen.total_str_lines - battle_report_screen.fisrt_display_line_idx +1;
    mvwaddchstr(win,print_line_idx,1,print_str);

    battle_report_screen.total_str_lines++;

    return len;    
}

/**
 * 战报输出函数，用于格式化并显示战斗报告信息,当前不支持中文
 * 
 * @param format 格式字符串，可以包含颜色代码和普通文本
 * @param ... 可变参数列表，根据格式字符串解析
 * 
 * @return 返回解析后的字符串长度
 * 
 * 该函数负责解析输入的格式字符串，应用颜色设置，并将结果分多行显示在屏幕上
 * 格式字符串中，颜色代码以 '$' 开头，后跟颜色对索引和括号内的文本，例如: $1{红色文本}
 * 如果要打印符号'$',应输入"$$"
 */
int battle_report(const char* format, ...)
{

    chtype* print_str = NULL;
    size_t print_len = 0;
    int max_display_cols = 0;

    int input_str_len = 0;
    int i,j;
    short color_pair_id = 0;
    char* p = NULL;
    chtype* end= NULL;
    

    char buffer[1024] = "";
    chtype chtype_buffer[1024] ={};

    va_list args;
    va_start(args, format);
    input_str_len = vsnprintf(buffer, 1024, format, args);
    va_end(args);

    max_display_cols = battle_report_screen.max_display_cols;

    //转换为ncurses的chtype
    for(i = 0, j = 0; j<input_str_len && i<1024; i++,j++){
        
        if(buffer[j] !='$'){
            chtype_buffer[i] = (chtype)buffer[j];
            continue;
        }

        //解析带颜色属性的字符串
        j++;
        if(j+3 < input_str_len && IS_CH_NUM(buffer[j]) && buffer[j+1]=='{'){
            color_pair_id = buffer[j]-'0';

            //定位到颜色字符串位置
            j = j+2;
            p = strchr(buffer+j, '}');
            if(p == NULL){
                //格式错误
                break;
            }

            //开始应用颜色设置
            for(; j<(p-buffer); j++,i++){
                chtype_buffer[i] = (chtype)buffer[j] | COLOR_PAIR(color_pair_id);
            }
            i--;
        }else if(buffer[j]=='$'){
            //打印符号'$'
            chtype_buffer[i] = (chtype)buffer[j];
        }else{
            //格式错误
            break;
        }
    }

    //多行处理
    end = chtype_buffer+i;
    print_str = chtype_buffer;
    while(1){
        if(print_str+max_display_cols <end){
            print_len = (size_t)max_display_cols;
            print_battle_report_line(print_str, print_len);
            print_str += print_len;
        }else{
            print_len = end-print_str;
            print_battle_report_line(print_str, print_len);
            break;
        }
    }
    wrefresh(battle_report_screen.screen);

    return i;
}

/**
 * 滚动战斗报告屏幕的显示内容
 * 
 * @param direction 滚动方向，'w' 表示向上滚动，'s' 表示向下滚动
 * @return 返回执行结果，0 表示成功，其他值表示失败
 * 
 * 此函数根据用户输入的方向键滚动战斗报告的显示内容它通过修改首行显示的索引来实现滚动效果，
 * 并重新绘制屏幕以反映内容的变更,如果已经达到滚动的边界，则函数返回 TK_OK，
 * 表示无需进行进一步操作
 */
int roll_battle_report(int direction)
{
    chtype* print_str = NULL;

    if(direction == 'w'){
        if(battle_report_screen.fisrt_display_line_idx == 0){
            return TK_OK;
        }

        battle_report_screen.fisrt_display_line_idx--;
    }else if(direction == 's'){
        if(battle_report_screen.fisrt_display_line_idx == battle_report_screen.total_str_lines-battle_report_screen.max_display_lines){
            return TK_OK;
        }

        battle_report_screen.fisrt_display_line_idx++;
    }else{
        return TK_ERROR;
    }

    werase(battle_report_screen.screen);
    box(battle_report_screen.screen, 0, 0);
    print_str = battle_report_screen.str + battle_report_screen.line_width*battle_report_screen.fisrt_display_line_idx;
    for(int i = 0; i<battle_report_screen.max_display_lines; i++){
        mvwaddchstr(battle_report_screen.screen,i+1,1,print_str);
        print_str += battle_report_screen.line_width;
    }
    wrefresh(battle_report_screen.screen);

    return 0;
}