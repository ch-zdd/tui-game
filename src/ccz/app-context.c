#include <string.h>

#include "app-context.h"
#include "../../lib/data_handle.h"

#define SKILL_NUM SKILL_MAX

static tk_context_t context;
#if 0
static tk_innate_skill_t common_innate_skill[SKILL_NUM] = {
    {"", 0, SKILL_NONE},
    {"先手行动", 0, SKILL_ACTION_FIRST},
    {"先手攻击", 0, SKILL_ATTACK_FIRST},
    {"无反击攻击", 0, SKILL_NO_COUNTER},
    {"主动连击率", 0, SKILL_ACTIVE_COMBO_RATIO,},
    {"反击双击率", 0, SKILL_COUNTER_COMBO_RATIO,},
    {"爆击率", 0, SKILL_CRIT_RATIO},
    {"爆伤率", 0, SKILL_CRIT_DAMAGE},
    {"每回合能力自动上升", 0, SKILL_ATTR_BUFF},
    {"能力下降攻击攻击", 0, SKILL_ATTR_DEBUFF},
    {"每回合恢复异常状态", 0, SKILL_RECOVERY_STATE},
    {"每回合恢复蓝量", 0, SKILL_RECOVERY_MP},
    {"每回合恢复血量", 0, SKILL_RECOVERY_HP},
    {"随机属性攻击", 0, SKILL_RAND_ATTR_ATTACK},
    {"", 0, SKILL_MAX}
};
#endif

int load_role_attr(tk_role_t* role, const char* role_context);
int load_role_common_attr(const char* role_common_context);
int load_role_innate(tk_innate_skill_t* skill, const char* innate_skill_context);

tk_context_t* get_app_context(void)
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

int load_app_cfg(const char* cfg_path)
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

    if(parse_key_value(cfg_str, "battle_window_width", buffer, 1024) == TG_OK) {
        if(atoi(buffer)<=0){
            log_error("Invalid battle window width");
            return TG_ERROR;
        }
        context.battle_window_width = atoi(buffer);
    }else{
        context.battle_window_width = 48;
    }

    free(cfg_str);
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

int load_game_cfg(void)
{
    tk_context_t* ctx = &context;
    const char* p = NULL;
    int count = 0;
    log_info("load game setting, path: %s", ctx->game_cfg_path);
    char* game_cfg_str = file_to_string(ctx->game_cfg_path);
    if(game_cfg_str == NULL) {
        log_error("could not open file to read role file");
        goto read_error;
    }
    comment_remove(game_cfg_str);

    //分割不同角色的数据,并加载不同角色的数据
    char role_buffer[MAX_ELEMENTS_SIZE];
    p = game_cfg_str;
    while(1){
        if(ctx->role_num > MAX_ROLE_NUM){
            log_warn("role num is too much, ignore the subsequent roles");
            break;
        }
        memset(role_buffer, 0, MAX_ELEMENTS_SIZE);
        p = parse_cfg_label(p, "@role_attr", role_buffer);
        if(p == NULL){
            break;
        }
 
        if(load_role_attr(&(ctx->role[count]), role_buffer) != TG_OK){
            log_warn("load this role failed");
            log_text("role=>\n%s\n", role_buffer);
            continue;
        }
        count++;
    }
    ctx->role_num = count;

    //加载角色的共同设定
    log_info("load role common setting");
    memset(role_buffer, 0, MAX_ELEMENTS_SIZE);
    if(NULL == parse_cfg_label(game_cfg_str, "@role_common", role_buffer)){
        log_error("No role common setting label found");
        goto read_error;
    }

    if(TG_OK != load_role_common_attr(role_buffer)){
        log_error("load role common setting context failed");
        goto read_error;
    }


    tg_free(game_cfg_str);
    return TG_OK;

read_error:
    tg_free(game_cfg_str);
    return TG_ERROR;
}

