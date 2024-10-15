#include "data_handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

char* read_file_to_str(const char* file_name)
{
    long file_len = 0;
    char* data_str = NULL;

    if(file_name == NULL){
        fm_error("Cannot input NULL ptr\n");
        return NULL;
    }

    FILE* file = fopen(file_name, "r");
    if(file == NULL){
        perror("Data file ");
        return NULL;
    }

    if(0 != fseek(file,0,SEEK_END)){
        perror("fseek data file:");
        goto READ_ERROR;
    }

    file_len = ftell(file);
    if(file_len < 0){
        perror("ftell data file:");
        goto READ_ERROR;
    }

    if(file_len == 0){
        fm_error("The file of resource is empty!\n");
        goto READ_ERROR;
    }

    rewind(file);
    data_str = (char*)malloc(file_len+1);
    if(data_str == NULL){
        perror("malloc");
        goto READ_ERROR;
    }
    if(file_len != fread(data_str, 1, file_len, file)){
        fm_error("Failed to read data file\n");
        goto READ_ERROR;
    }

    fclose(file);
    data_str[file_len] = '\0';

    return data_str;

READ_ERROR:
    if(file != NULL) fclose(file);
    if(data_str != NULL) free(data_str);
    return NULL;
}

cd_t convert_complex_float_to_double(cf_t cf) 
{  
    double real_part = crealf(cf);  // 获取复数的实部  
    double imag_part = cimagf(cf);  // 获取复数的虚部  
    return real_part + imag_part * _Complex_I;  // 构造新的 double _Complex 类型  
}

cf_t convert_complex_double_to_float(cf_t cf)
{  
    float real_part = (float)creal(cf);  // 获取复数的实部  
    float imag_part = (float)cimag(cf);  // 获取复数的虚部  
    return real_part + imag_part * _Complex_I;  // 构造新的 double _Complex 类型  
}

void print_int8(FILE* stream, int8_t* data, size_t len, int n_spaces)
{
    int i;
    char format[100] = {};

    if(len == 0){
        fm_error("Parameter len is 0 \n");
        return;
    }

    if(data == NULL){
        fm_error("Cannot input NULL ptr\n");
        return;
    }

    if(n_spaces >= 100){
        fm_error("Parameter n_spaces = %d is out of range\n", n_spaces);
        return;
    }

    FILL_HEAD_SPACES(format, n_spaces);

    fprintf(stream, "%s(", format);
    for(i = 0; i<(len -1); i++){
        fprintf(stream, "%2hhd,", data[i]);
        if((i+1)%32 == 0){
            fprintf(stream, "\n %s", format);
        }
    }
    fprintf(stream, "%2hhd)\n", data[i]);
}

void print_bit_uint8(FILE* stream, uint8_t* data, size_t len, int n_spaces)
{
    int i;
    char format[100] = {};

    if(len == 0){
        fm_error("Parameter len is 0 \n");
        return;
    }

    if(data == NULL){
        fm_error("Cannot input NULL ptr\n");
        return;
    }

    if(n_spaces >= 100){
        fm_error("Parameter n_spaces = %d is out of range\n", n_spaces);
        return;
    }

    FILL_HEAD_SPACES(format, n_spaces);

    fprintf(stream, "%s(", format);
    for(i = 0; i<(len -1); i++){
        if(data[i] == FM_FILLER_BIT){
            fprintf(stream, "-");
        }else{
            fprintf(stream, "%hhu", data[i]);
        }
        if((i+1)%32 == 0){
            fprintf(stream, "\n %s", format);
        }
    }

    if(data[i] == FM_FILLER_BIT){
        fprintf(stream, "-)\n");
    }else{
        fprintf(stream, "%hhu)\n", data[i]);
    }
}

void print_uint32(FILE* stream, uint32_t* data, size_t len, int n_spaces)
{
    int i;
    char format[100] = {};

    if(len == 0){
        fm_error("Parameter len is 0 \n");
        return;
    }

    if(data == NULL){
        fm_error("Cannot input NULL ptr\n");
        return;
    }

    if(n_spaces >= 100){
        fm_error("Parameter n_spaces = %d is out of range\n", n_spaces);
        return;
    }

    FILL_HEAD_SPACES(format, n_spaces);

    fprintf(stream, "%s(", format);
    for(i = 0; i<(len -1); i++){
        fprintf(stream, "%10u, ", data[i]);
        if((i+1)%8 == 0){
            fprintf(stream, "\n %s", format);
        }
    }
    fprintf(stream, "%10u)\n", data[i]);
}

