#include <string.h>
#include <ncurses.h>

#include "app.h"
#include "../lib/common.h"
#include "app-context.h"
#include "app-tui.h"

#define DAMAGE_FACTOR 1.1 //伤害系数
#define DAMAGE_RANDOM_FACTOR 5  //伤害浮动5%

#define BASIC_CRIT_DAMAGE_RATIO 1.5 //基础暴击伤害系数
#define BASIC_COMBO_DAMAGE_RATIO 0.75 //连击伤害系数，反击也算连击

#define DAMAGE_FORMULA(attacker_force, defender_defense) (((attacker_force/(float)defender_defense)/2.0*attacker_force)* DAMAGE_FACTOR)

static battler_t gbattlers[MAX_BATTLER_NUM] = {};
static int gbattlers_num = 0;

battler_t battler_init(tk_role_t* attacker, battle_type_t battle_type);
void set_battler_position(void);

void round_start(int round_index);
void round_end(int round_index);

int select_defender(int attacker_idx);

int battle(void);

int sort_action(void);
int battler_state_refresh(void);

float calc_crit_chance(int attacker_morale, int defender_morale);
float calc_combo_chance(int attacker_agile, int defender_agile);

int attack(battler_t* attacker, battler_t* defender);
int calc_damage(battler_t* attacker, battler_t* defender, float damage_extra_ratio);
void load_innate_attr_buff(battler_t* battler, int skill_value, tk_attr_state_t attr_state, int duration);
int set_attr_buff(battler_attr_t* set_attr, tk_attr_state_t set_state, int set_duration);

void buff_settlement(void);
bool battle_is_over(battler_t* attacker, battler_t* defender);

int app_run(void)
{
    int i = 0;

    for(i = 0; i < get_app_context()->role_num/2 && i< MAX_BATTLER_NUM; i++){
        gbattlers[i] = battler_init(&(get_app_context()->role[i]), BATTLE_TYPE_ENEMY);
    }
    for(; i < get_app_context()->role_num && i< MAX_BATTLER_NUM; i++){
        gbattlers[i] = battler_init(&(get_app_context()->role[i]), BATTLE_TYPE_SELF);
    }
    gbattlers_num = i;

    if(TK_OK != game_window_draw()){
        return TK_ERROR;
    }

    set_battler_position();

    battle();

    return TK_OK;
}

void round_start(int round_index)
{
    battle_report("Round %d start!", round_index);
    sort_action();
    battler_state_refresh();
}

void round_end(int round_index)
{
    buff_settlement();
}

void set_battler_position(void)
{
    int enemy_index = 0;
    int sefl_index = 0;
    for(int i = 0; i < gbattlers_num; i++){
        if(gbattlers[i].type == BATTLE_TYPE_ENEMY){
            gbattlers[i].position = enemy_index+1;
            enemy_index++;
        }else if(gbattlers[i].type == BATTLE_TYPE_SELF){
            gbattlers[i].position = sefl_index+1;
            sefl_index++;
        }
    }
}

int sort_action(void)
{
    for(int i = 0; i < gbattlers_num; i++){
        for(int j = 0; j < gbattlers_num-i-1; j++){
            if(gbattlers[j].speed.current_value < gbattlers[j+1].speed.current_value){
                battler_t tmp = gbattlers[j];
                gbattlers[j] = gbattlers[j+1];
                gbattlers[j+1] = tmp;
            }
        }
    }

    return TK_OK;
}

