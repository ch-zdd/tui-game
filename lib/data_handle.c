#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "common.h"
#include "data_handle.h"

int count_digits(int number)
{
    if (number == 0) return 1; // 特殊情况：0有1位
    number = (number < 0) ? -number : number; // 处理负数
    return (int)log10(number) + 1;
}

element_para_t get_elements_para(char* str, char delimiter)
{
    int max_len = 0;
    int count = 0;
    element_para_t ele = {};
    char* p = str;
    char* p1 = NULL;
    char* p2 = NULL;

    p1 = p;
    while(1){
        if(*p == delimiter || *p == '\0'){
            count++;
            p2 = p;
            if(max_len < (p2 - p1)){
                max_len = p2 - p1;
            }
            p1 = p+1;
        }

        if(*p == '\0'){
            break;
        }
        p++;
    }

    ele.max_length = max_len;
    ele.number = count;
    return ele;
}

/*
 * @param data 包含连续数组元素的数据字符串
 * @param ele 用于存储提取的数组元素的缓冲区
 * @param ele_max_size 缓冲区ele的最大大小，以字符为单位
 * @param delimiter 用于分隔数组元素的分隔符字符串
 * 
 * @return 返回指向分隔符后的数据字符串部分的指针，如果到达数据末尾或发生错误，则返回NULL
 */
char* pasrse_array(const char* data, char* ele, size_t ele_max_size, const char* delimiter)
{
    char* p = NULL;
    const char* pstart = NULL;
    size_t data_len = 0;
    size_t ele_len = 0;

    if(data == NULL || ele == NULL || delimiter == NULL){
        log_error("Input NULL ptr");
        return NULL;
    }

    data_len = strlen(data);
    if(data_len == 0 || strlen(delimiter) == 0 || ele_max_size == 0){
        log_warn("Input empty params");
        return NULL;
    }

    memset(ele, 0, ele_max_size);
    pstart = data;
    p = strstr(data, delimiter);
    if(p == NULL){
        ele_len = data_len;
        memcpy(ele, data, ele_len);
        ele[ele_len] = '\0';
        trim(ele);
        return NULL;
    }

    ele_len = p - pstart;
    if(ele_len >= ele_max_size){
        log_warn("element is too long, the element will be truncated!");
        ele_len = ele_max_size - 1;
    }
    memcpy(ele, data, ele_len);
    ele[ele_len] = '\0';
    trim(ele);

    return p+strlen(delimiter);
}

/**
 * 解析配置文件中的标签值
 * 
 * 该函数从给定的数据中解析出指定标签的值它首先查找标签名称，然后找到标签值的开始和结束位置，
 * 并将值复制到提供的缓冲区中
 * 
 * @param data 配置文件的数据
 * @param label_name 要解析的标签名称
 * @param label_context 用于存储标签值的缓冲区
 * @return 标签值的结尾, 如果解析过程中出现错误或数据格式不正确，则可能返回NULL
 */
const char* parse_cfg_label(const char* data, const char* label_name, char* label_context)
{
    // 初始化指针，用于遍历数据和寻找标签值的开始和结束位置
    const char* p = data;
    const char* pstart = NULL;
    const char* pend = NULL;

    // 检查输入参数的有效性，如果任何一个参数为NULL，则记录错误并返回NULL
    if(data == NULL || label_name == NULL || label_context == NULL){
        log_error("Invalid input");
        return NULL;
    }

    // 尝试找到标签名称在数据中的位置，如果找不到，则返回NULL
    pstart = strstr(data, label_name);
    if(pstart == NULL){
        return NULL;
    }

    // 找到标签名称后行的第一个换行符位置，如果找不到，则记录错误并返回NULL
    pstart = strchr(pstart, '\n');
    if(pstart == NULL){
        log_error("Invalid data format");
        return NULL;
    }
    // 跳过换行符，开始寻找标签值的结束位置
    pstart++;
    p = pstart;

    // 循环查找标签值的结束位置
    while (1)
    {
        // 如果遇到换行符且其后不是空格或制表符，则认为是标签值的结束位置
        if(*p == '\n'){
            if(*(p+1) != ' ' && *(p+1) != '\t'){
                pend = p;
                break;
            }
        }

        // 如果遇到字符串的结尾，则认为是标签值的结束位置
        if(*p == '\0'){
            pend = p;
            break;
        }

        // 继续遍历数据
        p++;
    }

    // 将标签值复制到label_context缓冲区中
    memcpy(label_context, pstart, pend-pstart);
    // 在label_context缓冲区中终止字符串
    label_context[pend-pstart] = '\0';

    // 返回解析操作结束时的指针位置
    return p;
}

char* file_to_string(const char* file_name)
{
    long file_len = 0;
    char* data_str = NULL;

    if(file_name == NULL || file_name[0] == '\0'){ 
        log_error("Empty path name");
        return NULL;
    }

    FILE* file = fopen(file_name, "r");
    if(file == NULL){
        log_error("Cannot open data file [%s]: %s", file_name, strerror(errno));
        return NULL;
    }

    if(0 != fseek(file,0,SEEK_END)){
        log_error("fseek file [%s]: %s", file_name, strerror(errno));
        goto READ_ERROR;
    }

    file_len = ftell(file);
    if(file_len < 0){
        log_error("ftell file [%s]: %s", file_name, strerror(errno));
        goto READ_ERROR;
    }

    if(file_len == 0){
        log_error("The file of resource is empty!");
        goto READ_ERROR;
    }

    rewind(file);
    data_str = (char*)tg_malloc(file_len+1);
    if(data_str == NULL){
        log_error("Failed to malloc for file [%s]: %s", file_name, strerror(errno));
        goto READ_ERROR;
    }
    if(file_len != fread(data_str, 1, file_len, file)){
        log_error("Failed to read data file [%s]", file_name);
        goto READ_ERROR;
    }

    fclose(file);
    data_str[file_len] = '\0';

    return data_str;

READ_ERROR:
    if(file != NULL) fclose(file);
    if(data_str != NULL) tg_free(data_str);
    return NULL;
}