int load_role_attr(tk_role_t* role, const char* role_context)
{
    int ret = TG_OK;
    char buffer[1024] = {};
    char* array_ptr = NULL;
    char ele[10] = {};
    float attr[10] = {};
    int attr_index = 0;

    ret = parse_key_value(role_context, "name", role->name, sizeof(role->name));
    if(ret != TG_OK) {
        log_error("parse role name failed");
        return TG_ERROR;
    }

    ret = parse_key_value(role_context, "gender", buffer, 10);
    if(ret != TG_OK){
        log_error("gender parse failed");
        return TG_ERROR;
    }
    if(strcmp(buffer, "female") == 0){
        role->gender = Female;
    }else if(strcmp(buffer, "male") == 0){
        role->gender = Male;
    }else{
        log_warn("unknown gender:%s", buffer);
    }

    //属性必须按顺序排列
    memset(buffer, 0, sizeof(buffer));
    ret = parse_key_value(role_context, "attr", buffer, 1024);
    if(ret != TG_OK) {
        log_error("parse_key_value failed, key is attr");
        return TG_ERROR;
    }

    array_ptr = buffer;
    while(1){
        array_ptr = pasrse_array(array_ptr, ele, 10, ",");
        if(!is_float_num(ele)){
            log_error("attr value is not float number");
            return TG_ERROR;
        }
        attr[attr_index] = atof(ele);
        attr_index++;
        if(array_ptr == NULL || attr_index >= 6){
            break;
        }
    }
    role->attr.force.base = attr[0];
    role->attr.intelligence.base = attr[1];
    role->attr.defense.base = attr[2];
    role->attr.agile.base = attr[3];
    role->attr.morale.base = attr[4];
    role->attr.speed.base = attr[5];
    memset(attr, 0, sizeof(attr));

    //属性成长也是同样的按顺序排列
    memset(buffer, 0, sizeof(buffer));
    ret = parse_key_value(role_context, "growth", buffer, 1024);
    if(ret != TG_OK){
        log_error("parse_key_value failed, key is growth");
        return TG_ERROR;
    }

    array_ptr = buffer;

    attr_index = 0;
    while(1){
        array_ptr = pasrse_array(array_ptr, ele, 10, ",");
        if(!is_float_num(ele)){
            log_error("attr value is not float number");
            return TG_ERROR;
        }
        attr[attr_index] = atof(ele);
        attr_index++;
        if(array_ptr == NULL || attr_index >= 6){
            break;
        }
    }
    role->attr.force.growth = attr[0];
    role->attr.intelligence.growth = attr[1];
    role->attr.defense.growth = attr[2];
    role->attr.agile.growth = attr[3];
    role->attr.morale.growth = attr[4];
    role->attr.speed.growth = attr[5];

    //加载角色天赋
    memset(buffer, 0, sizeof(buffer));
    ret = parse_key_value(role_context, "innate", buffer, 1024);
    if(ret != TG_OK){
        log_warn("load innate failed, ignore innate info");
        return TG_OK;
    }

    array_ptr = buffer;
    attr_index = 0;
    memset(&role->loaded_innate, 0, sizeof(role->loaded_innate));
    tk_innate_skill_t skill_tmp;
    int name_len = 0;
    while(1){
        array_ptr = pasrse_array(array_ptr, ele, 10, ",");

        memset(&skill_tmp, 0, sizeof(skill_tmp));
        if(load_role_innate(&skill_tmp, ele) != TG_OK){
            log_warn("load innate skill failed");
            break;
        }

        role->loaded_innate.innate_skill[skill_tmp.skill_num].skill_num = skill_tmp.skill_num;
        role->loaded_innate.innate_skill[skill_tmp.skill_num].skill_value |= skill_tmp.skill_value;
        role->loaded_innate.innate_mask |= 0x01 << skill_tmp.skill_num;

        name_len = strlen(role->loaded_innate.innate_skill[skill_tmp.skill_num].name);
        memcpy(&(role->loaded_innate.innate_skill[skill_tmp.skill_num].name[name_len]), skill_tmp.name, strlen(skill_tmp.name));
        log_debug("load innate skill num %d, value %d, name %s", skill_tmp.skill_num, skill_tmp.skill_value, skill_tmp.name);
        attr_index++;

        if(array_ptr == NULL || attr_index >= MAX_INNATE_SKILL_NUM){
            break;
        }
        
    }

    role->nof_innate_skill = attr_index;

    for(int i = 0; i<SKILL_MAX; i++){
        
        if(!(role->loaded_innate.innate_mask >> i & 0x01)){
            continue;
        }
        log_debug("name = %s, num = %d, value = %d, mask = %d pass", role->name, i, role->loaded_innate.innate_skill[i].skill_value, role->loaded_innate.innate_mask);
    }

    return TG_OK;
}