int battler_state_refresh(void)
{
    int i = 0, j = 0;
    for(i = 0; i < gbattlers_num; i++){
        for(j = 0; j<SKILL_MAX; j++){
            if(!(gbattlers[i].loaded_innate.innate_mask >> j & 0x01)){
                continue;
            }

            if(j == SKILL_ATTR_BUFF){
                load_innate_attr_buff(&gbattlers[i], gbattlers[i].loaded_innate.innate_skill[j].skill_value, ATTR_UP, BUFF_DURATION_INF);
            }
        }

        if(gbattlers[i].force.duration == 0) {
            gbattlers[i].force.current_value = gbattlers[i].force.base_value;
            gbattlers[i].force.state = ATTR_NORMAL;
        }
        if(gbattlers[i].intelligence.duration == 0) {
            gbattlers[i].intelligence.current_value = gbattlers[i].intelligence.base_value;
            gbattlers[i].intelligence.state = ATTR_NORMAL;
        }
        if(gbattlers[i].defense.duration == 0) {
            gbattlers[i].defense.current_value = gbattlers[i].defense.base_value;
            gbattlers[i].defense.state = ATTR_NORMAL;
        }
        if(gbattlers[i].agile.duration == 0) {
            gbattlers[i].agile.current_value = gbattlers[i].agile.base_value;
            gbattlers[i].agile.state = ATTR_NORMAL;
        }
        if(gbattlers[i].morale.duration == 0) {
            gbattlers[i].morale.current_value = gbattlers[i].morale.base_value;
            gbattlers[i].morale.state = ATTR_NORMAL;
        }
        if(gbattlers[i].speed.duration == 0) {
            gbattlers[i].speed.current_value = gbattlers[i].speed.base_value;
            gbattlers[i].speed.state = ATTR_NORMAL;
        }

    }
    return TK_OK;
}

void buff_settlement(void)
{
    for(int i = 0; i < gbattlers_num; i++){
        if(gbattlers[i].force.duration > 0) gbattlers[i].force.duration--;
        if(gbattlers[i].intelligence.duration > 0) gbattlers[i].intelligence.duration--;
        if(gbattlers[i].defense.duration > 0) gbattlers[i].defense.duration--;
        if(gbattlers[i].agile.duration > 0) gbattlers[i].agile.duration--;
        if(gbattlers[i].morale.duration > 0) gbattlers[i].morale.duration--;
        if(gbattlers[i].speed.duration > 0) gbattlers[i].speed.duration--;
    }
}

int battle()
{
    int round_idx = 0;
    int attacker_idx = 0;
    int defender_idx = 0;
    //tk_window_t* w_battle = &(get_app_context()->tui.battler_info);

    while(round_idx<8){
        

        //行动顺序
        round_start(round_idx+1);
        for(attacker_idx = 0; attacker_idx < gbattlers_num; attacker_idx++){
            show_battler();
            defender_idx = select_defender(attacker_idx);
            if(defender_idx < 0) {
                log_warn("ERROR: no defender for attacker %d", attacker_idx);
                return TK_OK;
            }
            attack(&gbattlers[attacker_idx], &gbattlers[defender_idx]);
            revert_battler_info_screen();
            //if(battle_is_over(&gbattlers[attacker_idx], &gbattlers[defender_idx])) return TK_OK;
        }

        round_end(round_idx+1);
        round_idx++;
        
        if(round_idx == 8){
            battle_report("The draw ends!");
        }
    }

    return TK_OK;
}

int select_defender(int attacker_idx)
{
    int defender_position = 1; //默认选择的敌人

    battler_highlight(gbattlers[attacker_idx].type, gbattlers[attacker_idx].position, A_NORMAL, 1);

    if(gbattlers[attacker_idx].type == BATTLE_TYPE_ENEMY){
        defender_position = rand()%MAX_SELF_BATTLER_NUM;
        for(int i = 0; i < gbattlers_num; i++){
            if(gbattlers[i].type == BATTLE_TYPE_SELF && gbattlers[i].position == defender_position+1){
                return i;
            }
        }
    }else if(gbattlers[attacker_idx].type == BATTLE_TYPE_SELF){
        defender_position = select_enemy_tui(attacker_idx);

        for(int i = 0; i < gbattlers_num; i++){
            if(gbattlers[i].type == BATTLE_TYPE_ENEMY && gbattlers[i].position == defender_position){
                return i;
            }
        }
    }
    

    return -1;
}

bool battle_is_over(battler_t* attacker, battler_t* defender)
{
    if(attacker->hp <= 0){
        battle_report("[%s]win", defender->name);
        return true;
    }else if(defender->hp <= 0){
        battle_report("[%s]win", attacker->name);
        return true;
    }else{
        return false;
    }

    return false;
}