void print_float(FILE* stream, float* data, size_t len, int n_spaces)
{
    int i;
    char format[100] = {};

    if(len == 0){
        fm_error("Parameter len is 0 \n");
        return;
    }

    if(data == NULL){
        fm_error("Cannot input NULL ptr\n");
        return;
    }

    if(n_spaces >= 100){
        fm_error("Parameter n_spaces = %d is out of range\n", n_spaces);
        return;
    }

    FILL_HEAD_SPACES(format, n_spaces);

    fprintf(stream, "%s(", format);
    for(i = 0; i<(len -1); i++){
        fprintf(stream, "%11.6f,", data[i]);
        if((i+1)%8 == 0){
            fprintf(stream, "\n %s", format);
        }
    }
    fprintf(stream, "%11.6f)\n", data[i]);
}

void print_double(FILE* stream, double* data, size_t len, int n_spaces)
{
    int i;
    char format[100] = {};

    if(len == 0){
        fm_error("Parameter len is 0 \n");
        return;
    }

    if(data == NULL){
        fm_error("Cannot input NULL ptr\n");
        return;
    }

    if(n_spaces >= 100){
        fm_error("Parameter n_spaces = %d is out of range\n", n_spaces);
        return;
    }

    FILL_HEAD_SPACES(format, n_spaces);

    fprintf(stream, "%s(", format);
    for(i = 0; i<(len -1); i++){
        fprintf(stream, "%11.8lf,", data[i]);
        if((i+1)%8 == 0){
            fprintf(stream, "\n %s", format);
        }
    }
    fprintf(stream, "%11.8lf)\n", data[i]);
}

void print_hex(FILE* stream, uint8_t* data, size_t len, int n_spaces)
{
    int i;
    char format[100] = {};

    if(len == 0){
        fm_error("Parameter len is 0 \n");
        return;
    }

    if(data == NULL){
        fm_error("Cannot input NULL ptr\n");
        return;
    }

    if(n_spaces >= 100){
        fm_error("Parameter n_spaces = %d is out of range\n", n_spaces);
        return;
    }

    FILL_HEAD_SPACES(format, n_spaces);

    fprintf(stream, "%s(", format);
    for(i = 0; i<(len -1); i++){
        fprintf(stream, "%02x, ", data[i]);
        if((i+1)%16 == 0){
            fprintf(stream, "\n %s", format);
        }
    }
    fprintf(stream, "%02x)\n", data[i]);
}

void print_complex(FILE* stream, cf_t* data, size_t len, int n_spaces)
{
    int i;
    char format[100] = {};

    if(len == 0){
        fm_error("Parameter len is 0 \n");
        return;
    }

    if(data == NULL){
        fm_error("Cannot input NULL ptr\n");
        return;
    }

    if(n_spaces >= 100){
        fm_error("Parameter n_spaces = %d is out of range\n", n_spaces);
        return;
    }

    FILL_HEAD_SPACES(format, n_spaces);

    fprintf(stream, "%s(", format);
    for(i = 0; i< len-1; i++){
        fprintf(stream, "%+8.4f%+8.4fi, ", crealf(data[i]), cimagf(data[i]));
        if((i+1)%4 == 0){
            fprintf(stream, "\n %s", format);
        }
    }
    fprintf(stream, "%+8.4f%+8.4fi)\n", crealf(data[i]), cimagf(data[i]));   
}

void print_complex_double(FILE* stream, cd_t* data, size_t len, int n_spaces)
{
    int i;
    char format[100] = {};

    if(len == 0){
        fm_error("Parameter len is 0 \n");
        return;
    }

    if(data == NULL){
        fm_error("Cannot input NULL ptr\n");
        return;
    }

    if(n_spaces >= 100){
        fm_error("Parameter n_spaces = %d is out of range\n", n_spaces);
        return;
    }

    FILL_HEAD_SPACES(format, n_spaces);

    fprintf(stream, "%s(", format);
    for(i = 0; i< len-1; i++){
        fprintf(stream, "%+10.8lf%+10.8lfi, ", creal(data[i]), cimag(data[i]));
        if((i+1)%4 == 0){
            fprintf(stream, "\n %s", format);
        }
    }
    fprintf(stream, "%+10.8lf%+10.8lfi)\n", creal(data[i]), cimag(data[i]));   
}

