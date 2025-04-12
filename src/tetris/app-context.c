#include <string.h>

#include "app-context.h"
#include "../../lib/data_handle.h"
#include "../../lib/tui.h"
#include "app-tui.h"

#define MAX_KEY_VALUE_LEN 1024

static app_context_t context;

int parse_shape(shape_t* shape, const char* shape_str);
int parse_shape_map(const char* map_str, int* map, int* map_width, int* map_height);
int parse_shape_common_attr(const char* common_str);

void init_app_context(void)
{
    memset(&context, 0, sizeof(app_context_t));
}

app_context_t* get_app_context(void)
{
    return &context;
}

int set_cfg_path(const char* path)
{
    if(path == NULL || strlen(path) == 0){
        tg_print("No config file path to set");
        return TG_ERROR;
    }

    strncpy(context.cfg_path, path, MAX_PATH_LEN);

    return TG_OK;
}

int parse_app_cfg(const char* cfg_path)
{
    int game_witdh = 0;
    int game_height = 0;
    char buffer[1024] = {};
    char* cfg_str = file_to_string(cfg_path);
    if(cfg_str == NULL) {
        log_error("Failed to read config file");
        return TG_ERROR;
    }

    comment_remove(cfg_str);

    if(parse_key_value(cfg_str, "log_file_path", buffer, 1024) == TG_OK) {
        set_log_file(buffer);
    }

    if(parse_key_value(cfg_str, "log_level", buffer, 1024) == TG_OK) {
        log_levle_t level = log_string_to_level(buffer);
        if(level > LOG_LEVEL_ALL){
            log_error("Illegal log level, use default INFO level");
            set_log_level(LOG_LEVEL_INFO);
        }else{
            set_log_level(level);
        }
    }

    if(parse_key_value(cfg_str, "game_cfg_path", buffer, 1024) == TG_OK) {
        set_game_cfg_path(buffer);
    }

    if(parse_key_value(cfg_str, "game_window_width", buffer, 1024) == TG_OK) {
        if(atoi(buffer)<=0){
            log_error("Invalid battle window width");
            return TG_ERROR;
        }
        game_witdh = atoi(buffer)/2*2;
    }else{
        game_witdh = DEFAULT_WINDOW_WIDTH;
    }
    if(parse_key_value(cfg_str, "game_window_height", buffer, 1024) == TG_OK){
        if(atoi(buffer)<=0){
            log_error("Invalid battle window height");
            return TG_ERROR;
        }
        game_height = atoi(buffer);
    }else{
        game_height = DEFAULT_WINDOW_HEIGHT;
    }

    log_info("set game window: %d * %d", game_witdh, game_height);
    set_game_win_size(game_height, game_witdh);

    tg_free(cfg_str);
    return TG_OK;
}

int set_game_cfg_path(const char* path)
{
    char* self_path = context.game_cfg_path;
    if(path == NULL || strlen(path) == 0){
        log_error("role path is NULL");
        return TG_ERROR;
    }

    memcpy(self_path, path, strlen(path));
    return TG_OK;
}

