#include <memory.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "app-init.h"
#include "../../lib/common.h"
#include "../../lib/data_handle.h"
#include "app-context.h"
#include "app-tui.h"

//int tk_thread_init(void);
//int tk_thread_final(void);
//void task_thread(void* arg);

int set_role(void);

int app_init(void)
{


    if(TG_OK != load_app_cfg(get_app_context()->cfg_path)){
        return TG_ERROR;
    }

    log_text("\tMY THREE KINGDOM!\n");
    log_info("cfg path:%s", get_app_context()->cfg_path);

    if(TG_OK != load_game_cfg()){
        return TG_ERROR;
    }

    srand(time(NULL));
    set_role();

    //tk_thread_init();

    if(TG_OK != app_tui_init()){
        log_error("tui init failed");
        app_tui_final();
        return TG_ERROR;
    }
    
    return TG_OK;
}

int app_final(void)
{
    app_tui_final();
    //tk_thread_final();

    return TG_OK;
}

#if 0
int tk_thread_init(void)
{
    tk_context_t* ctx = get_app_context();

    for(int i = 0; i < MAX_THREAD_NUM; i++){
        if(!pthread_create(&(ctx->tk_thread[i]), NULL, task_thread, NULL)){
            log_error("create tk thread %d failed: %s", i, strerror(errno));
            return TG_ERROR;
        }
    }

    return TG_OK;
}


int tk_thread_final(void)
{
    tk_context_t* ctx = get_app_context();

    for(int i = 0; i < MAX_THREAD_NUM; i++){
        pthread_join(ctx->tk_thread[i], NULL);
    }
    return TG_OK;
}
#endif

int set_role(void)
{
    tk_role_t* role = NULL;
    tk_context_t* ctx = get_app_context();
    int i = 0;
    for(i = 0; i < ctx->role_num; i++){
        role = &(ctx->role[i]);

        role->level = ctx->role_common_level;
        role->hp_max = ctx->role_common_hp;
        role->mp_max = ctx->role_common_mp;
        role->allocate_attribute_points = 0;
    }

    log_info("role info loaded:");
    for(i = 0; i < ctx->role_num; i++){
        show_role(&(ctx->role[i]));
    }

    return TG_OK;
}
