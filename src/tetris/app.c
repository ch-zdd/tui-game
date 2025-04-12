

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
    game_board_t game_board;
    tetromino_t current_tetromino;
}game_context;

pthread_mutex_t move_lock = PTHREAD_MUTEX_INITIALIZER;

task_t* get_task(int task_index)
{
    return &(task_list[task_index]);
}


game_board_t* get_game_board(void)
{
    return &game_context.game_board;
}


int game_ctx_init(void);
void game_ctx_final(void);

void game_loop(void);

int generate_tetrominoe(void);
bool collision(int direction);
int move_tetrominoe(int direction);
int settlement(void);

int start_task(int task_index);

int app_run(void)
{
    tui_context_t* ctx = get_tui_context();
    show_shape_for_debug(ctx->shape, ctx->shape_num);

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
    game_board_t *game_board = get_game_board();
    memset(game_board, 0, sizeof(game_board_t));

    game_board->height = get_tui_context()->game_window_height;
    game_board->width = get_tui_context()->game_window_width / get_tui_context()->cell_width;
    game_board->cell = (board_cell_t*)tg_malloc(game_board->width*game_board->height*sizeof(board_cell_t));
    if(game_board->cell == NULL){
        log_error("Failed to allocate memory for game board, %s", strerror(errno));
        return TG_ERROR;
    }
    memset(game_board->cell, 0, game_board->width*game_board->height*sizeof(board_cell_t));
    int i = 0;
    // 窗口边框与游戏版边重合, 板边默认符号无需专门设置
    for(i = 0; i<game_board->height; i++){
        game_board->cell[i*game_board->width].presence = 1;
        game_board->cell[i*game_board->width+game_board->width-1].presence = 1;
    }
    for(i = 0; i<game_board->width; i++){
        game_board->cell[i].presence = 1;
        game_board->cell[(game_board->height-1)*game_board->width+i].presence = 1;
    }
    draw_border(game_board);

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
    game_board_t *game_board = get_game_board();
    if(game_board->cell != NULL){
        tg_free(game_board->cell);
    }
}

void game_loop(void)
{
    //方块下落初始间隔1000毫秒
    int current_interval = 1000;

    game_context.game_is_running = true;

    start_task(TASK_INPUT_LISTENER);
    generate_tetrominoe();
    draw_tetromino(game_context.current_tetromino, false);
    
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
    tetromino_t tetromino;
    tui_context_t* ctx = get_tui_context();
    int index = rand() % ctx->shape_num;
    int tetromino_width = ctx->shape[index].map_width;

    tetromino.shape_index = index;

    tetromino.x = get_game_board()->width/2 - tetromino_width/2;
    // 简化游戏逻辑，出生地限制在游戏板内
    tetromino.y = 1;
    tetromino.rotate = 0;
    log_debug("Generate tetrominoe %s", ctx->shape[index].name);

    game_context.current_tetromino = tetromino;

    return TG_OK;
}

//在移动方块前调用
int move_tetrominoe(int direction)
{
    tetromino_t* tetromino = &game_context.current_tetromino;

    // 碰撞检测
    if(collision(direction)){
        log_debug("Collision");
        if(direction == MOVE_DOWN){
            settlement();
            generate_tetrominoe();
            // 绘制新方块
            draw_tetromino(game_context.current_tetromino, false);
        }
        return TG_OK;
    }

    // 清理旧方块
    draw_tetromino(*tetromino, true);

    // 移动方块
    if(direction == MOVE_DOWN){
        tetromino->y++;
    }else if(direction == MOVE_LEFT){
        tetromino->x--;
    }else if(direction == MOVE_RIGHT){
        tetromino->x++;
    }else if (direction == MOVE_UP){
        tetromino->rotate++;  
    }else{
        log_warn("Unknown direction");
        return TG_ERROR;
    }

    //绘制新方块
    draw_tetromino(*tetromino, false);

    return TG_OK;
}

bool collision(int direction)
{
    tetromino_t temp_tetromino = game_context.current_tetromino;

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
    
    game_board_t *board = get_game_board();
    // todo: shape的值需要考虑旋转变量rotate
    shape_t shape = get_tui_context()->shape[temp_tetromino.shape_index];
    int i = 0, j = 0;
    for(i = 0; i<shape.map_height; i++){
        for(j = 0; j<shape.map_width; j++){
            int board_flags_index = (temp_tetromino.y+i)*board->width + temp_tetromino.x + j; 
            if((shape.map >> (i*shape.map_width+j)) & 0x01 && board->cell[board_flags_index].presence == 1){
                return true;
            }
        }
    }

    return false;
}

int settlement(void)
{
    game_board_t *board = get_game_board();
    tetromino_t tetromino = game_context.current_tetromino;
    // todo: shape的值需要考虑旋转变量rotate
    shape_t shape = get_tui_context()->shape[tetromino.shape_index];
    int i = 0, j = 0;

    // 先合并方块到游戏板
    for(i = 0; i<shape.map_height; i++){
        for(j = 0; j<shape.map_width; j++){
            if((shape.map >> (i*shape.map_width+j)) & 0x01){
                // 不考虑异常情况下导致的方块和游戏板重合
                int board_flags_index = (tetromino.y+i)*board->width + tetromino.x + j; 
                board->cell[board_flags_index].presence = 1;
                board->cell[board_flags_index].Foreground_index = shape.symbol_index;
            }
        }
    }

    // 判断本行是否填满
    for(i = 1; i<board->height-1; i++){
        for(j = 1; j<board->width-1; j++){
            if(board->cell[i*board->width+j].presence == 0){
                break;
            }
        }

        // 提前跳出，本行有空的列
        if(j != board->width-1){
            continue;
        }
    
        log_debug("Full line %d", i);
        // 下移上边所有行的数据
        // 边界肯定为1，是否复制无影响
        memmove(&board->cell[2*board->width], &board->cell[1*board->width], board->width*(i - 1)*sizeof(board_cell_t));
        
        // todo: 计分
    }
    // 刷新界面
    draw_board(board);

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
