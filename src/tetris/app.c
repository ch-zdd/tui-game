

#include <memory.h>

#include "app.h"
#include "app-context.h"
#include "app-tui.h"
#include "../../lib/common.h"
#include "../../lib/tg_queue.h"
#include "../../lib/tg_time.h"

typedef struct{
    bool task_is_running;
    pthread_t task_id;
    const char* task_name;
    void* (*task_func)(void*);
    void* task_data;
}task_t;

enum{
    TASK_INPUT_LISTENER = 0,
    MAX_TASK_NUM
};

enum{
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP
};

void* handle_input(void* data);
static task_t task_list[MAX_TASK_NUM] = {
    {
        .task_is_running = false,
        .task_id = 0,
        .task_name = "input_listener",
        .task_func = handle_input,
        .task_data = NULL
    }
};

static struct{
    bool game_is_running;
    tui_tetromino_t tetromino;
}game_context;

pthread_mutex_t move_lock = PTHREAD_MUTEX_INITIALIZER;



task_t* get_task(int task_index)
{
    return &(task_list[task_index]);
}

int game_ctx_init(void);
void game_ctx_final(void);

int show_tetrominoe_for_debug(tetromino_t* tetromino, int num);
void game_loop(void);

int generate_tetrominoe(void);
bool collision(int direction);
int move_tetrominoe(int direction);
int settlement(void);

int start_task(int task_index);

int app_run(void)
{
    tetris_context_t* ctx = get_app_context();
    show_tetrominoe_for_debug(ctx->tetromino, ctx->tetrominoes_num);

    if(TG_OK != game_window_draw()){
        return TG_ERROR;
    }

    if(TG_OK != game_ctx_init()){
        return TG_ERROR;
    }

    game_loop();

    return TG_OK;
}

int app_stop()
{
    game_context.game_is_running = false;
    game_ctx_final();

    return TG_OK;
}

int game_ctx_init(void)
{
    // 初始化游戏板
    tui_game_board_t *game_board = get_game_board();
    int cell_width = get_app_context()->symbol_uniform_width;
    tetris_window_para_t* game = &(get_windows_para()->game);

    game_board->cell_width = cell_width;
    game_board->height = game->scr_line;
    game_board->width = game->scr_col/cell_width;
    game_board->flags = (uint8_t*)tg_malloc(game_board->width*game_board->height*sizeof(uint8_t));
    if(game_board->flags == NULL){
        log_error("Failed to allocate memory for game board, %s", strerror(errno));
        return TG_ERROR;
    }
    memset(game_board->flags, 0, game_board->width*game_board->height*sizeof(uint8_t));
    int i = 0;
    // 窗口边框与游戏版边重合
    for(i = 0; i<game_board->height; i++){
        game_board->flags[i*game_board->width] = 1;
        game_board->flags[i*game_board->width+game_board->width-1] = 1;
    }
    for(i = 0; i<game_board->width; i++){
        game_board->flags[i] = 1;
        game_board->flags[(game_board->height-1)*game_board->width+i] = 1;
    }
    draw_border();

    return TG_OK;
}

void game_ctx_final(void)
{
    int i = 0;
    // 停止所有任务
    for(i = 0; i<MAX_TASK_NUM; i++){
        if(task_list[i].task_is_running){
            pthread_cancel(task_list[i].task_id);
            task_list[i].task_is_running = false;
            pthread_join(task_list[i].task_id, NULL);
        }
        if(task_list[i].task_data != NULL){
            tg_free(task_list[i].task_data);
        }
    }
    // 清理游戏资源
    tui_game_board_t *game_board = get_game_board();
    if(game_board->flags != NULL){
        tg_free(game_board->flags);
    }
}

int show_tetrominoe_for_debug(tetromino_t* tetromino, int num)
{
    log_debug("Load tetromino num: %d", num);
    for(int i = 0; i< num; i++){
        log_text("Tetromino name[%s] map[%d] type[%s] symbol[%s] bk[%s] color[%s]", 
            tetromino[i].name, tetromino[i].map, tetromino_type_to_string(tetromino[i].type), tetromino[i].symbol, tetromino[i].background, color_to_string(tetromino[i].color_index));
        log_text("%s", tetromino_to_string(tetromino[i]));
    }

    return TG_OK;
}

void game_loop(void)
{
    //方块下落初始间隔1000毫秒
    int current_interval = 1000;

    game_context.game_is_running = true;

    start_task(TASK_INPUT_LISTENER);
    generate_tetrominoe();
    draw_tetromino(game_context.tetromino, false);
    
    while(game_context.game_is_running){
        usleep(current_interval*1000);
        //与输入监控的任务互斥
        pthread_mutex_lock(&move_lock);
        log_debug("loop down");
        move_tetrominoe(MOVE_DOWN);
        pthread_mutex_unlock(&move_lock);
    }

}

