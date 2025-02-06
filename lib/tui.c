#include <locale.h>
#include <memory.h>

#include "tui.h"
#include "data_handle.h"

#define MAX_SAVE_LINES 9999

void tui_init()
{
    setlocale(LC_ALL, "");
    initscr();
    keypad(stdscr, TRUE); // 启用 keypad 模式
    curs_set(0);          // 隐藏光标  
    cbreak();
    noecho();
    if (!has_colors()) {
        log_warn("Your terminal does not support color\n");
    }
    start_color();

    init_pair(0,COLOR_BLACK,COLOR_BLACK);
    init_pair(1,COLOR_RED,COLOR_BLACK);
    init_pair(2,COLOR_GREEN,COLOR_BLACK);
    init_pair(3,COLOR_YELLOW,COLOR_BLACK);
    init_pair(4,COLOR_BLUE,COLOR_BLACK);
    init_pair(5,COLOR_MAGENTA,COLOR_BLACK);
    init_pair(6,COLOR_CYAN,COLOR_BLACK);
    init_pair(7,COLOR_WHITE,COLOR_BLACK);
}

int color_to_index(char* color_name)
{
    if(strcmp(color_name, "black") == 0){
        return 0;
    }else if(strcmp(color_name, "red") == 0){
        return 1;
    }else if(strcmp(color_name, "green") == 0){
        return 2;
    }else if(strcmp(color_name, "yellow") == 0){
        return 3;
    }else if(strcmp(color_name, "blue") == 0){
        return 4;
    }else if(strcmp(color_name, "magenta") == 0){
       return 5;
    }else if(strcmp(color_name, "cyan") == 0){
        return 6;
    }else if(strcmp(color_name, "white") == 0){
        return 7;
    }else{ 
        return TG_ERROR;
    }
}

const char* color_to_string(int index)
{
    switch(index){
        case 0:
            return "black";
        case 1:
            return "red";
        case 2:
            return "green";
        case 3:
            return "yellow";
        case 4:
            return "blue";
        case 5:
            return "magenta";
        case 6:
            return "cyan";
        case 7:
            return "white";
        default:
            return "unknown";
    }
}

