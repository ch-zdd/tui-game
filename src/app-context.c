#include <string.h>

#include "app-context.h"
#include "../lib/data_handle.h"

static app_context_t app_context;

int load_role_attr(role_t* role, const char* role_context);
int load_role_common(const char* role_common_context);

app_context_t* get_app_context(void)
{
    return &app_context;
}

void set_cfg_path(const char* path)
{
    if(path == NULL || strlen(path) == 0){
        tk_print("No config file path to set");
        return;
    }
    strncpy(app_context.cfg_path, path, MAX_PATH_LEN);
}

int load_cfg(const char* cfg_path)
{
    char buffer[1024] = {};
    char* cfg_str = file_to_string(cfg_path);
    if(cfg_str == NULL) {
        log_error("Failed to read config file");
        TK_ABORT();
        return TK_ERROR;
    }

    comment_remove(cfg_str);

    if(parse_key_value(cfg_str, "log_file_path", buffer, 1024) == TK_OK) {
        set_log_file(buffer);
    }

    if(parse_key_value(cfg_str, "log_level", buffer, 1024) == TK_OK) {
        log_levle_t level = log_string_to_level(buffer);
        if(level > LOG_LEVEL_ALL){
            log_error("Illegal log level");
        }else{
            set_log_level(level);
        }
    }

    if(parse_key_value(cfg_str, "role_path", buffer, 1024) == TK_OK) {
        set_role_path(buffer);
    }

    free(cfg_str);
    return TK_OK;
}

int set_role_path(const char* path)
{
    char* self_path = app_context.role_path;
    if(path == NULL || strlen(path) == 0){
        log_error("role path is NULL");
        return TK_ERROR;
    }

    memcpy(self_path, path, strlen(path));
    return TK_OK;
}

int load_role(void)
{
    app_context_t* ctx = &app_context;
    const char* p = NULL;
    int count = 0;
    log_info("load role setting, path: %s", ctx->role_path);
    char* roles_str = file_to_string(ctx->role_path);
    if(roles_str == NULL) {
        log_error("could not open file to read role file");
        goto read_error;
    }
    comment_remove(roles_str);

    //分割不同角色的数据,并加载不同角色的数据
    char role_buffer[MAX_ROLE_DATA_SIZE];
    p = roles_str;
    while(1){
        if(ctx->role_num > MAX_ROLE_NUM){
            log_warn("role num is too much, ignore the subsequent roles");
            break;
        }
        memset(role_buffer, 0, MAX_ROLE_DATA_SIZE);
        p = parse_cfg_label(p, "@role_attr", role_buffer);
        if(p == NULL){
            break;
        }
 
        if(load_role_attr(&(ctx->role[count]), role_buffer) != TK_OK){
            log_warn("load this role failed");
            log_text("role=>\n%s\n", role_buffer);
            continue;
        }
        count++;
    }
    ctx->role_num = count;

    //加载角色的共同设定
    log_info("load role common setting");
    memset(role_buffer, 0, MAX_ROLE_DATA_SIZE);
    if(NULL == parse_cfg_label(roles_str, "@role_common", role_buffer)){
        log_error("No role common setting label found");
        goto read_error;
    }

    if(TK_OK != load_role_common(role_buffer)){
        log_error("load role common setting context failed");
        goto read_error;
    }


    tk_free(roles_str);
    return TK_OK;

read_error:
    tk_free(roles_str);
    TK_ABORT();
    return TK_ERROR;
}