int parse_game_cfg(void)
{
    tui_context_t* tui_ctx = get_tui_context();
    app_context_t* ctx = get_app_context();
    const char* p = NULL;
    int count = 0;

    if(ctx->game_cfg_path == NULL || strlen(ctx->game_cfg_path) == 0){
        log_error("No game config file path to set");
        return TG_ERROR;
    }

    log_info("parse game setting, path: %s", ctx->game_cfg_path);
    char* game_cfg_str = file_to_string(ctx->game_cfg_path);
    if(game_cfg_str == NULL) {
        log_error("Failed to read game config file");
        goto read_error;
    }
    comment_remove(game_cfg_str);

    char buffer[MAX_ELEMENTS_SIZE];
    p = game_cfg_str;
    //从配置文件读取不同的方块属性
    while(1){
        if(count >= MAX_TETROMINOES_NUM){
            log_warn("Too many shape, only load %d", MAX_TETROMINOES_NUM);
            break;
        }

        memset(buffer, 0, MAX_ELEMENTS_SIZE);
        p = parse_cfg_label(p, "@shape", buffer);
        if(p == NULL){
            log_debug("No more shape to load, count = %d", count);
            break;
        }
 
        if(parse_shape(&(tui_ctx->shape[count]), buffer) != TG_OK){
            log_warn("Parse shape failed");
            log_text("shape=>\n%s\n", buffer);
            continue;
        }
        count++;
    }
    tui_ctx->shape_num = count;
    if(count == 0){
        log_error("No shape loaded");
        goto read_error;
    }

    //加载方块的共同设定
    log_info("load shape common setting");
    memset(buffer, 0, MAX_ELEMENTS_SIZE);
    if(NULL == parse_cfg_label(game_cfg_str, "@shape_common_attr", buffer)){
        log_error("No role common setting label found");
        goto read_error;
    }

    if(TG_OK != parse_shape_common_attr(buffer)){
        log_error("load role common setting context failed");
        goto read_error;
    }


    tg_free(game_cfg_str);
    return TG_OK;

read_error:
    tg_free(game_cfg_str);
    return TG_ERROR;
}

int parse_shape(shape_t* shape, const char* shape_str)
{
    char buffer[MAX_ELEMENTS_SIZE] = "";
    const char* p = shape_str;

    if(parse_key_value(p, "name", buffer, MAX_ELEMENTS_SIZE) != TG_OK){
        log_error("No name found");
        strncpy(shape->name, "NONE", MAX_TETROMINOES_NAME_LEN);
    }else{
        strncpy(shape->name, buffer, MAX_TETROMINOES_NAME_LEN);
    }
    

    if(parse_key_value(p, "type", buffer, MAX_ELEMENTS_SIZE) != TG_OK){
        log_error("No type found");
        shape->type = SHAPE_TYPE_NORMAL;
    }else{
        int type = shape_type_to_int(buffer);
        if(type == SHAPE_TYPE_NONE){
            log_error("Invalid type %s", buffer);
            shape->type = SHAPE_TYPE_NORMAL;
        }else{
            shape->type = type;
        }
    }

    if(parse_key_value(p, "map", buffer, MAX_ELEMENTS_SIZE) != TG_OK){
        log_error("No map found");
        return TG_ERROR;
    }
    if(TG_OK != parse_shape_map(buffer, &(shape->map), &(shape->map_width), &(shape->map_height))){
        log_error("Invalid map %s", buffer);
        return TG_ERROR;
    }

    //以下为可选项
    if(parse_key_value(p, "symbol", buffer, MAX_ELEMENTS_SIZE) != TG_OK){
        log_debug("Use common symbol for shape %s", shape->name);
        shape->symbol_index = -1;
    }else{
        remove_quotation_marks(buffer);
        shape->symbol_index = add_cell_symbol(buffer);
    }

    if(parse_key_value(p, "color", buffer, MAX_ELEMENTS_SIZE) != TG_OK){
        log_debug("Use common color for shape %s", shape->name);
        shape->color_index = -1;
    }else{
        int index = color_to_index(buffer);
        if(index == TG_ERROR){
            log_error("Invalid color %s", buffer);
            shape->color_index = -1;
        }else{
            shape->color_index = index;
        }
    }

    return TG_OK;
}

int shape_type_to_int(const char* type)
{
    if(strcmp(type, "normal") == 0){
        return SHAPE_TYPE_NORMAL;
    }else if(strcmp(type, "penetrate") == 0){
        return SHAPE_TYPE_PENETRATE;
    }else if(strcmp(type, "bomb") == 0){
        return SHAPE_TYPE_BOMB;
    }else{
        return SHAPE_TYPE_NONE;
    }

    return SHAPE_TYPE_NONE;
} 

const char* shape_type_to_string(int type)
{
    switch(type){
        case SHAPE_TYPE_NORMAL:
            return "normal";
        case SHAPE_TYPE_PENETRATE:
            return "penetrate";
        case SHAPE_TYPE_BOMB:
            return "bomb";
        default:
            return "none";
    }
}

