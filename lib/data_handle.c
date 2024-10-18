#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "common.h"
#include "data_handle.h"

void* tk_malloc(size_t size)
{
	return malloc(size);
}

void tk_free(void* ptr)
{
	free(ptr);
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

int parse_array_to_string(const char* data, char** ele, int ele_max_size, char delimiter)
{
	const char* p = data;
	const char* p1 = NULL;
	const char* p2 = NULL;
	int count = 0;
	int max_len = 0;

	if(data == NULL || ele == NULL){
		log_error("Invalid input");
		return TK_ERROR;
	}

	p1 = p;
	while(1){
		if(*p == delimiter || *p == '\0'){
			count++;
			p2 = p;
			if(max_len < (p2 - p1)){
				max_len = p2- p1;
			}

			if(max_len >= (ele_max_size-1)){ //包括'\0'
				log_error("The element of this array is too long");
				return TK_ERROR;
			}

			//以最大元素长度作为每个元素缓冲区的长度
			memcpy(((char(*)[ele_max_size])ele)[count], p1, p2-p1);
			((char(*)[ele_max_size])ele)[count][p2-p1] = '\0';
			tk_print("{%s}", ((char(*)[ele_max_size])ele)[count]);

			p1 = p+1;
		}

		if(*p == '\0'){
			break;
		}
        p++;
	}

	return count;
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
    data_str = (char*)tk_malloc(file_len+1);
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
    if(data_str != NULL) tk_free(data_str);
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

/**
 * 解析键值对字符串
 * 
 * @param data 原始数据字符串
 * @param key 要解析的键
 * @param value 解析后的值存储结构
 * @param value_type 期望解析的值类型
 * 
 * @return 返回TK_OK表示成功，否则返回TK_ERROR表示失败
 * 
 * 此函数旨在从给定的字符串数据中，解析出指定键对应的值，并根据指定的值类型进行转换和存储
 * 它首先检查输入参数的有效性，然后寻找键在数据字符串中的位置，如果找到，将键后的字符串作为值进行处理
 * 根据传入的值类型，将字符串转换为相应的数据类型，并存储在value参数指向的结构体中
 */
int parse_key_value(const char* data, const char* key, data_t* const value, value_type_t value_type)
{
    char* pstr = NULL;
	char* p_start = NULL;
	char* p_end = NULL;
	int value_len = 0;
	char tmp[1024] = "";

	// 检查输入参数是否为NULL
	if(data == NULL || key == NULL || value == NULL){
		log_error("Invalid input");
		return TK_ERROR;
	}

	// 在数据字符串中寻找键的位置
	if((pstr = strstr(data, key) )== NULL){
        log_error("No key [%s] found", key);
        return TK_ERROR;
    }

	// 键找到后，确定值的开始位置和结束位置
	p_start = pstr + strlen(key);
	p_start = strchr(p_start, ':');
	if(p_start == NULL){
		log_error("No value found for key [%s]", key);
		return TK_ERROR;
	}
	p_start++;
	p_end = strchr(p_start, '\n');
	if(p_end == NULL){
		p_end = strchr(p_start, '\0');
		if(p_end == NULL){
			log_error("Invalid data format");
			return TK_ERROR;
		}
	}
	value_len = p_end - p_start;

	// 检查值的长度是否超过最大限制
	if(value_type == type_array){
		if(value_len >= MAX_ELEMENTS_SIZE*MAX_ELEMENTS_NUM){
			log_error("The value of [%s] is too long", key);
			return TK_ERROR;
		}
	}else{
		if(value_len >= MAX_ELEMENTS_SIZE){
			log_error("The value of [%s] is too long", key);
			return TK_ERROR;
		}
	}

	// 复制值到临时缓冲区，并去除空白字符
	memcpy(tmp, p_start, value_len);
	tmp[value_len]= '\0';
	trim(tmp);

	value->type_t = value_type;
	value->elements_num = 1;
	value->elements_size = 0;

	// 根据指定的值类型，将字符串转换为相应的数据类型
	switch(value_type){
		case type_bool:
			if(strcmp(tmp, "true") == 0){
				value->boolean = true;
			}else if(strcmp(tmp, "false") == 0){
				value->boolean = false;
			}else{
				log_debug("tmp = %s", tmp);
				log_error("Invalid bool value of key [%s]", key);
				goto parse_error;
			}
			value->elements_num = 1;
			value->elements_size = sizeof(bool);
			break;
		case type_char:
			if(strlen(tmp)!=1){
				log_error("Invalid char value of key [%s]", key);
				goto parse_error;
			}
			value->elements_num = 1;
			value->elements_size = sizeof(char);
			value->ch = tmp[0];
			break;
		case type_float:
			if(sscanf(tmp, "%f", &(value->flt)) != 1){
				log_error("Invalid float value of key [%s]", key);
				goto parse_error;
			}
			value->elements_num = 1;
			value->elements_size = sizeof(float);
			break;
		case type_int:
			if(is_all_digits(tmp) != true){
				log_error("Invalid int value of key [%s]", key);
				goto parse_error;
			}
			if(sscanf(tmp, "%d", &(value->intger)) != 1){
				log_error("Invalid int value of key [%s]", key);
				goto parse_error;
			}
			value->elements_num = 1;
			value->elements_size = sizeof(int);
			break;
		case type_int64:
			if(is_all_digits(tmp) != true){
				log_error("Invalid int64 value of key [%s]", key);
				goto parse_error;
			}
			if(sscanf(tmp, "%ld", &(value->intger64)) != 1){
				log_error("Invalid int64 value of key [%s]", key);
				goto parse_error;
			}
			value->elements_num = 1;
			value->elements_size = sizeof(int64_t);
			break;
		case type_string:
			remove_quotation_marks(tmp);
			value->str = (char*)tk_malloc(value_len+1);
			if(value->str == NULL){
				log_error("Failed to tk_malloc: %s, key = %s", strerror(errno), key);
				goto parse_error;
			}
			memcpy(value->str, tmp, value_len+1);
			value->elements_num = 1;
			value->elements_size = value_len+1;
			break;
		case type_array:
			element_para_t ele;
			ele = get_elements_para(tmp, ',');
			if(ele.max_length >= MAX_ELEMENTS_NUM){
				log_error("The number of elements with key value [%s] exceeds the limit %d", key, MAX_ELEMENTS_NUM);
				goto parse_error;
			}

			value->array = (char**)calloc(ele.number, sizeof(char)*(ele.max_length+1));
			if(value->array == NULL){
				log_error("Failed to tk_malloc: %s, key = %s", strerror(errno), key);
				goto parse_error;
			}

			value->elements_num = ele.number;
			value->elements_size = ele.max_length+1;
			if(TK_ERROR == parse_array_to_string(tmp, value->array, value->elements_size,',')){
				log_error("Failed to parse array string, key = %s", key);
				goto parse_error;
			}
			break;
		default:
			log_error("Invalid int value type of key [%s]", key);
			goto parse_error;
	}

	return TK_OK;

parse_error:
	value->elements_num = 0;
	value->elements_size = 0;
	value->type_t = type_none;
	if(value->array != NULL) tk_free(value->array);
	if(value->str != NULL) tk_free(value->str);
	return TK_ERROR;
}

void cfg_context_comment_remove(char* cfg_context)
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
	char* result = (char*)tk_malloc(strlen(cfg_context)+1);
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
			log_error("cfg_context_comment_remove error, %ld", p - p_string_end);
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
	tk_free(result);
}