void print_llr(FILE* stream, int8_t* data, size_t len, int n_spaces)
{
    int i = 0;
    char format[100] = {};

    if(len == 0){
        fm_error("Parameter len is 0 \n");
        return;
    }

    if(data == NULL){
        fm_error("Cannot input NULL ptr\n");
        return;
    }

    if(n_spaces >= 100){
        fm_error("Parameter n_spaces = %d is out of range\n", n_spaces);
        return;
    }

    FILL_HEAD_SPACES(format, n_spaces);

    fprintf(stream, "%s(", format);
    for(i = 0; i<(len -1); i++){
        fprintf(stream, "%d, ", data[i]>=0 ? 1:0);
        if((i+1)%16 == 0){
            fprintf(stream, "\n %s", format);
        }
    }
    fprintf(stream, "%d)\n", data[i]>=0 ? 1:0);
}

void gen_bit_data(uint8_t* result, int result_num)
{
    if(result == NULL){
        fm_error("Cannot input NULL ptr\n");
        return;
    }

    for(int i=0; i<result_num; i++){
        result[i] = rand()%2;
    }
}

void gen_complex_data(cf_t* result, int result_num)
{
    if(result == NULL){
        fm_error("Cannot input NULL ptr\n");
        return;
    }

    for(int i=0; i<result_num; i++){
        result[i] = (rand()%100-50)/25.0 + (rand()%100-50)/25.0*1i;
    }
}

void gen_complex_double_data(cd_t* result, int result_num)
{
    if(result == NULL){
        fm_error("Cannot input NULL ptr\n");
        return;
    }

    for(int i=0; i<result_num; i++){
        result[i] = (rand()%100-50)/25.0f + (rand()%100-50)/25.0f*1i;
    }
}

int parse_bit_data(const char* source, int source_is_file, uint8_t* result, int result_num)
{
    uint32_t i = 0, j = 0;
    char* data_str = NULL;
    size_t str_len = 0;
    char* p1=NULL, *p2=NULL;

    if(result_num <= 0){
        fm_error("Cannot read 0 data\n");
        return FM_ERROR;
    }

    if(result == NULL){
        fm_error("Cannot input NULL ptr\n");
        return FM_ERROR;
    }

    if(source_is_file == FM_TRUE){
        data_str = read_file_to_str(source);
        if(data_str == NULL){
            return FM_ERROR;
        }
        str_len = strlen(data_str);
    }else{
        str_len = strlen(source);
        data_str = (char*)malloc(str_len+1);
        if(data_str == NULL){
            perror("malloc error");
            return FM_ERROR;
        }
        memcpy(data_str, source, str_len);
        data_str[str_len] = '\0';
    }

    if(str_len == 0){
        goto PARSE_ERR;
    }

    p1 = strchr(data_str, '[');
    p2 = strchr(data_str, ']');
    if(p1 == NULL || p2 == NULL || p2<p1){
        fm_error("The correct data format needs to be entered, for example [1, 0, ...]\n");
        goto PARSE_ERR;
    }

    for(j = p1-data_str; j<str_len && i<result_num ; j++){
        if(data_str[j] == '0' || data_str[j] =='1'){
            result[i] = data_str[j]-'0';
            i++;
            continue;
        }else if(data_str[j] == '-'){
            result[i] = FM_FILLER_BIT;
            i++;
            continue;
        }else if(data_str[j] == ',' || data_str[j] == ' '){
            continue;
        }else if(data_str[j] == ']'){
            break;
        }else{
            continue;
        }
    }

    if(i == 0 || i< result_num){
        goto PARSE_ERR;
    }

    if(data_str != NULL) free(data_str);
    return FM_OK;

PARSE_ERR:
    if(data_str != NULL) free(data_str);
    return FM_ERROR;
}

