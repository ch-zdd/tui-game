#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib/tui.h"

#define WIDTH 20
#define HEIGHT 10
#define LENGTH 10
#define MOVE_UP (-WIDTH)
#define MOVE_DOWN WIDTH
#define MOVE_LEFT (-1)
#define MOVE_RIGHT 1

void game_run_2(void)
{
    nodelay(stdscr, 1); // 设置为非阻塞模式（1表示启用）
    int board[HEIGHT * WIDTH] = {0}; // 游戏界面
    int queue[LENGTH] = {0}; //蛇坐标循环队列，这里定义蛇最大长度为10
    int i = 0, length = 1, tail = 0, food = 0, key = 0, direction = MOVE_RIGHT, next = 0;
    do{board[i] = (i/WIDTH == 0 || i/WIDTH == HEIGHT-1 || i % WIDTH == 0 || i % WIDTH == WIDTH-1) ? 1 : 0;}while(i++ < HEIGHT*WIDTH); // 边界定义
    board[queue[0] = (HEIGHT/2)*WIDTH + WIDTH/2] = 1; // 初始长度 1， 画面中央创建蛇
    do{food = WIDTH + 1 + rand() % ((HEIGHT-2) * (WIDTH-2));}while(board[food] == 1); board[food] = 1;  //初始食物位置
    do{ key = getch();
        // 按键控制
        if(key == 'a' && direction != MOVE_RIGHT) direction = MOVE_LEFT;
        else if(key == 'd' && direction != MOVE_LEFT) direction = MOVE_RIGHT;
        else if(key == 'w' && direction != MOVE_DOWN) direction = MOVE_UP;
        else if(key == 's' && direction != MOVE_UP)  direction = MOVE_DOWN;
        next = queue[(tail+length-1)%LENGTH] + direction; 
        if(food == next) {
            // 吃食物
            if(length >= LENGTH) break; 
            queue[(tail+(++length)-1)%LENGTH] = next; 
            do{food = WIDTH + 1 + rand() % ((HEIGHT-2) * (WIDTH-2));}while(board[food] == 1); 
            board[food] = 1;
        }else if(board[next] == 1) break; // 撞墙结束游戏
        else {
            // 移动
            board[queue[tail]] = 0; 
            queue[((tail = (tail+1)%LENGTH)+length-1)%LENGTH] = next;
        }
        // 蛇体入库
        for(i = tail; i < tail+length; i++) board[queue[i%LENGTH]] = 1;
        // 绘制游戏界面
        for(i = 0; i < HEIGHT*WIDTH; i++) mvwaddch(stdscr, i/WIDTH, i%WIDTH, board[i] ? (i == food ? '@' : '#') : ' ');
        refresh();
        usleep(1000000);
    }while(key != 'q');
}

void game_final_2(void)
{
    ;
}