int attack(battler_t* attacker, battler_t* defender)
{
    char header[64] = "";
    int print_indetation = 2;
    float damage_extra_ratio = 0;
    float combo_chance = 0;

    float battler_extra_crit_ratio = 0;
    float battler_extra_combo_ratio = 0;

    int attacker_agile = attacker->agile.current_value;
    int defender_agile = defender->agile.current_value;
    int attacker_morale = attacker->morale.current_value;
    int defender_morale = defender->morale.current_value;

    memset(header, ' ', print_indetation);
    battler_blink_delay(defender->type, defender->position, 3, 0);
    if((defender->loaded_innate.innate_mask >> SKILL_ATTACK_FIRST) & 0x01){
        //先手攻击，攻击防守互换
        battler_t* tmp = attacker;
        attacker = defender;
        defender = tmp;

        attacker_agile = attacker->agile.current_value;
        defender_agile = defender->agile.current_value;
        attacker_morale = attacker->morale.current_value;
        defender_morale = defender->morale.current_value;

        battle_report("%s[%s] first attack effect triggered", header, attacker->name);
    }

    if((attacker->loaded_innate.innate_mask >> SKILL_ACTIVE_COMBO_RATIO) & 0x01){
        battler_extra_combo_ratio = attacker->loaded_innate.innate_skill[SKILL_ACTIVE_COMBO_RATIO].skill_value/100.0;
    }
    if((attacker->loaded_innate.innate_mask >> SKILL_CRIT_RATIO) & 0x01){
        battler_extra_crit_ratio = attacker->loaded_innate.innate_skill[SKILL_CRIT_RATIO].skill_value/100.0;
    }

    combo_chance = calc_combo_chance(attacker_agile, defender_agile) + battler_extra_combo_ratio;
    battle_report("%s[%s] attack, combo ratio[%3.2f%%] crit ratio[%3.2f%%]", header, attacker->name,
        combo_chance*100,
        calc_crit_chance(attacker_morale, defender_morale)*100 + battler_extra_crit_ratio*100);

    //第一击
    damage_extra_ratio = 1;
    calc_damage(attacker, defender, damage_extra_ratio);

    //连击
    if(TK_CHANCE_APPEAR(combo_chance)){
        battle_report("%s[%s] $3{combo} [%s]", header, attacker->name, defender->name);
        damage_extra_ratio = BASIC_COMBO_DAMAGE_RATIO;
        calc_damage(attacker, defender, damage_extra_ratio);
    }

    if(defender->hp <= 0 || defender->hp <= 0){
        return TK_OK;
    }

    //反击
    if(defender->loaded_innate.innate_mask >> SKILL_COUNTER_COMBO_RATIO & 0x01){
        combo_chance = defender->loaded_innate.innate_skill[SKILL_COUNTER_COMBO_RATIO].skill_value/100.0;
    }else{
        combo_chance = 0;
    }

    battler_extra_crit_ratio = 0;
    if((defender->loaded_innate.innate_mask >> SKILL_CRIT_RATIO) & 0x01){
        battler_extra_crit_ratio = defender->loaded_innate.innate_skill[SKILL_CRIT_RATIO].skill_value;
    }
    battle_report("%s[%s] counter [%s], combo ratio[%3.2f%%] crit ratio[%3.2f%%]", header, defender->name, attacker->name, 
        combo_chance*100, calc_crit_chance(defender_morale, attacker_morale)*100 + battler_extra_crit_ratio) ;
    damage_extra_ratio = BASIC_COMBO_DAMAGE_RATIO;
    calc_damage(defender, attacker, damage_extra_ratio);

    //反击连击
    if(TK_CHANCE_APPEAR(combo_chance)){
        battle_report("%s[%s] counter $3{combo} [%s]", header, defender->name, attacker->name);
        damage_extra_ratio = BASIC_COMBO_DAMAGE_RATIO;
        calc_damage(defender, attacker, damage_extra_ratio);
    }

    
    return TK_OK;
}