/*解析形如[ a1+b1*i, a2-b2*i , 3*i]和 [(a1, b1)(a2, b2)(0, c)] 的复数数据，格式不能混合使用*/
int parse_complex_data(const char* source, int source_is_file, cf_t* result, int result_num)
{
    size_t str_len = 0;
    uint32_t i = 0;
    char* data_str = NULL;
    char* token = NULL;
    char* real_str = NULL;
    char* imag_str = NULL;
    char token_str[128] = {0};
    size_t token_str_len = 0;
    float complex_real = 0;
    float complex_imag = 0;
    float temp_float = 0;
    char* p1=NULL, *p2=NULL;

    if(result_num <=0){
        fm_error("Cannot read 0 data\n");
        return FM_ERROR;
    }

    if(result == NULL){
        fm_error("Cannot input NULL ptr\n");
        return FM_ERROR;
    }

    if(source_is_file == FM_TRUE){
        data_str = read_file_to_str(source);
        if(data_str == NULL){
            return FM_ERROR;
        }
        str_len = strlen(data_str);
    }else{
        str_len = strlen(source);
        data_str = (char*)malloc(str_len+1);
        if(data_str == NULL){
            perror("malloc error");
            return FM_ERROR;
        }
        memcpy(data_str, source, str_len);
        data_str[str_len] = '\0';
    }

    if(str_len == 0){
        goto PARSE_ERR;
    }

    p1 = strchr(data_str, '[');
    p2 = strchr(data_str, ']');
    if(p1 == NULL || p2 == NULL || p2<p1){
        fm_error("The correct data format needs to be entered, for example [-1-i, 0.5i, ...]\n");
        goto PARSE_ERR;
    }

    i = 0;
    token = strtok(p1+1, ",");
    while( token != NULL){
        token_str_len = strlen(token); 
        if(token_str_len>=128){
            fm_error("The length of token string is too long!\n");
            goto PARSE_ERR;
        }
        memset(token_str, 0, 128);
        memcpy(token_str, token, token_str_len);

        temp_float = strtof(token_str, &real_str);
        if (errno == ERANGE) {  
            fm_error("Overflow occurred: value too large or too small, str = %s\n", data_str); 
            goto PARSE_ERR;
        }

        //解析实部
        if(real_str[0] != 'i'){
            complex_real = temp_float;
        }else{
            complex_real = 0;
            //纯虚数，需要将real_str指针指向token开头
            real_str = token_str;
        }

        //解析虚部
        temp_float = strtof(real_str, &imag_str);
        if (errno == ERANGE) {  
            fm_error("Overflow occurred: value too large or too small, str = %s\n", data_str); 
            goto PARSE_ERR;
        }
        
        if(temp_float == 0){
            if(strstr(imag_str, "-i") != NULL){
                //case: -i
                complex_imag = -1;
            }else if(strstr(imag_str, "+i") != NULL){
                //case: +i
                complex_imag = 1;
            }else if(strstr(imag_str, "i") != NULL){
                if(real_str == imag_str){
                    //case: i
                    complex_imag = 1;
                }else{
                    //case: 0i, 0.00i, +0.000i, -0.00i
                    complex_imag = 0;
                }
            }else{
                //case: 无虚部
                complex_imag = 0;
            }
        }else{
            //case: xi
            complex_imag = temp_float;
        }

        result[i] = complex_real + complex_imag*1I;

        token = strtok(NULL, ",");
        i++;
        if(i == result_num){
            break;
        }
    }

    if(i < result_num){
        fm_error("Insufficient data length(= %u), need %d\n", i, result_num); 
        goto PARSE_ERR;
    }

    if(data_str != NULL) free(data_str);
    return FM_OK;

PARSE_ERR:
    if(data_str != NULL) free(data_str);
    return FM_ERROR;
}