int parse_shape_map(const char* map_str, int* map, int* map_width, int* map_height)
{
    int map_para = 0;
    int map_width_para = 0;
    int map_height_para = 0;
    size_t len = 0;
    char* p = NULL;

    char map_buffer[MAX_ELEMENTS_SIZE] = "";

    memcpy(map_buffer, map_str, strlen(map_str));
    trim_chars(map_buffer, " ;");
    
    map_height_para = count_char(map_buffer, '\n')+1;
    if(map_height_para < 2){
        log_error("Invalid map height");
        return TG_ERROR;
    }

    trim_chars(map_buffer, "\n");
    if(is_all_digits(map_buffer) == false){
        log_error("Invalid map");
        return TG_ERROR;
    }

    len = strlen(map_buffer);
    if(len >30){
        log_error("Map is too large");
        return TG_ERROR;
    }

    map_width_para = len/map_height_para;
    if(len != map_width_para*map_height_para){
        log_error("Invalid map width");
        return TG_ERROR;
    }

    p = map_buffer;
    int i = 0;
    while(*p != '\0'){
        if(*p != '0'){
            map_para |= 1<<i;
        }
        i++;
        p++;
        if(i > len){
            break;
        }   
    }

    log_debug("map = %d, map width = %d, map height = %d", map_para, map_width_para, map_height_para);

    *map = map_para;
    *map_width = map_width_para;
    *map_height = map_height_para;

    return TG_OK;
}

int parse_shape_common_attr(const char* common_str)
{
    tui_context_t* ctx = get_tui_context();
    char buffer[MAX_ELEMENTS_SIZE] = "";
    const char* p = common_str;
    // 考虑引号占位长度2
    char symbol[MAX_SYMBOL_LEN] = "";
    char background[MAX_SYMBOL_LEN] = "";
    int cfg_symbol_len = MAX_SYMBOL_LEN+2;
    int color_index = -1;

    if(parse_key_value(p, "symbol", buffer, cfg_symbol_len) != TG_OK){
        log_error("No name found");
        return TG_ERROR;
    }else{
        remove_quotation_marks(buffer);
        if(strlen(buffer) == 0){
            log_error("Invalid symbol");
            return TG_ERROR;
        }
        strncpy(symbol, buffer, strlen(buffer)>MAX_SYMBOL_LEN? MAX_SYMBOL_LEN:strlen(buffer));
    }

    memset(buffer, 0, MAX_ELEMENTS_SIZE);
    if(parse_key_value(p, "background", buffer, cfg_symbol_len) != TG_OK){
        log_error("No name found");
        return TG_ERROR;
    }else{
        remove_quotation_marks(buffer);
        if(strlen(buffer) == 0){
            log_error("Invalid background");
            return TG_ERROR;
        }
        strncpy(background, buffer, strlen(buffer)>MAX_SYMBOL_LEN? MAX_SYMBOL_LEN:strlen(buffer));
    }

    if(strlen(background) != strlen(symbol)){
        log_error("Symbol and background should have the same length, %d != %d", strlen(symbol), strlen(background));
        return TG_ERROR;
    }
    ctx->background_index = add_cell_symbol(background);
    
    memset(buffer, 0, MAX_ELEMENTS_SIZE);
    if(parse_key_value(p, "color", buffer, MAX_COLOR_LEN) != TG_OK){
        log_error("No name found");
        color_index = -1;
    }else{
        remove_quotation_marks(buffer);
        int index = color_to_index(buffer);
        if(index == TG_ERROR){
            log_error("Invalid color %s", buffer);
            color_index = -1;
        }else{
            color_index = index;
        }
    }

    int symbol_index = add_cell_symbol(symbol);
    for(int i = 0; i< ctx->shape_num; i++){
        if(ctx->shape[i].symbol_index < 0){
            ctx->shape[i].symbol_index = symbol_index;
        }

        if(ctx->shape[i].color_index < 0){
            ctx->shape[i].color_index = color_index;
        }
    }

    return TG_OK;
}
