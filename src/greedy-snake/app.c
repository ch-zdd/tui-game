#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib/tui.h"
#include "../../lib/common.h"

#define input() getch()

// 绘制
#define draw_snake(x, y) do{mvwaddchstr(stdscr,y, x*3, ch_to_chtype("[*]", 1)); refresh();}while(0)
#define clear_snake(x, y) do{mvwaddchstr(stdscr,y, x*3, ch_to_chtype("   ", 1));refresh();}while(0)
#define draw_food(x, y) do{mvwaddchstr(stdscr,y, x*3, ch_to_chtype("[@]", 1));refresh();}while(0)
#define draw_wall(x, y) do{mvwaddchstr(stdscr,y, x*3, ch_to_chtype("+++", 1));}while(0)

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define clean_game() do{\
    if(board.score == 0){\
        break;\
    }\
    snake_t* tmp = Head;\
    while(1){\
        node_t* next_node = tmp->node.next;\
        tg_free(tmp);\
        if(next_node == NULL) break;\
        tmp = container_of(next_node, snake_t, node);\
    }\
    board.score = 0;\
}while(0)

typedef struct node_t{
    struct node_t* prev;
    struct node_t* next;
} node_t;

typedef struct {
    node_t node;
    int x;
    int y;
} snake_t;

enum{
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT
};

typedef struct {
    int direction;
    int window_width;
    int window_height;
    int score;
} board_t;

bool Is_running = true;
// 初始化屏幕
board_t board = {
        .direction = MOVE_RIGHT, // 移动方向
        .window_width = 20, // 列数
        .window_height = 10, // 行数
        .score = 1 // 分数,即蛇的长度
    };
// 标记头部节点
snake_t* Head = NULL;
snake_t* Tail = NULL;
// 当前食物位置
int Food_x = 0;
int Food_y = 0;
// 键盘监控线程
pthread_t Input_task_id;
// 互斥锁
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void new_food();
void* handle_input(void* data);
int move_snake_s();
void game_run(void)
{
    srand(time(NULL));
    // 初始化窗口
    int index = 0;
    for(index = 0; index < board.window_height; index++){
        draw_wall(board.window_width, index);
    }
    for(index = 0; index < board.window_width; index++){
        draw_wall(index, board.window_height);
    }
    refresh();
    // 初始化蛇
    snake_t* snake = tg_malloc(sizeof(snake_t));
    snake->x = 0;
    snake->y = 0;
    snake->node.prev = NULL;
    snake->node.next = NULL;
    Head = snake;
    Tail = snake;
    draw_snake(snake->x, snake->y);
    // 初始化食物
    new_food();

    // 启动键盘监控
    if(0 != pthread_create(&Input_task_id, NULL, handle_input, NULL)){
        log_error("Failed to start task for input");
        return;
    }
    int current_interval = 1000;
    int ret = 0;
    while(Is_running){
        usleep(current_interval*1000);
        ret = move_snake_s();
        if(ret == TG_OK){
            log_info("auto: head x %d, y %d", Head->x, Head->y);
        }else if (ret == TG_DONE){
            log_info("auto: Game over");
            Is_running = false;
        }else{
            ;
        }
    }
    // 结束监控线程
    pthread_cancel(Input_task_id);
    pthread_join(Input_task_id, NULL);
}

void game_final(void)
{
    clean_game();
}

void new_food()
{
    int x = 0, y = 0;
    bool is_valid = false;
    while(is_valid == false){
        x = rand() % board.window_width;
        y = rand() % board.window_height;
        
        snake_t* tmp = Head;
        while(1){
            if(tmp->x == x && tmp->y == y){
                is_valid = false;
                break;
            }
            if(tmp->node.next == NULL){
                is_valid = true;
                break;
            } 
            tmp = container_of(tmp->node.next, snake_t, node);
        }
    }
    Food_x = x;
    Food_y = y;
    log_info("New food x %d, y %d", x, y);
    draw_food(Food_x, Food_y);
}

int move_snake(void);
int move_snake_s()
{
    pthread_mutex_lock(&mutex);
    int ret = move_snake();
    pthread_mutex_unlock(&mutex);
    return ret;
}
int move_snake(void)
{
    
    int x = Head->x;
    int y = Head->y;
    switch(board.direction){
        case MOVE_UP: 
            y--;
            break;
        case MOVE_DOWN:
            y++;
            break;
        case MOVE_LEFT:
            x--;
            break;
        case MOVE_RIGHT:
            x++;
            break;
        default:
            log_error("Invalid direction");
            return TG_ERROR;
    }
    // 撞墙检测
    if(x < 0 || x >= board.window_width || y < 0 || y >= board.window_height){
        log_error("Collide with wall");
        clean_game();
        return TG_DONE;
    }
    // 撞自己检测
    snake_t* tmp = Head;
    while(1){
        if(tmp->x == x && tmp->y == y){
            log_error("Collide with self");
            clean_game();
            return TG_DONE;
        }
        if(tmp->node.next == NULL){
            break;
        } 
        tmp = container_of(tmp->node.next, snake_t, node);
    }
    if(x == Food_x && y == Food_y){
        log_info("Eat food, current legth of snake is %d", ++board.score);
        snake_t* snake = tg_malloc(sizeof(snake_t));
        snake->x = x;
        snake->y = y;
        snake->node.prev = NULL;
        snake->node.next = &Head->node;
        Head->node.prev = &snake->node;
        Head = snake;
        draw_snake(Head->x, Head->y);
        new_food();
    }else{
        clear_snake(Tail->x, Tail->y);
        // 将尾巴节点作为新头部，其他不变
        if(Head == Tail){
            Tail->x = x;
            Tail->y = y;
        }else{
            node_t* tail_prev = Tail->node.prev;
            tail_prev->next = NULL;
            Tail->x = x;
            Tail->y = y;
            Tail->node.next = &Head->node;
            Tail->node.prev = NULL;
            Head->node.prev = &Tail->node;;
            Head = Tail;
            Tail = container_of(tail_prev, snake_t, node);
        }
        draw_snake(Head->x, Head->y);
    }

    return TG_OK;
}

void* handle_input(void* data)
{
    int key = 0;
    bool valid_move = true;
    while(Is_running){
        key = input();
        log_debug("key %d", key);
        valid_move = true;
        switch(key){
            case KEY_UP:
            case 'w':
                if(board.direction != MOVE_DOWN) board.direction = MOVE_UP;
                else valid_move = false;
                break;
            case KEY_DOWN:
            case 's':
                if(board.direction != MOVE_UP) board.direction = MOVE_DOWN;
                else valid_move = false;
                break;
            case KEY_LEFT:
            case 'a':
                if(board.direction != MOVE_RIGHT) board.direction = MOVE_LEFT;
                else valid_move = false;
                break;
            case KEY_RIGHT:
            case 'd':
                if(board.direction != MOVE_LEFT) board.direction = MOVE_RIGHT;
                else valid_move = false;
                break;
            default:
                log_warn("Unknow key: %d", key);
                valid_move = false;
                break;
        }

        if(valid_move){
            int ret = move_snake_s();
            if(ret == TG_OK){
                log_info("ctrl: head x %d, y %d", Head->x, Head->y);
            }else if (ret == TG_DONE){
                log_info("ctrl: Game over");
                Is_running = false;
            }else{
                ;
            }
        }
    }

    return NULL;
}
