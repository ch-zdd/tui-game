

#include <memory.h>

#include "app.h"
#include "app-context.h"
#include "app-tui.h"
#include "../../lib/common.h"
#include "../../lib/tg_queue.h"
#include "../../lib/tg_time.h"

static struct{
    struct {
        struct{
            pthread_t task_id;
            bool is_running;
        }detect_input, tetromino_down_timer;
    }app_task;

    struct{
        tg_timer_t* tetromino_down_timer;
    }app_timer;

    struct{
        queue_t* q_input_key;
    }app_queue;

    tui_game_screen_t* game_screen;
    bool tetromino_down_flag;
}Game_context;

void game_ctx_init(void);

int show_tetrominoe_for_debug(tetromino_t* tetromino, int num);
void game_loop(void);
tui_tetromino_t generate_tetrominoe(void);

int handle_input(tui_tetromino_t* tetromino);
void clear_input();
int handle_tetromino_down(tui_tetromino_t* tetromino);

int reach_bottom(tui_tetromino_t tetromino);
int settlement(tui_tetromino_t tetromino);

int start_detect_input_task(void);
int stop_detect_input_task(void);
void *detect_input_task(void* arg);

int start_tetromino_down_timer_task(void);
int stop_tetromino_down_timer_task(void);
void *tetromino_down_timer_task(void* arg);


int app_run(void)
{
    tetris_context_t* ctx = get_app_context();
    show_tetrominoe_for_debug(ctx->tetromino, ctx->tetrominoes_num);

    if(TG_OK != game_window_draw()){
        return TG_ERROR;
    }

    if(TG_OK != game_screen_create(10, 10)){
        return TG_ERROR;
    }

    game_ctx_init();

    game_loop();

    game_screen_destroy();
    return TG_OK;
}

int app_stop()
{
    stop_detect_input_task();
    stop_tetromino_down_timer_task();
    return TG_OK;
}

void game_ctx_init(void)
{
    memset(&Game_context, 0, sizeof(Game_context));
    Game_context.game_screen = get_game_screen();
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
    tui_tetromino_t next_tetromino_index;
    tui_tetromino_t current_tui_tetromino;

    next_tetromino_index = generate_tetrominoe();
    current_tui_tetromino = generate_tetrominoe();

    if(TG_OK != start_tetromino_down_timer_task()){
        log_error("Start task of detecting input failed");
        return;
    }

    if(TG_OK != start_detect_input_task()){
        log_error("Start task of detecting input failed");
        return;
    }   

    tg_timer_start(Game_context.app_timer.tetromino_down_timer);
    
    while(1){
        tetromino_draw(current_tui_tetromino);
        clear_input();

        while(1){
            //返回TG_DONE代表方块已经到达底部，无法再操作
            if(TG_DONE == handle_input(&current_tui_tetromino)){
                break;
            }

            if(TG_DONE == handle_tetromino_down(&current_tui_tetromino)){
                break;
            }
        }

        if(reach_bottom(current_tui_tetromino) == TG_OK){
            settlement(current_tui_tetromino);
            current_tui_tetromino = next_tetromino_index;
            next_tetromino_index = generate_tetrominoe();
        }
    }

}

tui_tetromino_t generate_tetrominoe(void)
{
    tui_tetromino_t tetromino;
    tetris_context_t* ctx = get_app_context();
    int index = rand() % ctx->tetrominoes_num;
    tetromino.tetromino_index = index;
    tetromino.x = Game_context.game_screen->width/2;
    tetromino.y = 0;
    tetromino.rotate = 0;
    log_debug("Generate tetrominoe index: %d", index);

    return tetromino;
}

