#include <string.h>

#include "app-context.h"
#include "../../lib/data_handle.h"
#include "../../lib/tui.h"
#include "app-tui.h"

#define MAX_KEY_VALUE_LEN 1024

static tetris_context_t context;

int parse_tetrominoe(tetromino_t* tetromino, const char* tetrominoes_str);
int parse_tetromino_map(const char* map_str, int* map, int* map_width, int* map_height);
int parse_tetrominoe_common_attr(const char* common_str);

void init_app_context(void)
{
    memset(&context, 0, sizeof(tetris_context_t));
    context.game_window_width = 48;
}

tetris_context_t* get_app_context(void)
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
        context.game_window_width = atoi(buffer)/2*2;
    }else{
        context.game_window_width = DEFAULT_WINDOW_WIDTH;
    }
    if(parse_key_value(cfg_str, "game_window_height", buffer, 1024) == TG_OK){
        if(atoi(buffer)<=0){
            log_error("Invalid battle window height");
            return TG_ERROR;
        }
        context.game_window_height = atoi(buffer) > get_windows_para()->main.scr_line ? get_windows_para()->main.scr_line : atoi(buffer);
    }else{
        context.game_window_height = DEFAULT_WINDOW_HEIGHT > get_windows_para()->main.scr_line ? get_windows_para()->main.scr_line : DEFAULT_WINDOW_HEIGHT;
    }

    log_info("game window: %d * %d", context.game_window_width, context.game_window_height);

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
    tetris_context_t* ctx = &context;
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
            log_warn("Too many tetromino, only load %d", MAX_TETROMINOES_NUM);
            break;
        }

        memset(buffer, 0, MAX_ELEMENTS_SIZE);
        p = parse_cfg_label(p, "@tetromino", buffer);
        if(p == NULL){
            log_debug("No more tetromino to load, count = %d", count);
            break;
        }
 
        if(parse_tetrominoe(&(ctx->tetromino[count]), buffer) != TG_OK){
            log_warn("Parse tetromino failed");
            log_text("tetromino=>\n%s\n", buffer);
            continue;
        }
        count++;
    }
    ctx->tetrominoes_num = count;
    if(count == 0){
        log_error("No tetromino loaded");
        goto read_error;
    }

    //加载方块的共同设定
    log_info("load tetromino common setting");
    memset(buffer, 0, MAX_ELEMENTS_SIZE);
    if(NULL == parse_cfg_label(game_cfg_str, "@tetrominoes_common_attr", buffer)){
        log_error("No role common setting label found");
        goto read_error;
    }

    if(TG_OK != parse_tetrominoe_common_attr(buffer)){
        log_error("load role common setting context failed");
        goto read_error;
    }


    tg_free(game_cfg_str);
    return TG_OK;

read_error:
    tg_free(game_cfg_str);
    return TG_ERROR;
}

