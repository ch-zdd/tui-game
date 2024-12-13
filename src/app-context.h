
#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "../lib/common.h"
#include <ncurses.h>

#define MAX_ROLE_NUM 10
#define MAX_ROLE_DATA_SIZE 1024
#define MAX_INNATE_SKILL_NUM 5
#define MAX_NAME_STRING_LEN 30
#define MAX_THREAD_NUM 2

typedef enum {
    Male,
    Female
} tk_gender_t;

typedef enum{
    BATTLER_STATE_NORMAL,
    BATTLER_STATE_SKILL_DOWN,
    BATTLER_STATE_ATTACK_DOWN,
    BATTLER_STATE_ALL_DOWN,
    BATTLER_STATE_CONFUSION,
    BATTLER_STATE_DEAD,
}tk_battler_state_t;

typedef enum{
    POSITION_STATE_ALIVE,
    POSITION_STATE_DEAD,
    POSITION_STATE_INVALID,
}tk_position_state_t;

#define MASK_TYPE_SKILL(skill_num) (skill_num == SKILL_ATTR_BUFF || skill_num == SKILL_ATTR_DEBUFF || skill_num == SKILL_RECOVERY_STATE)

typedef enum{
    SKILL_NONE,
    SKILL_ACTION_FIRST = 1,
    SKILL_ATTACK_FIRST = 2,
    SKILL_NO_COUNTER = 3,
    SKILL_ACTIVE_COMBO_RATIO = 4,
    SKILL_COUNTER_COMBO_RATIO = 5,
    SKILL_CRIT_RATIO = 6,
    SKILL_CRIT_DAMAGE = 7,
    SKILL_ATTR_BUFF = 8, //效果值决定何种buff提升
    SKILL_ATTR_DEBUFF = 9, //效果值决定何种deff攻击
    SKILL_RECOVERY_STATE = 10, //效果值决定恢复能力恢复还是控制状态恢复
    SKILL_RECOVERY_MP = 11,
    SKILL_RECOVERY_HP = 12,
    SKILL_RAND_STATUS_ATTACK = 13,
    SKILL_RAND_ATTR_ATTACK = 14,
    SKILL_MAX = 64
}tk_innate_skill_num_t;

typedef enum{
    ATTR_TYPE_NONE = 0x00,
    ATTR_TYPE_FORCE = 0x01,
    ATTR_TYPE_INTELLIGENCE = 0x02,
    ATTR_TYPE_DEFENSE = 0x04,
    ATTR_TYPE_AGILITY = 0x08,
    ATTR_TYPE_MORALE = 0x10,
    ATTR_TYPE_SPEED = 0x20,
    ATTR_TYPE_ALL = 0x3F
}tk_attr_type_t;

typedef struct{
    char name[30];
    int skill_value;
    tk_innate_skill_num_t skill_num;
}tk_innate_skill_t;

typedef struct{
    struct{
        float base;
        float growth;
    }force,intelligence,defense,agile,morale,speed;//必须按照 武智防敏士速 排列
} tk_role_attr_t;

typedef struct{
    uint64_t innate_mask; //天赋是否存在的标志,也指向技能结构体的的索引
    tk_innate_skill_t innate_skill[SKILL_MAX]; //天赋
}tk_loaded_innate_t;

typedef struct{
    char name[MAX_NAME_STRING_LEN]; //角色姓名   
    tk_gender_t gender;   //角色性别
    int level;  //等级
    int hp_max;
    int mp_max;
    int allocate_attribute_points; //自由分配属性点
    tk_role_attr_t attr; //身板
    tk_loaded_innate_t loaded_innate; //天赋
    int nof_innate_skill;//拥有的天赋数量
}tk_role_t;

typedef struct{
    int scr_line;
    int scr_col;
    WINDOW* w;
    char store_path[MAX_PATH_LEN];
    bool active;
}tk_window_t;

typedef struct{
    tk_window_t main;
    tk_window_t battler_info;
    tk_window_t battle_report;
    tk_window_t battle;
}tk_tui_t;

typedef struct{
    char cfg_path[MAX_PATH_LEN];
    char game_cfg_path[MAX_PATH_LEN];

    tk_tui_t tui;
    pthread_t tk_thread[MAX_THREAD_NUM];

    int battle_window_width;

    tk_role_t role[MAX_ROLE_NUM];
    int role_num;
    
    //角色统一的初始设定
    int role_common_level;
    int role_common_hp;
    int role_common_mp;
}tk_context_t;

typedef enum{
    ATTR_NORMAL,
    ATTR_UP,
    ATTR_DOWN,
}tk_attr_state_t;

typedef enum{
    BATTLE_TYPE_NONE,
    BATTLE_TYPE_ENEMY,
    BATTLE_TYPE_SELF
}battle_type_t;

typedef struct{
    tk_attr_state_t state;
    int duration;//持续回合
    int current_value;
    int base_value;
    float buff_ratio; //buff提升或降低的比例
}battler_attr_t;

typedef struct{
    char name[MAX_NAME_STRING_LEN];
    int position; //角色界面位置，1234等
    battle_type_t type;
    tk_battler_state_t state; //角色状态
    tk_loaded_innate_t loaded_innate;
    int loaded_skill_num;
    int hp; //场内血条
    int hp_max; //场外血条
    int mp; //场内蓝条
    int mp_max; //场外蓝条
    battler_attr_t force; //武力
    battler_attr_t intelligence; //智力
    battler_attr_t defense; //防御
    battler_attr_t agile; //敏捷
    battler_attr_t morale; //士气
    battler_attr_t speed; //速度
}battler_t;

tk_context_t* get_app_context(void);

int set_cfg_path(const char* path);
int load_app_cfg(const char* cfg_path);
int set_game_cfg_path(const char* path);

int load_game_cfg(void);

void show_role(tk_role_t* role);
char growth_to_char(int growth);
const char* role_state_to_string(tk_battler_state_t state);
const char* role_innate_to_string(tk_innate_skill_num_t innate_num, int skill_value);
const char* attr_buff_to_string(tk_innate_skill_num_t num, int value);

tk_role_t* search_role_by_name(const char* name);

#endif