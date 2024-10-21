
#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "../lib/common.h"

#define MAX_ROLE_NUM 10
#define MAX_ROLE_DATA_SIZE 1024

typedef enum{
    active,
    passive
}tatctics_type_t;

typedef enum {
    Male,
    Female
} gender_t;

typedef enum{
    alive,
    dead
}role_status_t;

typedef struct{
    int wu;
    int zhi;
    int tong;
    int min;
    int shi;
    int speed;
} body_attribute_t;

typedef struct{
    int wu;
    int zhi;
    int tong;
    int min;
    int shi;
    int speed;    
} body_attribute_growth_t;

typedef struct{
    const char name[30];
    const char description[100];
    tatctics_type_t type;
    //int level;
    bool active;
} tactics_t;

typedef struct{
    char name[30]; //角色姓名   
    gender_t gender;   //角色性别
    int level;  //等级
    role_status_t status; //角色状态
    int hp; //当前血量，限制了兵力
    int hp_max;
    int mp; //当前蓝条，限制了战法释放
    int mp_max;
    int allocate_attribute_points; //自由分配属性点
    body_attribute_t init_body_attribute; //初始身板
    body_attribute_t curent_body_attribute; //当前身板
    body_attribute_growth_t body_attribute_growth;  //身板成长
    tactics_t self_tactics; //自带战法
    tactics_t learned_tactics[2];  //已学战法
}role_t;

typedef struct{
    char cfg_path[MAX_PATH_LEN];
    char role_path[MAX_PATH_LEN];

    role_t role[MAX_ROLE_NUM];
    int role_num;
    
    //角色统一的初始设定
    int role_common_level;
    int role_common_hp;
    int role_common_mp;
}app_context_t;

app_context_t* get_app_context(void);

void set_cfg_path(const char* path);
int load_cfg(const char* cfg_path);
int set_role_path(const char* path);

int load_role(void);

void show_role(role_t* role);
char growth_to_char(int growth);

#endif