int calc_damage(battler_t* attacker, battler_t* defender, float damage_extra_ratio)
{
    char header[64] = "";
    int print_indetation = 2;
    memset(header, ' ', print_indetation);
    float random_factor = 1+ (float)(rand()%DAMAGE_RANDOM_FACTOR)/100.0;

    float crit_damage_ratio = 0;
    float crit_damage_extra_ratio = 0;

    float crit_chance = 0;
    float crit_extra_chance = 0;

    float basic_damage = 0;
    int final_damage = 0;

    int current_hp = defender->hp;

    int attacker_force = attacker->force.current_value; //武
    int defender_defense = defender->defense.current_value;//防
    int attacker_morale = attacker->morale.current_value;
    int defender_morale = defender->morale.current_value;

    basic_damage = DAMAGE_FORMULA(attacker_force, defender_defense)* random_factor * damage_extra_ratio;
    
    if(attacker->loaded_innate.innate_mask >> SKILL_CRIT_DAMAGE & 0x01) {
        crit_damage_extra_ratio = attacker->loaded_innate.innate_skill[SKILL_CRIT_DAMAGE].skill_value/100.0;
    }

    if(attacker->loaded_innate.innate_mask >> SKILL_CRIT_RATIO & 0x01){
        crit_extra_chance = attacker->loaded_innate.innate_skill[SKILL_CRIT_RATIO].skill_value/100.0;
    }

    crit_chance = calc_crit_chance(attacker_morale, defender_morale);
    crit_damage_ratio = BASIC_CRIT_DAMAGE_RATIO + crit_chance/2.0 + crit_damage_extra_ratio; //根据士气差加成
    crit_chance += crit_extra_chance;

    if(TK_CHANCE_APPEAR(crit_chance)){
        final_damage = (int)(basic_damage*crit_damage_ratio);
        defender->hp -= final_damage;
        battle_report("%s[%s]hp[%d] cause $1{crit} [%d=>%d] damage to [%s]hp[%d=>%d]", header, 
                attacker->name, attacker->hp, (int)basic_damage, final_damage, defender->name, current_hp, defender->hp);
    }else{
        final_damage = (int)basic_damage;
        defender->hp -= final_damage;
        battle_report("%s[%s]hp[%d] cause [%d] damage to [%s]hp[%d=>%d]", header, attacker->name, attacker->hp, final_damage, defender->name, current_hp, defender->hp);
    }

    if(attacker->loaded_innate.innate_mask >> SKILL_ATTR_DEBUFF & 0x01){
        load_innate_attr_buff(defender, attacker->loaded_innate.innate_skill[SKILL_ATTR_DEBUFF].skill_value, ATTR_DOWN, BUFF_DURATION_NOR);
    }

    return 0;
}

float calc_crit_chance(int attacker_morale, int defender_morale)
{
    float crit_chance = 0.0;

    crit_chance = (attacker_morale- defender_morale)/2.0/(float)defender_morale;
    crit_chance = crit_chance < 0.01? 0.01 : crit_chance;
    crit_chance = crit_chance >= 1 ? 1 : crit_chance;

    return crit_chance;
}

float calc_combo_chance(int attacker_agile, int defender_agile)
{
    float combo_chance = 0.0;

    combo_chance = (attacker_agile - defender_agile)/2.0/(float)defender_agile;
    combo_chance = combo_chance < 0.01? 0.01 : combo_chance;
    combo_chance = combo_chance >=1 ? 1 : combo_chance;

    return combo_chance;
}