int load_role_common_attr(const char* role_common_context)
{
    int ret = TG_OK;
    char buffer[10] = {};

    ret = parse_key_value(role_common_context, "level", buffer, sizeof(buffer));
    if(ret != TG_OK) return TG_ERROR;
    if(false == is_all_digits(buffer)){
        log_error("role common level is not digit");
        return TG_ERROR;
    }
    context.role_common_level = atoi(buffer);

    memset(buffer, 0, sizeof(buffer));
    ret = parse_key_value(role_common_context, "hp", buffer, sizeof(buffer));
    if(ret != TG_OK) return TG_ERROR;
    if(false == is_all_digits(buffer)){
        log_error("role common hp is not digit");
        return TG_ERROR;
    }
    context.role_common_hp = atoi(buffer);

    memset(buffer, 0, sizeof(buffer));
    ret = parse_key_value(role_common_context, "mp", buffer, sizeof(buffer));
    if(ret != TG_OK) return TG_ERROR;
    if(false == is_all_digits(buffer)){
        log_error("role common mp is not digit");
        return TG_ERROR;
    }
    context.role_common_mp = atoi(buffer);

    return TG_OK;
}

int load_role_innate(tk_innate_skill_t* skill, const char* innate_skill_context)
{
    int skill_num = 0;
    int skill_value = 0;
    int ret = TG_OK;
    const char* name = NULL;

    if(skill == NULL || innate_skill_context == NULL){
        return TG_ERROR;
    }

    
    ret = sscanf(innate_skill_context, "%d/%d", &skill_num, &skill_value);
    if(ret != 2){
        log_error("innate skill format error");
        return TG_ERROR;
    }

    if(skill_num>=SKILL_MAX || skill_num <0 || skill_value<0 || skill_value>100){
        log_error("innate skill parameter error");
        return TG_ERROR;
    }

    if(MASK_TYPE_SKILL(skill_num)){
        skill_value = skill_value == 0 ? ATTR_TYPE_ALL: (1<<(skill_value-1));
    }

    skill->skill_num = skill_num;
    skill->skill_value = skill_value;

    name = role_innate_to_string(skill->skill_num, skill->skill_value);
    if(name == NULL){
        log_error("Failed to get name of role innate skill");
        return TG_ERROR;
    }

    memcpy(skill->name, name, strlen(name));

    return TG_OK;
}