// 去除字符串首尾空格
void trim(char *str)
{

    if(str == NULL){
        return;
    }

    int start = 0; // 字符串开始位置
    int end = strlen(str) - 1; // 字符串结束位置

    // 寻找字符串起始位置
    while (isspace((unsigned char)str[start])) {
        start++;
    }

    // 寻找字符串结束位置
    while (end >= 0 && isspace((unsigned char)str[end])) {
        end--;
    }

    // 修改字符串长度
    str[end + 1] = '\0'; // 确保字符串以'\0'结尾

    // 如果有需要，移动字符串起始位置
    if (start > 0) {
        memmove(str, str + start, end - start + 2);
    }
}

void remove_quotation_marks(char* str)
{
    if(str == NULL){
        return;
    }

    int len = strlen(str);
    if(len >= 2 && str[0] == '"' && str[len-1] == '"'){
        memmove(str, str+1, len-2);
        str[len-2] = '\0';
    }
}

bool is_all_digits(const char *str)
{
    if (str == NULL) {
        return false;
    }

    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return false;
        }
        str++;
    }

    return true;
}

bool is_float_num(const char *str)
{
    if (str == NULL) {
        return false;
    }

    while (*str) {
        if (*str != '.' ) {
            if(*str < '0' || *str > '9')
                return false;
        }else{
            if(*(str+1) == '\0'){
                return false;
            }
        }

        str++;
    }

    return true;
}

/**
 * 解析键值对字符串
 * 
 * @param data 原始数据字符串
 * @param key 要解析的键
 * @param value 解析后的值存储结构
 * @param value_type 期望解析的值类型
 * 
 * @return 返回TG_OK表示成功，否则返回TG_ERROR表示失败
 * 
 * 此函数旨在从给定的字符串数据中，解析出指定键对应的值，任何类型的值都视为字符串
 */
int parse_key_value(const char* data, const char* key, char* const value, size_t value_max_len)
{
    char* pstr = NULL;
    char* p_start = NULL;
    char* p_end = NULL;
    size_t value_len = 0;

    // 检查输入参数是否为NULL
    if(data == NULL || key == NULL || value == NULL){
        log_error("Invalid input");
        return TG_ERROR;
    }

    // 在数据字符串中寻找键的位置
    if((pstr = strstr(data, key) )== NULL){
        log_error("No key [%s] found", key);
        return TG_ERROR;
    }

    // 键找到后，确定值的开始位置和结束位置
    p_start = pstr + strlen(key);
    p_start = strchr(p_start, ':');
    if(p_start == NULL){
        log_error("No value found for key [%s]", key);
        return TG_ERROR;
    }
    p_start++;
    p_end = strchr(p_start, '\n');
    if(p_end == NULL){
        p_end = strchr(p_start, '\0');
        if(p_end == NULL){
            log_error("Invalid data format");
            return TG_ERROR;
        }
    }

    //跳过首尾空白字符
    while (isspace((unsigned char)(*p_start))) {
        p_start++;
    }
    while (isspace((unsigned char)(*p_end))) {
        p_end--;
    }
    value_len = p_end - p_start+1;

    // 检查值的长度是否超过最大限制
    if(value_len >= value_max_len){
        log_warn("The value of [%s] is too long", key);
        value_len = value_max_len - 1;
    }

    // 复制值到输出参数，
    memcpy(value, p_start, value_len);
    value[value_len]= '\0';

    return TG_OK;
}

void comment_remove(char* cfg_context)
{
    if(cfg_context == NULL){
        log_error("Invalid input");
        return;
    }

    char* p = NULL;
    char* p_copy_start = NULL;
    char* p_string_start = NULL;
    char* p_string_end = NULL;
    char* p_cfg_context = NULL;
    size_t copy_len = 0;
    char* result = (char*)tg_malloc(strlen(cfg_context)+1);
    memset(result, 0, strlen(cfg_context)+1);
 
    p_string_start = cfg_context;
    p_string_end = cfg_context + strlen(cfg_context);
    p_copy_start = cfg_context;
    p_cfg_context = result;
    p = cfg_context;
    while(1){
        p = strchr(p_copy_start, '#');
        if(p == NULL){
            p = p_string_end;
        }

        copy_len = p - p_copy_start;
        memcpy(p_cfg_context, p_copy_start, copy_len);
        p_cfg_context += copy_len;
        
        p_copy_start = strchr(p, '\n');
        

        if(p_copy_start == NULL || p_copy_start+1 >= p_string_end){
            break;
        }

        if(p == p_string_end) {
            break;
        }
        if(p > p_string_end) {
            log_error("comment_remove error, %ld", p - p_string_end);
            break;
        }
    
        //整行都是注释，删除整行
        if(p == p_string_start || *(p-1) == '\n'){
            p_copy_start++;
            continue;
        }
    }

    memset(cfg_context, 0, strlen(cfg_context));
    memcpy(cfg_context, result, strlen(result));
    cfg_context[strlen(result)] = '\0';
    tg_free(result);
}