int parse_tetrominoe(tetromino_t* tetromino, const char* tetrominoes_str)
{
    char buffer[MAX_ELEMENTS_SIZE] = "";
    const char* p = tetrominoes_str;
    if(parse_key_value(p, "name", buffer, MAX_ELEMENTS_SIZE) != TG_OK){
        log_error("No name found");
        strncpy(tetromino->name, "NONE", MAX_TETROMINOES_NAME_LEN);
    }else{
        strncpy(tetromino->name, buffer, MAX_TETROMINOES_NAME_LEN);
    }
    

    if(parse_key_value(p, "type", buffer, MAX_ELEMENTS_SIZE) != TG_OK){
        log_error("No type found");
        tetromino->type = TETROMINO_TYPE_NORMAL;
    }else{
        int type = tetromino_type_to_int(buffer);
        if(type == TETROMINO_TYPE_NONE){
            log_error("Invalid type %s", buffer);
            tetromino->type = TETROMINO_TYPE_NORMAL;
        }else{
            tetromino->type = type;
        }
    }

    if(parse_key_value(p, "map", buffer, MAX_ELEMENTS_SIZE) != TG_OK){
        log_error("No map found");
        return TG_ERROR;
    }
    if(TG_OK != parse_tetromino_map(buffer, &(tetromino->map), &(tetromino->map_width), &(tetromino->map_height))){
        log_error("Invalid map %s", buffer);
        return TG_ERROR;
    }

    //以下为可选项

    if(parse_key_value(p, "symbol", buffer, MAX_ELEMENTS_SIZE) != TG_OK){
        log_debug("Use common symbol for tetromino %s", tetromino->name);
        memset(tetromino->symbol, 0, MAX_SYMBOL_LEN);
    }else{
        remove_quotation_marks(buffer);
        strncpy(tetromino->symbol, buffer, strlen(buffer)>MAX_SYMBOL_LEN? MAX_SYMBOL_LEN:strlen(buffer));
    }

    if(parse_key_value(p, "background", buffer, MAX_ELEMENTS_SIZE) != TG_OK){
        log_debug("Use common background for tetromino %s", tetromino->name);
        memset(tetromino->background, 0, MAX_SYMBOL_LEN);
    }else{
        remove_quotation_marks(buffer);
        strncpy(tetromino->background, buffer, strlen(buffer)>MAX_SYMBOL_LEN? MAX_SYMBOL_LEN:strlen(buffer));
    }

    if(parse_key_value(p, "color", buffer, MAX_ELEMENTS_SIZE) != TG_OK){
        log_debug("Use common color for tetromino %s", tetromino->name);
        tetromino->color_index = -1;
    }else{
        int index = color_to_index(buffer);
        if(index == TG_ERROR){
            log_error("Invalid color %s", buffer);
            tetromino->color_index = -1;
        }else{
            tetromino->color_index = index;
        }
    }

    return TG_OK;
}

int tetromino_type_to_int(const char* type)
{
    if(strcmp(type, "normal") == 0){
        return TETROMINO_TYPE_NORMAL;
    }else if(strcmp(type, "penetrate") == 0){
        return TETROMINO_TYPE_PENETRATE;
    }else if(strcmp(type, "bomb") == 0){
        return TETROMINO_TYPE_BOMB;
    }else{
        return TETROMINO_TYPE_NONE;
    }

    return TETROMINO_TYPE_NONE;
} 

const char* tetromino_type_to_string(int type)
{
    switch(type){
        case TETROMINO_TYPE_NORMAL:
            return "normal";
        case TETROMINO_TYPE_PENETRATE:
            return "penetrate";
        case TETROMINO_TYPE_BOMB:
            return "bomb";
        default:
            return "none";
    }
}

int parse_tetromino_map(const char* map_str, int* map, int* map_width, int* map_height)
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

char* tetromino_to_string(tetromino_t tetromino)
{
    static char buff[1024];
    
    memset(buff, 0, sizeof(buff));

    for(int i = 0; i < tetromino.map_height; i++){
        for(int j = 0; j < tetromino.map_width; j++){
            if((tetromino.map >> (i*tetromino.map_width+j)) & 0x01){
                snprintf(buff+strlen(buff), sizeof(buff)-strlen(buff), "%s", tetromino.symbol);
            }else{
                snprintf(buff+strlen(buff), sizeof(buff)-strlen(buff), "%s", tetromino.background);
            }
            
        }
        snprintf(buff+strlen(buff), sizeof(buff)-strlen(buff), "\n");
    }

    return buff;
}

int parse_tetrominoe_common_attr(const char* common_str)
{
    tetris_context_t* ctx = &context;
    char buffer[MAX_ELEMENTS_SIZE] = "";
    const char* p = common_str;
    char symbol[MAX_SYMBOL_LEN] = "";
    char background[MAX_SYMBOL_LEN] = "";
    int color_index = -1;

    if(parse_key_value(p, "symbol", buffer, MAX_SYMBOL_LEN) != TG_OK){
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
    if(parse_key_value(p, "background", buffer, MAX_SYMBOL_LEN) != TG_OK){
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

    for(int i = 0; i< ctx->tetrominoes_num; i++){
        if(strlen(ctx->tetromino[i].symbol) == 0){
            strncpy(ctx->tetromino[i].symbol, symbol, strlen(symbol));
        }
        if(strlen(ctx->tetromino[i].background) == 0){
            strncpy(ctx->tetromino[i].background, background, strlen(background));
        }
        if(ctx->tetromino[i].color_index < 0){
            ctx->tetromino[i].color_index = color_index;
        }
    }

    return TG_OK;
}