void show_role(tk_role_t* role)
{
    log_text("name: %s", role->name);
    log_text("    hp_max: %d", role->hp_max);
    log_text("    mp_max: %d", role->mp_max);
    log_text("    gender: %s", role->gender == Female ? "female":"male");
    log_text("    level: %d", role->level);
    log_text("    attribute(base/growth):");
    log_text("        force (%0.2f/%0.2f) defense (%0.2f/%0.2f) intelligence = (%0.2f/%0.2f) agile (%0.2f/%0.2f) morale (%0.2f/%0.2f) speed (%0.2f/%0.2f)", 
                        role->attr.force.base, role->attr.force.growth,
                        role->attr.intelligence.base, role->attr.intelligence.growth,
                        role->attr.defense.base, role->attr.defense.growth,
                        role->attr.agile.base, role->attr.agile.growth,
                        role->attr.morale.base, role->attr.morale.growth,
                        role->attr.speed.base, role->attr.speed.growth);
    
    log_text("    allocate_attribute_points: %d", role->allocate_attribute_points);
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

const char* role_state_to_string(tk_battler_state_t state)
{
    switch(state){
        case BATTLER_STATE_NORMAL: return "NORMAL";
        case BATTLER_STATE_ATTACK_DOWN: return "ATTACK_DOWN";
        case BATTLER_STATE_CONFUSION: return "CONFUSION";
        case BATTLER_STATE_DEAD: return "DEAD";
        case BATTLER_STATE_ALL_DOWN: return "ALL_DOWN";
        default: return "UNKNOWN";
    }
}

const char* attr_buff_to_string(tk_innate_skill_num_t num, int value)
{
    if(num == SKILL_ATTR_BUFF){
        switch(value){
            case ATTR_TYPE_ALL: return "霸气";
            case ATTR_TYPE_FORCE: return "升攻";
            case ATTR_TYPE_INTELLIGENCE: return "升智";
            case ATTR_TYPE_DEFENSE: return "升防";
            case ATTR_TYPE_AGILITY: return "升爆";
            case ATTR_TYPE_MORALE: return "升士";
            case ATTR_TYPE_SPEED: return "升速";
            default: return "未定义特效值";
        }
    }else if(num == SKILL_ATTR_DEBUFF){
        switch(value){
            case ATTR_TYPE_ALL: return "衰气";
            case ATTR_TYPE_FORCE: return "破攻";
            case ATTR_TYPE_INTELLIGENCE: return "破智";
            case ATTR_TYPE_DEFENSE: return "破防";
            case ATTR_TYPE_AGILITY: return "破爆";
            case ATTR_TYPE_MORALE: return "破士";
            case ATTR_TYPE_SPEED: return "破速";
            default: return "未定义特效值";
        }
    }else{
        return NULL;
    }
}

const char* role_innate_to_string(tk_innate_skill_num_t innate_num, int skill_value)
{
   static char buffer[128] = {};

    switch(innate_num){
        case SKILL_NONE: return "无";
        case SKILL_ACTION_FIRST: return "先手行动";
        case SKILL_ATTACK_FIRST: return "攻击先手";
        case SKILL_NO_COUNTER: return "无反击攻击";
        case SKILL_ACTIVE_COMBO_RATIO: sprintf(buffer, "主动连击率+%d%%", skill_value); return buffer;
        case SKILL_COUNTER_COMBO_RATIO: sprintf(buffer, "反击连击率+%d%%", skill_value); return buffer;
        case SKILL_CRIT_RATIO: sprintf(buffer, "暴击+%d%%", skill_value); return buffer;
        case SKILL_CRIT_DAMAGE: sprintf(buffer, "暴伤+%d%%", skill_value); return buffer;
        case SKILL_ATTR_BUFF: 
        case SKILL_ATTR_DEBUFF: return attr_buff_to_string(innate_num, skill_value);
        case SKILL_RECOVERY_STATE: if(skill_value == 0) return "属性和状态恢复"; else if(skill_value==1) return "状态异常恢复"; else return "属性下降恢复";
        case SKILL_RECOVERY_MP: sprintf(buffer, "每回合恢复mp %d", skill_value); return buffer;
        case SKILL_RECOVERY_HP: sprintf(buffer, "每回合恢复hp %d", skill_value); return buffer;
        case SKILL_RAND_STATUS_ATTACK: return "随机状态攻击";
        case SKILL_RAND_ATTR_ATTACK: return "随机属性攻击";
        default: return "未定义";
    }

   return NULL;
}

tk_role_t* search_role_by_name(const char* name)
{
    int i = 0;

    for(i = 0; i < context.role_num; i++){
        if(strcmp(context.role[i].name, name) == 0){
            return &(context.role[i]);
        }
    }

    return NULL;
}