#include <memory.h>
#include <string.h>

#include "app-init.h"
#include "../lib/common.h"
#include "../lib/data_handle.h"
#include "app-context.h"

int app_init(void)
{
    if(TK_OK != load_role()){
        TK_ABORT();
    }

    set_role();
    
    return TK_OK;
}

int app_final(void)
{
    return TK_OK;
}

int set_role(void)
{
    role_t* role = NULL;
    app_context_t* ctx = get_app_context();
    int i = 0;
    for(i = 0; i < ctx->role_num; i++){
        role = &(ctx->role[i]);

        role->level = ctx->role_common_level;
        role->hp_max = ctx->role_common_hp;
        role->mp_max = ctx->role_common_mp;
        role->hp = role->hp_max;
        role->mp = role->mp_max;
        role->status = alive;
        role->allocate_attribute_points = 0;

        role->curent_body_attribute.wu = role->init_body_attribute.wu + role->body_attribute_growth.wu*role->level;
        role->curent_body_attribute.tong = role->init_body_attribute.tong + role->body_attribute_growth.tong*role->level;
        role->curent_body_attribute.zhi = role->init_body_attribute.zhi + role->body_attribute_growth.zhi*role->level;
        role->curent_body_attribute.min = role->init_body_attribute.min + role->body_attribute_growth.min*role->level;
        role->curent_body_attribute.shi = role->init_body_attribute.shi + role->body_attribute_growth.shi*role->level;
        role->curent_body_attribute.speed = role->init_body_attribute.speed + role->body_attribute_growth.speed*role->level;

        role->self_tactics.active = false;
        role->learned_tactics[0].active = false;
        role->learned_tactics[1].active = false;
    }

    log_info("role info loaded:");
    for(i = 0; i < ctx->role_num; i++){
        show_role(&(ctx->role[i]));
    }

    return TK_OK;
}