int load_role_attr(role_t* role, const char* role_context)
{
    int ret = TK_OK;
    char buffer[1024] = {};
    char* arr_ptr = NULL;
    char arr_ele[10] = {};
    int* int_ptr = NULL;
    int attr_len = 0;
    int index = 0;

    ret = parse_key_value(role_context, "name", role->name, sizeof(role->name));
    if(ret != TK_OK) return TK_ERROR;
    
    ret = parse_key_value(role_context, "gender", buffer, 10);
    if(ret != TK_OK) return TK_ERROR;
    if(strcmp(buffer, "female") == 0){
        role->gender = Female;
    }else if(strcmp(buffer, "male") == 0){
        role->gender = Male;
    }else{
        log_warn("unknown gender:%s", buffer);
    }

    //属性必须按顺序排列
    ret = parse_key_value(role_context, "attr", buffer, 1024);
    if(ret != TK_OK) return TK_ERROR;

    arr_ptr = buffer;
    int_ptr = &(role->init_body_attribute.wu);
    attr_len = sizeof(role->init_body_attribute)/sizeof(int);
    index = 0;
    while( arr_ptr != NULL ) {
        memset(arr_ele, 0, 10);
        arr_ptr = pasrse_array(arr_ptr, arr_ele, 10, ",");
        if(is_all_digits(arr_ele)){
            int_ptr[index] = atoi(arr_ele);
            index++;
        }
        if(index > attr_len){
            log_error("attr array buffer is not enough");
            return TK_ERROR;
        }
    }
    if(index < attr_len){
        log_error("Incomplete role attribute configuration");
        return TK_ERROR;
    }
    //log_text("load role attr:");
    //print_array_int(&(role->init_body_attribute.wu), attr_len, 4);


    //属性成长也是同样的按顺序排列
    ret = parse_key_value(role_context, "growth", buffer, 1024);
    if(ret != TK_OK) return TK_ERROR;

    arr_ptr = buffer;
    int_ptr = &(role->body_attribute_growth.wu);
    index = 0;
    while( arr_ptr != NULL ) {
        memset(arr_ele, 0, 10);
        arr_ptr = pasrse_array(arr_ptr, arr_ele, 10, ",");
        if(is_all_digits(arr_ele)){
            int_ptr[index] = atoi(arr_ele);
            index++;
        }
        if(index > attr_len){
            log_error("attr array buffer is not enough");
            return TK_ERROR;
        }
    }
    if(index < attr_len){
        log_error("Incomplete role attribute configuration");
        return TK_ERROR;
    }
    //log_text("load role attr growth:");
    //print_array_int(&(role->body_attribute_growth.wu), attr_len, 4);    

    return TK_OK;
}

int load_role_common(const char* role_common_context)
{
    int ret = TK_OK;
    char buffer[10] = {};

    ret = parse_key_value(role_common_context, "level", buffer, sizeof(buffer));
    if(ret != TK_OK) return TK_ERROR;
    if(false == is_all_digits(buffer)){
        log_error("role common level is not digit");
        return TK_ERROR;
    }
    app_context.role_common_level = atoi(buffer);

    memset(buffer, 0, sizeof(buffer));
    ret = parse_key_value(role_common_context, "hp", buffer, sizeof(buffer));
    if(ret != TK_OK) return TK_ERROR;
    if(false == is_all_digits(buffer)){
        log_error("role common hp is not digit");
        return TK_ERROR;
    }
    app_context.role_common_hp = atoi(buffer);

    memset(buffer, 0, sizeof(buffer));
    ret = parse_key_value(role_common_context, "mp", buffer, sizeof(buffer));
    if(ret != TK_OK) return TK_ERROR;
    if(false == is_all_digits(buffer)){
        log_error("role common mp is not digit");
        return TK_ERROR;
    }
    app_context.role_common_mp = atoi(buffer);

    return TK_OK;
}

void show_role(role_t* role)
{
    body_attribute_t* attr = NULL;
    body_attribute_growth_t* attr_growth = NULL;
    log_text("name: %s", role->name);
    log_text("    hp: %d/%d", role->hp, role->hp_max);
    log_text("    mp: %d/%d", role->mp, role->mp_max);
    log_text("    gender: %s", role->gender == Female ? "female":"male");
    log_text("    level: %d", role->level);

    attr = &(role->init_body_attribute);
    log_text("    base    attribute: wu = %d tong = %d zhi = %d min = %d shi = %d speed = %d", 
                        attr->wu, attr->tong, attr->zhi, attr->min, attr->shi, attr->speed);
    attr_growth = &(role->body_attribute_growth);
    log_text("    growth  attribute: wu = %c tong = %c zhi = %c min = %c shi = %c speed = %c", 
                        growth_to_char(attr_growth->wu), growth_to_char(attr_growth->tong), growth_to_char(attr_growth->zhi), 
                        growth_to_char(attr_growth->min), growth_to_char(attr_growth->shi), growth_to_char(attr_growth->speed));
    attr = &(role->curent_body_attribute);
    log_text("    current attribute: wu = %d tong = %d zhi = %d min = %d shi = %d speed = %d", 
                        attr->wu, attr->tong, attr->zhi, attr->min, attr->shi, attr->speed);
    
    log_text("    allocate_attribute_points: %d", role->allocate_attribute_points);
    //log_text("status: %d\n", role->status);
    //log_text("self_tactics: %s\n", role->self_tactics.description);
    //log_text("learned_tactics: %s %s\n", role->learned_tactics[0].description, role->learned_tactics[1].description);
}

char growth_to_char(int growth)
{
    switch(growth){
        case 1: return 'C';
        case 2: return 'B';
        case 3: return 'A';
        case 4: return 'S';
        case 5: return 'X';
        case 6: return 'Y';
        case 7: return 'Z';
        default: return '?';
    }
}