int generate_tetrominoe(void)
{
    tui_tetromino_t tetromino;
    tetris_context_t* ctx = get_app_context();
    int index = rand() % ctx->tetrominoes_num;
    int tetromino_width = ctx->tetromino[index].map_width;
    int tetromino_height = ctx->tetromino[index].map_height;

    tetromino.shape = ctx->tetromino[index];
    tetromino.x = get_game_board()->width/2 - tetromino_width/2;
    //tetromino.y = 0-tetromino_height;
    tetromino.y = 2;
    tetromino.rotate = 0;
    log_debug("Generate tetrominoe %s", ctx->tetromino[index].name);

    game_context.tetromino = tetromino;

    return TG_OK;
}

//在移动方块前调用
int move_tetrominoe(int direction)
{
    tui_tetromino_t* tui_tetromino = &game_context.tetromino;

    // 碰撞检测
    if(collision(direction)){
        log_debug("Collision");
        if(direction == MOVE_DOWN){
            settlement();
            generate_tetrominoe();
        }
        return TG_OK;
    }

    // 清理旧方块
    draw_tetromino(*tui_tetromino, true);

    // 移动方块
    if(direction == MOVE_DOWN){
        tui_tetromino->y++;
    }else if(direction == MOVE_LEFT){
        tui_tetromino->x--;
    }else if(direction == MOVE_RIGHT){
        tui_tetromino->x++;
    }else if (direction == MOVE_UP){
        tui_tetromino->rotate++;  
    }else{
        log_warn("Unknown direction");
        return TG_ERROR;
    }

    //绘制新方块
    draw_tetromino(*tui_tetromino, false);

    return TG_OK;
}

bool collision(int direction)
{
    tui_tetromino_t temp_tetromino = game_context.tetromino;

    // 判断移动后的位置是否超出游戏区域
    if(direction == MOVE_DOWN){
        temp_tetromino.y++;
    }else if(direction == MOVE_LEFT){
        temp_tetromino.x--;
    }else if(direction == MOVE_RIGHT){
        temp_tetromino.x++;
    }else if (direction == MOVE_UP){
        temp_tetromino.rotate++;   
    }else{
        log_error("Unknown direction %d", direction);
        return true;
    }
    
    tui_game_board_t *board = get_game_board();
    tetromino_t shape = game_context.tetromino.shape;
    int i = 0, j = 0;
    for(i = 0; i<temp_tetromino.shape.map_height; i++){
        // 对于位于游戏板上界 及 之上的部分不判断
        if(temp_tetromino.y+i <= 1){
            continue;
        }
        for(j = 0; j<temp_tetromino.shape.map_width; j++){
            int board_flags_index = (temp_tetromino.y+i)*board->width + temp_tetromino.x + j; 
            log_debug("board_flags_index %d, y %d, i %d, j %d, ", board_flags_index, temp_tetromino.y, i, j);
            if((shape.map >> (i*shape.map_width+j)) & 0x01 && board->flags[board_flags_index] == 1){
                return true;
            }
        }
    }

    return false;
}

int settlement(void)
{
    return TG_OK;
}

int start_task(int task_index)
{
    task_t* task = get_task(task_index);
    task->task_is_running = true;
    if(0 != pthread_create(&task->task_id, NULL, task->task_func, task->task_data)){
        log_error("Failed to start task for [%s]", task->task_name);
        return TG_ERROR;
    }

    return TG_OK;
}

void* handle_input(void* data)
{
    int key = 0;
    task_t* task = get_task(TASK_INPUT_LISTENER);
    while(task->task_is_running){
        key = getch();
        log_debug("key %d", key);

        pthread_mutex_lock(&move_lock);
        switch(key){
            case KEY_UP:
            case 'w':
                move_tetrominoe(MOVE_UP);
                log_debug("Tetromino rotate");
                break;
            case KEY_DOWN:
            case 's':
                move_tetrominoe(MOVE_DOWN);
                log_debug("Tetromino down");
                break;
            case KEY_LEFT:
            case 'a':
                move_tetrominoe(MOVE_LEFT);
                log_debug("Tetromino left");
                break;
            case KEY_RIGHT:
            case 'd':
                move_tetrominoe(MOVE_RIGHT);
                log_debug("Tetromino right");
                break;
            case '1':
                log_debug("Pause the game");
                break;
            case '3':
                log_debug("Exit the game");
                break;
            default:
                log_warn("Unknow key: %d", key);
                break;
        }
        pthread_mutex_unlock(&move_lock);
    }

    return NULL;
}
