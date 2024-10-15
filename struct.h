
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
    const char name[30];
    gender_t gender;
    int level;
    role_status_t status;
    int troops;
    int allocate_attribute_points;
    body_attribute_t init_body_attribute;
    body_attribute_t curent_body_attribute;
    body_attribute_growth_t body_attribute_growth;
    tactics_t self_tactics;
    tactics_t learned_tactics[2];
}role_t;