/*解析形如[ a1+b1*i, a2-b2*i , 3*i]和 [(a1, b1)(a2, b2)(0, c)] 的复数数据，格式不能混合使用*/
int parse_complex_double_data(const char* source, int source_is_file, cd_t* result, int result_num)
{
    size_t str_len = 0;
    uint32_t i = 0;
    char* data_str = NULL;
    char* token = NULL;
    char* real_str = NULL;
    char* imag_str = NULL;
    char token_str[128] = {0};
    size_t token_str_len = 0;
    double complex_real = 0;
    double complex_imag = 0;
    double temp_double = 0;
    char* p1=NULL, *p2=NULL;

    if(result_num <=0){
        fm_error("Cannot read 0 data\n");
        return FM_ERROR;
    }

    if(result == NULL){
        fm_error("Cannot input NULL ptr\n");
        return FM_ERROR;
    }

    if(source_is_file == FM_TRUE){
        data_str = read_file_to_str(source);
        if(data_str == NULL){
            return FM_ERROR;
        }
        str_len = strlen(data_str);
    }else{
        str_len = strlen(source);
        data_str = (char*)malloc(str_len+1);
        if(data_str == NULL){
            perror("malloc error");
            return FM_ERROR;
        }
        memcpy(data_str, source, str_len);
        data_str[str_len] = '\0';
    }

    if(str_len == 0){
        goto PARSE_ERR;
    }

    p1 = strchr(data_str, '[');
    p2 = strchr(data_str, ']');
    if(p1 == NULL || p2 == NULL || p2<p1){
        fm_error("Data needs to be packaged by []\n");
        goto PARSE_ERR;
    }

    if(strchr(data_str, '(') != NULL && strchr(data_str, ')') != NULL){
        //解析坐标形式的复数数据
        int ret = 0;
        char token[100] = "";
        char* current_pointer = data_str;
        int token_len = 0;
        while(1){
            p1 = strchr(current_pointer, '(');
            p2 = strchr(current_pointer, ')');
            
            current_pointer = p2+1;
            if(current_pointer >= data_str+str_len || (p1 == NULL && p2 == NULL)){
                break;
            }
            if(p1[0] !='('  || p2[0] !=')' ){
                fm_error("Error data format, symbol '(' ')'does not match.\n");
                goto PARSE_ERR;
            }

            token_len = p2-p1+1;
            if(token_len > 100){
                fm_error("token too long.\n");
                goto PARSE_ERR;
            }
            memcpy(token, p1, token_len);    
            ret = sscanf(token, "%*[^.0-9]%lf%*[^.0-9]%lf]", &complex_real, &complex_imag);
            if(ret !=2 ){
                fm_error("Error data format, for example:[(1,2) (1.33, -3.44)]\n");
                goto PARSE_ERR;
            }

            result[i] = complex_real + complex_imag*1I;
            i++;
            if(i == result_num){
                break;
            }
        }

    }else{
        i = 0;
        token = strtok(p1+1, ",");
        while( token != NULL){
            token_str_len = strlen(token); 
            if(token_str_len>=128){
                fm_error("The length of token string is too long!\n");
                goto PARSE_ERR;
            }
            memset(token_str, 0, 128);
            memcpy(token_str, token, token_str_len);

            temp_double = strtod(token_str, &real_str);
            if (errno == ERANGE) {  
                fm_error("Overflow occurred: value too large or too small, str = %s\n", data_str); 
                goto PARSE_ERR;
            }

            //解析实部
            if(real_str[0] != 'i'){
                complex_real = temp_double;
            }else{
                complex_real = 0;
                //纯虚数，需要将real_str指针指向token开头
                real_str = token_str;
            }

            //解析虚部
            temp_double = strtod(real_str, &imag_str);
            if (errno == ERANGE) {  
                fm_error("Overflow occurred: value too large or too small, str = %s\n", data_str); 
                goto PARSE_ERR;
            }
            
            if(temp_double == 0){
                if(strstr(imag_str, "-i") != NULL){
                    //case: -i
                    complex_imag = -1;
                }else if(strstr(imag_str, "+i") != NULL){
                    //case: +i
                    complex_imag = 1;
                }else if(strstr(imag_str, "i") != NULL){
                    if(real_str == imag_str){
                        //case: i
                        complex_imag = 1;
                    }else{
                        //case: 0i, 0.00i, +0.000i, -0.00i
                        complex_imag = 0;
                    }
                }else{
                    //case: 无虚部
                    complex_imag = 0;
                }
            }else{
                //case: xi
                complex_imag = temp_double;
            }

            result[i] = complex_real + complex_imag*1I;

            token = strtok(NULL, ",");
            i++;
            if(i == result_num){
                break;
            }
        }
    }

    if(i < result_num){
        fm_error("Insufficient data length(= %u), need %d\n", i, result_num); 
        goto PARSE_ERR;
    }

    if(data_str != NULL) free(data_str);
    return FM_OK;

PARSE_ERR:
    if(data_str != NULL) free(data_str);
    return FM_ERROR;
}

