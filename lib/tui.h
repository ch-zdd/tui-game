
#ifndef TUI_H
#define TUI_H

#include <ncurses.h>
#include "common.h"

#define MAX_COLOR_LEN 16

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


typedef struct {
    WINDOW* screen;
    chtype* str;
    int line_width; //每行占用的字符数，大小一般为max_display_cols+1，多余的一个为字符结尾符
    int max_display_lines; //实际显示的行数
    int max_display_cols;   //实际显示的列数

    int line_number_str_len; //行号字符串长度
    int total_str_lines;    
    int fisrt_display_line_idx;
} scroll_t;

void tui_init();
void tui_final();

int color_to_index(char* color_name);
const char* color_to_string(int index);

void wchar_blink(WINDOW* win, int line, int col, int n, int blink_times);
scroll_t* new_scroll_obj(WINDOW* scr, int scr_col, int scr_line, bool has_border);
int del_scroll_obj(scroll_t* scroll);

int scroll_print_line(scroll_t* scroll, chtype* chstr, size_t len);
int scroll_print(scroll_t* scroll, const char* format, ...) CHECK_FMT(2,3);
int scroll_direction(scroll_t* scroll, int direction);
int str_to_chtype(str_t str, chtype* chstr, int color_index);

#endif