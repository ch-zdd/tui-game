

#include <memory.h>

#include "app.h"
#include "app-context.h"
#include "app-tui.h"
#include "../../lib/common.h"
#include "../../lib/tg_queue.h"
#include "../../lib/tg_time.h"

// 简化游戏逻辑，出生地限制在游戏板内
#define TETROMINO_BORN_Y 1

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

typedef struct{
    int symbol_index;
    int color_index;
    bool presence;
}board_cell_t;

typedef struct{
    board_cell_t* cell;
    int bkg_symbol_index;
    int bkg_color_index;
    int width;
    int height;
}game_board_t;

typedef struct{
    int map;
    int width;
    int height;
    int symbol_index;
    int color_index;
    // 左上角为坐标原点，x和y为方块左上角的坐标
    int x;
    int y;
}tetromino_t;

enum{
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_NONE
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
    tetromino_t* tetromino_pool;
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

void end_game();
void restart_game();
void pause_game();

void game_loop(void);

int generate_tetrominoe(tetromino_t* result_tetromino);
bool collision(int direction);
int move_tetrominoe(int direction);
int settlement(void);
int rotate_shape(tetromino_t* shape);

void draw_border(void);
void draw_board(void);
void draw_tetromino(tetromino_t tetromino);
void clear_tetromino(tetromino_t tetromino);
void draw_tetromino_2(tetromino_t tetromino);


int start_task(int task_index);
int stop_task(int task_index);

int app_run(void)
{
    tui_context_t* ctx = get_tui_context();
    show_shape_for_debug(ctx->shape, ctx->shape_num);

    if(TG_OK != game_window_draw()){
        return TG_ERROR;
    }

    // 启动任务
    start_task(TASK_INPUT_LISTENER);

    if(TG_OK != game_ctx_init()){
        return TG_ERROR;
    }

    game_loop();

    return TG_OK;
}

int app_stop()
{
    int i = 0;

    game_context.game_is_running = false;
    for(i = 0; i<MAX_TASK_NUM; i++){
        stop_task(i);
    }

    game_ctx_final();

    return TG_OK;
}

// 负责和绘画模块对接
int game_ctx_init(void)
{
    // 初始化游戏板
    game_board_t *game_board = get_game_board();
    tui_context_t * tui_ctx = get_tui_context();
    memset(game_board, 0, sizeof(game_board_t));

    game_board->height = tui_ctx->game_window_height/ tui_ctx->cell_height;
    game_board->width = tui_ctx->game_window_width / tui_ctx->cell_width;
    log_debug("Game board height: %d, width: %d", game_board->height, game_board->width);
    game_board->cell = (board_cell_t*)tg_malloc(game_board->width*game_board->height*sizeof(board_cell_t));
    if(game_board->cell == NULL){
        log_error("Failed to allocate memory for game board, %s", strerror(errno));
        return TG_ERROR;
    }
    memset(game_board->cell, 0, game_board->width*game_board->height*sizeof(board_cell_t));
    int i = 0;
    // 窗口边框与游戏版边重合
    // 背景统一
    game_board->bkg_symbol_index = tui_ctx->bkg_symbol_index;
    game_board->bkg_color_index = DEFAULT_BKG_COLOR_IDX;
    for(i = 0; i<game_board->height*game_board->width; i++){
        game_board->cell[i].presence = 0;
        game_board->cell[i].symbol_index = DEFAULT_BKG_SYMBOL_IDX;
        game_board->cell[i].color_index = DEFAULT_BKG_COLOR_IDX;
    }
    for(i = 0; i<game_board->height; i++){
        game_board->cell[i*game_board->width].presence = 1;
        game_board->cell[i*game_board->width].symbol_index = DEFAULT_SYMBOL_IDX;
        game_board->cell[i*game_board->width].color_index = DEFAULT_SYMBOL_COLOR_IDX;

        game_board->cell[i*game_board->width+game_board->width-1].presence = 1;
        game_board->cell[i*game_board->width+game_board->width-1].symbol_index = DEFAULT_SYMBOL_IDX;
        game_board->cell[i*game_board->width+game_board->width-1].color_index = DEFAULT_SYMBOL_COLOR_IDX;
    }
    for(i = 0; i<game_board->width; i++){
        game_board->cell[i].presence = 1;
        game_board->cell[i].symbol_index = DEFAULT_SYMBOL_IDX;
        game_board->cell[i].color_index = DEFAULT_SYMBOL_COLOR_IDX;

        game_board->cell[(game_board->height-1)*game_board->width+i].presence = 1;
        game_board->cell[(game_board->height-1)*game_board->width+i].symbol_index = DEFAULT_SYMBOL_IDX;
        game_board->cell[(game_board->height-1)*game_board->width+i].color_index = DEFAULT_SYMBOL_COLOR_IDX;
    }
    draw_border();

    // 获取所有方块到本地池
    int tetromino_num = tui_ctx->shape_num;
    game_context.tetromino_pool = (tetromino_t*)tg_malloc(tetromino_num*sizeof(tetromino_t));
    for(i = 0; i<tetromino_num; i++){
        game_context.tetromino_pool[i].symbol_index = tui_ctx->shape[i].symbol_index;
        game_context.tetromino_pool[i].color_index = tui_ctx->shape[i].color_index;
        game_context.tetromino_pool[i].height = tui_ctx->shape[i].map_height;
        game_context.tetromino_pool[i].width = tui_ctx->shape[i].map_width;
        game_context.tetromino_pool[i].map = tui_ctx->shape[i].map;
        game_context.tetromino_pool[i].x = 0;
        game_context.tetromino_pool[i].y = 0;
    }

    return TG_OK;
}

void game_ctx_final(void)
{
    // 清理游戏资源
    game_board_t *game_board = get_game_board();
    if(game_board->cell != NULL){
        tg_free(game_board->cell);
    }

    // 清理方块池
    if(game_context.tetromino_pool != NULL){
        tg_free(game_context.tetromino_pool);
    }
}

void game_loop(void)
{
    //方块下落初始间隔1000毫秒
    int current_interval = 1000;

    game_context.game_is_running = true;

    if(TG_OK !=  generate_tetrominoe(&game_context.current_tetromino)){
        log_warn("Failed to generate tetrominoe, maybe the screen is too small ");
        return;
    }
    draw_tetromino(game_context.current_tetromino);
    
    while(game_context.game_is_running){
        usleep(current_interval*1000);
        //与输入监控的任务互斥
        pthread_mutex_lock(&move_lock);
        log_debug("loop down");
        if(TG_OK != move_tetrominoe(MOVE_DOWN)){
            // todo: 显示游戏结束
            log_debug("Exit game loop");
            break;
        }
        pthread_mutex_unlock(&move_lock);
    }
}

int generate_tetrominoe(tetromino_t* result_tetromino)
{
    tetromino_t tetromino;
    tui_context_t* ctx = get_tui_context();
    int index = rand() % ctx->shape_num;

    tetromino = game_context.tetromino_pool[index];

    tetromino.x = get_game_board()->width/2 - tetromino.width/2;
    tetromino.y = TETROMINO_BORN_Y;
    log_debug("Generate tetrominoe %s", ctx->shape[index].name);

    if(collision(MOVE_NONE)){
        return TG_ERROR;
    }

    memcpy(result_tetromino, &tetromino, sizeof(tetromino_t));
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
            if(TG_OK != generate_tetrominoe(&game_context.current_tetromino)){
                // 没有空间生成新方块，游戏结束
                // todo: 显示游戏结束
                log_debug("Game over");
                pause_game();
                restart_game();
                
                return TG_OK;
            }
            // 绘制新方块
            draw_tetromino(game_context.current_tetromino);
        }
        return TG_OK;
    }

    // 清理旧方块
    clear_tetromino(*tetromino);

    // 移动方块
    if(direction == MOVE_DOWN){
        tetromino->y++;
    }else if(direction == MOVE_LEFT){
        tetromino->x--;
    }else if(direction == MOVE_RIGHT){
        tetromino->x++;
    }else if (direction == MOVE_UP){
        rotate_shape(tetromino); 
    }else if(direction == MOVE_NONE){
        log_debug("Move none");
    }else{
        log_warn("Unknown direction");
        return TG_ERROR;
    }

    //绘制新方块
    draw_tetromino(*tetromino);

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
        rotate_shape(&temp_tetromino);   
    }else if(direction == MOVE_NONE){
        log_debug("Move none");
    }else{
        log_error("Unknown direction %d", direction);
        return true;
    }
    
    game_board_t *board = get_game_board();
    
    int i = 0, j = 0;
    for(i = 0; i<temp_tetromino.height; i++){
        for(j = 0; j<temp_tetromino.width; j++){
            int board_flags_index = (temp_tetromino.y+i)*board->width + temp_tetromino.x + j; 
            if((temp_tetromino.map >> (i*temp_tetromino.width+j)) & 0x01 && board->cell[board_flags_index].presence == 1){
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
    int i = 0, j = 0;

    // 先合并方块到游戏板
    for(i = 0; i<tetromino.height; i++){
        for(j = 0; j<tetromino.width; j++){
            if((tetromino.map >> (i*tetromino.width+j)) & 0x01){
                // 不考虑异常情况下导致的方块和游戏板重合
                int board_flags_index = (tetromino.y+i)*board->width + tetromino.x + j; 
                board->cell[board_flags_index].presence = 1;
                board->cell[board_flags_index].symbol_index = tetromino.symbol_index;
                board->cell[board_flags_index].color_index = tetromino.color_index;
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
    draw_board();

    return TG_OK;
}

int rotate_shape(tetromino_t* shape)
{
    int i = 0, j = 0 , index = 0;
    int tmp_map = shape->map;
    int size = shape->width;

    shape->map = 0;
    for(i = 0; i< size; i++){
        for(j = 0; j< size; j++){
            index = (size - 1 - j)*size+i;
            if((tmp_map >> index) & 0x01){
                shape->map |= (1<<(i*size+j));
            }
        }
    }

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

int stop_task(int task_index)
{
    task_t* task = get_task(task_index);
    if(task->task_is_running){
        task->task_is_running = false;
        // 由于getch()阻塞,手动取消
        pthread_cancel(task->task_id);
        pthread_join(task->task_id, NULL);
    }

    if(task->task_data != NULL){
        tg_free(task->task_data);
    }

    return TG_OK;
}

void* handle_input(void* data)
{
    int key = 0, ret = TG_OK;
    task_t* task = get_task(TASK_INPUT_LISTENER);
    while(task->task_is_running){
        key = getch();
        log_debug("key %d", key);

        pthread_mutex_lock(&move_lock);
        switch(key){
            case KEY_UP:
            case 'w':
                ret = move_tetrominoe(MOVE_UP);
                log_debug("Tetromino rotate");
                break;
            case KEY_DOWN:
            case 's':
                ret = move_tetrominoe(MOVE_DOWN);
                log_debug("Tetromino down");
                break;
            case KEY_LEFT:
            case 'a':
                ret = move_tetrominoe(MOVE_LEFT);
                log_debug("Tetromino left");
                break;
            case KEY_RIGHT:
            case 'd':
                ret = move_tetrominoe(MOVE_RIGHT);
                log_debug("Tetromino right");
                break;
            case '1':
                log_debug("Pause the game");
                pause_game();
                break;
            case '3':
                log_debug("Exit the game");
                break;
            default:
                log_warn("Unknow key: %d", key);
                break;
        }
        pthread_mutex_unlock(&move_lock);

        if(ret == TG_ERROR){
            log_error("Failed to move tetrominoe, exit the game");
            end_game();
        }
    }

    return NULL;
}

void draw_border(void)
{
    int i;
    board_cell_t cell1, cell2;
    game_board_t* board = get_game_board();

    for(i = 0; i<board->height; i++){
        cell1 = board->cell[i*board->width];
        cell2 = board->cell[i*board->width+board->width-1];
        if(cell1.presence != 1 || cell2.presence != 1){
            log_warn("border of board error");
        }
        draw_cell(i, 0, cell1.symbol_index, cell1.color_index);
        draw_cell(i, board->width-1, cell2.symbol_index, cell2.color_index);
    }
    for(i = 0; i<board->width; i++){
        cell1 = board->cell[i];
        cell2 = board->cell[(board->height-1)*board->width+i];
        if(cell1.presence != 1 || cell2.presence != 1){
            log_warn("border of board error");
        }
        draw_cell(0, i, cell1.symbol_index, cell1.color_index);
        draw_cell(board->height-1, i, cell2.symbol_index, cell2.color_index);
    }
}

void draw_board(void)
{
    int i, j;
    board_cell_t cell;
    game_board_t* board = get_game_board();

    start_draw();
    for(i = 1; i < board->height-1; i++){
        for(j = 1; j < board->width-1; j++){
            cell = board->cell[i*board->width+j];
            if(cell.presence == 1){
                draw_cell(i, j, cell.symbol_index, cell.color_index);
            }else{
                draw_cell(i, j, board->bkg_symbol_index, board->bkg_color_index);
            }
        }
    }
    end_draw();
}

void draw_tetromino(tetromino_t tetromino)
{
    draw_tetromino_2(tetromino);
}

void clear_tetromino(tetromino_t tetromino)
{
    tetromino.symbol_index = get_game_board()->bkg_symbol_index;
    tetromino.color_index = get_game_board()->bkg_color_index;
    draw_tetromino_2(tetromino);
}

void draw_tetromino_2(tetromino_t tetromino)
{
    int i = 0;
    int j = 0;
    int x = tetromino.x;
    int y = tetromino.y;

    start_draw();
    for(i = 0; i < tetromino.height; i++){
        // 边界外不显示
        if(tetromino.y+i <= 0){
            continue;
        }

        for(j = 0; j < tetromino.width; j++){           
            if((tetromino.map >> (i*tetromino.width+j)) & 0x01){
                draw_cell(y+i, x+j, tetromino.symbol_index, tetromino.color_index);
            }else{
                // 游戏板背景统一
                ;
            }
        }
    }
    end_draw();
}

void end_game()
{
    game_context.game_is_running = false;
}

void restart_game()
{
    // 重置游戏上下文
    game_ctx_final();

    // 重新初始化游戏上下文
    game_ctx_init();

    if(TG_OK !=  generate_tetrominoe(&game_context.current_tetromino)){
        log_warn("Failed to generate tetrominoe, maybe the screen is too small ");
        return;
    }
    draw_tetromino(game_context.current_tetromino);
    
}

void pause_game()
{
    //put_text(2, 2, "Pause", COLOR_GREEN);
    getch();
}