int parse_float_data(const char* source, int source_is_file, float* result, int result_num)
{
    size_t str_len = 0;
    uint32_t i = 0;
    char* data_str = NULL;
    char* token = NULL;
    char token_str[128] = {0};
    size_t token_str_len = 0;
    float temp_float = 0;
    char* p1=NULL, *p2=NULL;

    if(result_num <= 0){
        fm_error("Cannot read 0 data\n");
        return FM_ERROR;
    }

    if(result == NULL){
        fm_error("Cannot input NULL ptr\n");
        return FM_ERROR;
    }

    if(source_is_file == FM_TRUE){
        data_str = read_file_to_str(source);
        if(data_str == NULL){
            return FM_ERROR;
        }
        str_len = strlen(data_str);
    }else{
        str_len = strlen(source);
        data_str = (char*)malloc(str_len+1);
        if(data_str == NULL){
            perror("malloc error");
            return FM_ERROR;
        }
        memcpy(data_str, source, str_len);
        data_str[str_len] = '\0';
    }

    if(str_len == 0){
        goto PARSE_ERR;
    }

    p1 = strchr(data_str, '[');
    p2 = strchr(data_str, ']');
    if(p1 == NULL || p2 == NULL || p2<p1){
        fm_error("The correct data format needs to be entered, for example [1, 0.5, ...]  %s\n", data_str);
        goto PARSE_ERR;
    }

    i = 0;
    token = strtok(p1+1, ",");
    while( token != NULL){
        token_str_len = strlen(token); 
        if(token_str_len>=128){
            fm_error("The length of token string is too long!\n");
            goto PARSE_ERR;
        }
        memset(token_str, 0, 128);
        memcpy(token_str, token, token_str_len);

        temp_float = strtof(token_str, NULL);
        if (errno == ERANGE) {
            fm_error("Overflow occurred: value too large or too small, str = %s\n", token_str); 
            goto PARSE_ERR;
        }
    
        result[i] = temp_float;
        token = strtok(NULL, ",");
        i++;
        if(i == result_num){
            break;
        }
    }

    if(i < result_num){
        fm_error("Insufficient data length(= %u), need %d\n", i, result_num); 
        goto PARSE_ERR;
    }

    if(data_str != NULL) free(data_str);

    return FM_OK;

PARSE_ERR:
    if(data_str != NULL) free(data_str);
    return FM_ERROR;
}

int parse_double_data(const char* source, int source_is_file, double* result, int result_num)
{
    size_t str_len = 0;
    uint32_t i = 0;
    char* data_str = NULL;
    char* token = NULL;
    char token_str[128] = {0};
    size_t token_str_len = 0;
    double temp_double = 0;
    char* p1=NULL, *p2=NULL;

    if(result_num <= 0){
        fm_error("Cannot read 0 data\n");
        return FM_ERROR;
    }

    if(result == NULL){
        fm_error("Cannot input NULL ptr\n");
        return FM_ERROR;
    }

    if(source_is_file == FM_TRUE){
        data_str = read_file_to_str(source);
        if(data_str == NULL){
            return FM_ERROR;
        }
        str_len = strlen(data_str);
    }else{
        str_len = strlen(source);
        data_str = (char*)malloc(str_len+1);
        if(data_str == NULL){
            perror("malloc error");
            return FM_ERROR;
        }
        memcpy(data_str, source, str_len);
        data_str[str_len] = '\0';
    }

    if(str_len == 0){
        goto PARSE_ERR;
    }

    p1 = strchr(data_str, '[');
    p2 = strchr(data_str, ']');
    if(p1 == NULL || p2 == NULL || p2<p1){
        fm_error("The correct data format needs to be entered, for example [1, 0.5, ...]  %s\n", data_str);
        goto PARSE_ERR;
    }

    i = 0;
    token = strtok(p1+1, ",");
    while( token != NULL){
        token_str_len = strlen(token); 
        if(token_str_len>=128){
            fm_error("The length of token string is too long!\n");
            goto PARSE_ERR;
        }
        memset(token_str, 0, 128);
        memcpy(token_str, token, token_str_len);

        temp_double = strtod(token_str, NULL);
        if (errno == ERANGE) {
            fm_error("Overflow occurred: value too large or too small, str = %s\n", token_str); 
            goto PARSE_ERR;
        }
    
        result[i] = temp_double;
        token = strtok(NULL, ",");
        i++;
        if(i == result_num){
            break;
        }
    }

    if(i < result_num){
        fm_error("Insufficient data length(= %u), need %d\n", i, result_num); 
        goto PARSE_ERR;
    }

    if(data_str != NULL) free(data_str);

    return FM_OK;

PARSE_ERR:
    if(data_str != NULL) free(data_str);
    return FM_ERROR;
}

uint32_t bit_diff(const uint8_t* x, const uint8_t* y, int nbits)
{
  uint32_t errors = 0;
  for (int i = 0; i < nbits; i++) {
    if (x[i] != y[i]) {
      errors++;
    }
  }
  return errors;
}

#ifdef __cplusplus
}
#endif