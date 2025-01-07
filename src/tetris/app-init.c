#include <memory.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "app-init.h"
#include "../../lib/common.h"
#include "../../lib/data_handle.h"
#include "app-context.h"
#include "app-tui.h"

int set_role(void);

int app_init(void)
{
    if(TG_OK != app_tui_init()){
        log_error("tui init failed");
        app_tui_final();
        return TG_ERROR;
    }

    if(get_app_context()->cfg_path == NULL){
        log_info("No config file path to set");
        return TG_ERROR;
    }

    if(TG_OK != parse_app_cfg(get_app_context()->cfg_path)){
        return TG_ERROR;
    }

    log_text("\tTETRIS!\n");
    log_info("cfg path:%s", get_app_context()->cfg_path);

    if(TG_OK != parse_game_cfg()){
        return TG_ERROR;
    }
    
    return TG_OK;
}

int app_final(void)
{
    app_tui_final();

    return TG_OK;
}