void tui_final()
{
    endwin();
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

scroll_t* new_scroll_obj(WINDOW* scr, int scr_col, int scr_line, bool has_border)
{
    scroll_t* scroll = (scroll_t*)tg_malloc(sizeof(scroll_t));
    if(scroll == NULL){
        log_error("Failed to alloc memory for scroll_t");
        return NULL;
    }
    memset(scroll, 0, sizeof(scroll_t));
    scroll->screen = scr;
    scroll->fisrt_display_line_idx = 0;
    scroll->total_str_lines = 0;
    scroll->line_number_str_len = count_digits(MAX_SAVE_LINES)+1;
    if(has_border){
        scroll->max_display_lines = scr_line-2;
        scroll->max_display_cols = scr_col-2;
        //最大支持9999行
        scroll->line_width = scroll->max_display_cols+1 + scroll->line_number_str_len;
        
    }else{
        scroll->max_display_lines = scr_line;
        scroll->max_display_cols = scr_col;
        scroll->line_width = scr_col+scroll->line_number_str_len+1;
        scroll->str = (chtype*)tg_malloc(sizeof(chtype) * scroll->max_display_lines * scroll->line_width);
    }

    scroll->str = (chtype*)tg_malloc(sizeof(chtype) * scroll->max_display_lines * scroll->line_width);
    if(scroll->str == NULL){
        log_error("Failed to alloc memory for string of scroll ");
        return NULL;
    }

    return scroll;   
}

int del_scroll_obj(scroll_t* scroll)
{
    if(scroll == NULL){
        return TG_OK;
    }

    if(scroll->str != NULL){
        tg_free(scroll->str);
    }

    tg_free(scroll);

    return TG_OK;
}

/**
 * 滚屏显示函数，用于将一行字符打印到指定的窗口中
 * 
 * 此函数负责将一行字符串打印到指定的窗口中它需要确保信息的长度不超过窗口的最大列数，
 * 并且在必要时重新分配内存以容纳更多的信息，如果显示的行数达到窗口的最大行数，则会向上滚动一行屏幕内容
 * 
 * @param chstr 要打印的带属性的字符数组
 * @param len 要打印的字符数组的长度
 * 
 * @return 成功返回打印的字符数，失败返回TG_ERROR
 */
int scroll_print_line(scroll_t* scroll, chtype* chstr, size_t len)
{
    WINDOW* win = NULL;
    int print_line_idx = 0;
    int displayed_lines = 0;
    chtype* print_str = NULL;
    int max_display_cols = 0;
    int max_display_lines = 0;
    int buffer_line_width = 0;

    if(scroll == NULL){
        log_error("scroll is NULL");
        return TG_ERROR;
    }

    if(scroll->str == NULL){
        log_error("scroll->str is NULL");
        return TG_ERROR;
    }

    if(scroll->screen == NULL){
        log_error("scroll->screen is NULL");
        return TG_ERROR;
    }

    if(chstr == NULL || len == 0){
        log_error("chstr is NULL or len is 0");
        return TG_ERROR;
    }

    if(scroll->total_str_lines >= MAX_SAVE_LINES){
        log_warn("The number of lines to be displayed exceeds the maximum limit, %d", MAX_SAVE_LINES);
        return TG_ERROR;
    }

    win = scroll->screen;
    max_display_cols = scroll->max_display_cols;
    max_display_lines = scroll->max_display_lines;
    buffer_line_width = scroll->line_width;

    if(chstr == NULL || len > max_display_cols){
        return TG_ERROR;
    }

    //已申请内容使用完毕，申请一次容纳屏幕内容的内存
    if( scroll->total_str_lines % max_display_lines == 0 && scroll->total_str_lines > 0){
        int request_line_num = scroll->total_str_lines+max_display_lines;

        chtype* temp = (chtype*)tg_realloc(scroll->str, sizeof(chtype)*request_line_num*buffer_line_width);
        if(temp == NULL){
            endwin();
            log_error("fatal error: %s\n", strerror(errno));
            exit(1);
        }
        scroll->str = temp;
    }

    print_str = scroll->str + buffer_line_width*scroll->total_str_lines;
    //处理行号
    {
        char line_number_str[6] = "";
        int index = scroll->line_number_str_len;
        sprintf(line_number_str, "%*d ", scroll->line_number_str_len-1, scroll->total_str_lines+1);
        while(index--){
            print_str[index] = (chtype)line_number_str[index] | COLOR_PAIR(3);
        }
    }
    memcpy(print_str + scroll->line_number_str_len, chstr, sizeof(chtype)*len);
    displayed_lines = scroll->total_str_lines - scroll->fisrt_display_line_idx;

    //将屏幕边框内的内容上移一行
    if(displayed_lines == max_display_lines){
        scroll->fisrt_display_line_idx++;
        werase(win);
        box(win, 0, 0);
        int fresh_line_idx = scroll->fisrt_display_line_idx;
        for(int i = 0; i< max_display_lines-1; i++, fresh_line_idx++){
            mvwaddchstr(win,i+1,1, scroll->str + buffer_line_width*fresh_line_idx);
        }
        wrefresh(win);
    }else if(displayed_lines > max_display_lines){
        //当前显示的内容不是最新的几行，直接返回，这种情况是因为滚动了屏幕，显示的之前的内容
        scroll->total_str_lines++;
        return len;
    }

    //打印到屏幕
    print_line_idx = scroll->total_str_lines - scroll->fisrt_display_line_idx +1;
    mvwaddchstr(win,print_line_idx,1,print_str);

    scroll->total_str_lines++;

    return len;    
}

/**
 * 用于解析并格式化输入的字符串, 当前不支持中文
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
int scroll_print(scroll_t* scroll, const char* format, ...)
{

    chtype* print_str = NULL;
    size_t print_len = 0;
    int max_display_cols = 0;

    int input_str_len = 0;
    int i,j;
    short color_pair_id = 0;
    char* p = NULL;
    chtype* end= NULL;
    
    if(scroll == NULL){
        log_error("scroll is NULL");
        return TG_ERROR;
    }

    char buffer[1024] = "";
    chtype chtype_buffer[1024] ={};

    va_list args;
    va_start(args, format);
    input_str_len = vsnprintf(buffer, 1024, format, args);
    va_end(args);

    max_display_cols = scroll->max_display_cols;

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
            scroll_print_line(scroll, print_str, print_len);
            print_str += print_len;
        }else{
            print_len = end-print_str;
            scroll_print_line(scroll, print_str, print_len);
            break;
        }
    }
    wrefresh(scroll->screen);

    return i;
}

/**
 * 滚动屏幕的显示内容
 * 
 * @param direction 滚动方向，'w' 表示向上滚动，'s' 表示向下滚动
 * @return 返回执行结果，0 表示成功，其他值表示失败
 * 
 * 此函数根据用户输入的方向键滚动战斗报告的显示内容它通过修改首行显示的索引来实现滚动效果，
 * 并重新绘制屏幕以反映内容的变更,如果已经达到滚动的边界，则函数返回 TG_OK，
 * 表示无需进行进一步操作
 */
int scroll_direction(scroll_t* scroll, int direction)
{
    chtype* print_str = NULL;

    if(scroll->total_str_lines < scroll->max_display_lines){
        return TG_OK;
    }

    if(direction == 'w'){
        if(scroll->fisrt_display_line_idx <= 0){
            return TG_OK;
        }

        scroll->fisrt_display_line_idx--;
    }else if(direction == 's'){
        if(scroll->fisrt_display_line_idx >= scroll->total_str_lines-scroll->max_display_lines){
            return TG_OK;
        }

        scroll->fisrt_display_line_idx++;
    }else{
        log_error("invalid direction: %c\n", direction);
        return TG_ERROR;
    }

    werase(scroll->screen);
    box(scroll->screen, 0, 0);
    print_str = scroll->str + scroll->line_width*scroll->fisrt_display_line_idx;
    for(int i = 0; i<scroll->max_display_lines; i++){
        mvwaddchstr(scroll->screen,i+1,1,print_str);
        print_str += scroll->line_width;
    }
    wrefresh(scroll->screen);

    return 0;
}