battler_t battler_init(tk_role_t* attacker, battle_type_t battle_type)
{
    battler_t battler;
    int i = 0;

    memset(&battler, 0, sizeof(battler));

    BATTLER_ATTR_INIT(battler.force, attacker->attr.force, attacker->level);
    battler.force.buff_ratio = 0.2;

    BATTLER_ATTR_INIT(battler.intelligence, attacker->attr.intelligence, attacker->level);
    battler.intelligence.buff_ratio = 0.2;

    BATTLER_ATTR_INIT(battler.defense, attacker->attr.defense, attacker->level);
    battler.defense.buff_ratio = 0.2;

    BATTLER_ATTR_INIT(battler.agile, attacker->attr.agile, attacker->level);
    battler.agile.buff_ratio = 0.3;

    BATTLER_ATTR_INIT(battler.morale, attacker->attr.morale, attacker->level);
    battler.morale.buff_ratio = 0.3;

    BATTLER_ATTR_INIT(battler.speed, attacker->attr.speed, attacker->level);
    battler.speed.buff_ratio = 0.3;

    battler.type = battle_type;
    battler.hp = attacker->hp_max;
    battler.hp_max = attacker->hp_max;
    battler.mp = attacker->mp_max;
    battler.mp_max = attacker->mp_max;
    memcpy(battler.name, attacker->name, sizeof(attacker->name));
    battler.state = BATTLER_STATE_NORMAL;

    battler.loaded_skill_num = attacker->nof_innate_skill;

    memcpy(&battler.loaded_innate, &attacker->loaded_innate, sizeof(tk_loaded_innate_t));


    for(i = 0; i<SKILL_MAX; i++){
        if(!(attacker->loaded_innate.innate_mask >> i & 0x01)){
            continue;
        }

        log_debug("name %s, skill mask %lu, skill num %d", battler.name, battler.loaded_innate.innate_mask, i);
        if(i == SKILL_ATTR_BUFF){
            load_innate_attr_buff(&battler, battler.loaded_innate.innate_skill[i].skill_value, ATTR_UP, BUFF_DURATION_INF);
        }
    }

    return battler;
}

void load_innate_attr_buff(battler_t* battler, int skill_value, tk_attr_state_t attr_state, int duration)
{
    int j = 0;
    
    for(j = 1; j<ATTR_TYPE_ALL; j = j<<1){
        if(skill_value & j){
            //tk_print("[%s] load innate attr buff value %x, j = %x", battler->name, skill_value, j);
            switch(j){
                case ATTR_TYPE_FORCE: set_attr_buff(&battler->force, attr_state, duration); break;    
                case ATTR_TYPE_INTELLIGENCE: set_attr_buff(&battler->intelligence, attr_state, duration); break;
                case ATTR_TYPE_DEFENSE: set_attr_buff(&battler->defense, attr_state, duration); break;
                case ATTR_TYPE_AGILITY: set_attr_buff(&battler->agile, attr_state, duration); break;
                case ATTR_TYPE_MORALE: set_attr_buff(&battler->morale, attr_state, duration); break;
                case ATTR_TYPE_SPEED: set_attr_buff(&battler->speed, attr_state, duration); break;
                default:
                    log_error("Invalid attribute type %d, skill_value = %d", j, skill_value);
            }
        }
    }   

}

int set_attr_buff(battler_attr_t* set_attr, tk_attr_state_t set_state, int set_duration)
{
    do{                                                                     
        set_attr->duration = set_duration;                                                 
        if(set_state == ATTR_UP){
            //无限回合的buff在回合开始时必定为上升状态                                     
            if(set_duration == BUFF_DURATION_INF){                                    
                set_attr->state = ATTR_UP;                                       
                set_attr->current_value = set_attr->base_value*(1+set_attr->buff_ratio);   
                set_attr->duration = BUFF_DURATION_INF;                          
                break;                                                      
            } 
                                                              
            if(set_attr->state == ATTR_DOWN){                                    
                set_attr->state = ATTR_NORMAL;                                   
                set_attr->current_value = set_attr->base_value;                       
                set_attr->duration = 0;                                          
            }else if(set_attr->state == ATTR_NORMAL){                            
                set_attr->state = ATTR_UP;                                       
                set_attr->current_value = set_attr->base_value*(1+set_attr->buff_ratio);   
            }                                                               
        }else if(set_state == ATTR_DOWN){                                   
            if(set_attr->state == ATTR_UP){                                      
                set_attr->state = ATTR_NORMAL;                                   
                set_attr->current_value = set_attr->base_value;                       
                set_attr->duration = 0;                                          
            }else if(set_attr->state == ATTR_NORMAL){                            
                set_attr->state = ATTR_DOWN;                                     
                set_attr->current_value = set_attr->base_value*(1-set_attr->buff_ratio);   
            }                                                               
        }                                                                   
    }while(0);

    return TK_OK;                                                           
}

battler_t* get_battlers(void)
{
    return gbattlers;
}

int get_battlers_num(void)
{
    return gbattlers_num;
}