int handle_input(tui_tetromino_t* tetromino)
{
    int key = 0;
    if(queue_is_empty(Game_context.app_queue.q_input_key)){
        return TG_OK;
    }

    queue_pop(Game_context.app_queue.q_input_key, &key);

    switch(key){
        case KEY_UP:
        case 'w':
            tetromino->rotate = (tetromino->rotate + 1) % 4;
            log_debug("Tetromino rotate");
            break;
        case KEY_DOWN:
        case 's':
            if(tetromino->y >= Game_context.game_screen->height){
                return TG_DONE;
            }else{
                tetromino->y++;
                log_debug("Tetromino down");
            }
            break;
        case KEY_LEFT:
        case 'a':
            if(tetromino->x > 0){
                tetromino->x--;
                log_debug("Tetromino left");
            }
            break;
        case KEY_RIGHT:
        case 'd':
            if(tetromino->x < Game_context.game_screen->width){
                tetromino->x++;
                log_debug("Tetromino right");
            }
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

    return TG_OK;
}

void clear_input()
{
    queue_clear(Game_context.app_queue.q_input_key);
}

int handle_tetromino_down(tui_tetromino_t* tetromino)
{
    if(Game_context.tetromino_down_flag){
        Game_context.tetromino_down_flag = false;
    }else{
        return TG_OK;
    }

    if(tetromino->y >= Game_context.game_screen->height){
        log_debug("Tetromino down reach bottom");
        return TG_DONE;
    }else{
        log_debug("Tetromino auto down");
        tetromino->y++;
    }

    return TG_OK;
}

int reach_bottom(tui_tetromino_t tetromino)
{
    return TG_OK;
}

int settlement(tui_tetromino_t tetromino)
{
    return TG_OK;
}

int start_detect_input_task(void)
{
    queue_t* q = queue_create(sizeof(int), 10);
    if(q == NULL){
        log_error("Failed to create queue");
        return TG_ERROR;
    }
    Game_context.app_queue.q_input_key = q;

    Game_context.app_task.detect_input.is_running = true;
    if(0 != pthread_create(&Game_context.app_task.detect_input.task_id, NULL, detect_input_task, NULL)){
        log_error("Failed to create thread for detecting input");
        return TG_ERROR;
    }

    return TG_OK;
}

int stop_detect_input_task(void)
{
    Game_context.app_task.detect_input.is_running = false;
    log_debug("Waiting for detect input task to stop...");
    pthread_cancel(Game_context.app_task.detect_input.task_id);
    pthread_join(Game_context.app_task.detect_input.task_id, NULL);
    queue_destroy(Game_context.app_queue.q_input_key);
    log_debug("Detect input task stopped successfully");

    return TG_OK;
}

void *detect_input_task(void* arg)
{
    int key = 0;

    while(Game_context.app_task.detect_input.is_running){
        key = getch();
        log_debug("Get key: %#x", key);
        switch(key){
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            case 'w':
            case 's':
            case 'a':
            case 'd':
            case '1':
            case '2':
            case '3':
                if(queue_is_full(Game_context.app_queue.q_input_key)){
                    log_warn("Input queue full");
                    break;
                }
                queue_push(Game_context.app_queue.q_input_key, &key);
                break;
            default:
                break;
        }
    }

    return NULL;
}

int start_tetromino_down_timer_task(void)
{
    tg_utime_t timeout = {0, 1000*1000};
    tg_timer_t* timer = tg_timer_create(timeout);
    if(timer == NULL){
        log_error("Failed to create tetromino down timer");
        return TG_ERROR;
    }
    Game_context.app_timer.tetromino_down_timer = timer;

    Game_context.app_task.tetromino_down_timer.is_running = true;
    if(0 != pthread_create(&Game_context.app_task.detect_input.task_id, NULL, tetromino_down_timer_task, NULL)){
        log_error("Failed to create thread for tetromino down timer");
        return TG_ERROR;
    }

    return TG_OK;
}

int stop_tetromino_down_timer_task(void)
{
    Game_context.app_task.detect_input.is_running = false;
    log_debug("Waiting for tetromino down timer task to stop...");
    pthread_cancel(Game_context.app_task.detect_input.task_id);
    pthread_join(Game_context.app_task.detect_input.task_id, NULL);
    tg_timer_destroy(Game_context.app_timer.tetromino_down_timer);
    log_debug("Tetromino down timer task stopped successfully");

    return TG_OK;
}

void *tetromino_down_timer_task(void* arg)
{
    while(Game_context.app_task.tetromino_down_timer.is_running){
        if(tg_timer_is_expired(Game_context.app_timer.tetromino_down_timer)){
            Game_context.tetromino_down_flag = true;
            log_debug("Tetromino down timer expired");
            tg_timer_reset(Game_context.app_timer.tetromino_down_timer);
        }
        usleep(1000);
    }

    return NULL;
}
