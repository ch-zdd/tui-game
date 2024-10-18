
#ifndef TK_STRUCT_H
#define TK_STRUCT_H


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
    int force;
    int intelligence;
    int defense;
    int agile;
    int morale;
    int speed;
} body_attribute_t;

typedef struct{
    int attack_growth;
    int intelligence_growth;
    int defense_growth;
    int agile_growth;
    int morale_growth;
    int speed_growth;    
} body_attribute_growth_t;

typedef struct{
    const char name[30];
    const char description[100];
    tatctics_type_t type;
    //int level;
    //int status;
} tactics_t;

typedef struct{
    const char name[30]; //角色姓名   
    gender_t gender;   //角色性别
    int level;  //等级
    role_status_t status; //角色状态
    int troops; //兵力，相当于hp，但是可自由分配
    int allocate_attribute_points; //自由分配属性点
    body_attribute_t init_body_attribute; //初始身板
    body_attribute_t curent_body_attribute; //当前身板
    body_attribute_growth_t body_attribute_growth;  //身板成长
    tactics_t self_tactics; //自带战法
    tactics_t learned_tactics[2];  //已学战法
}role_t;

#endif