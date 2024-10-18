#include <memory.h>
#include <string.h>

#include "app-init.h"
#include "../lib/common.h"
#include "../lib/data_handle.h"
#include "struct.h"

#define MAX_ROLE_NUM 10
#define MAX_ROLE_DATA_SIZE 1024

role_t G_role[MAX_ROLE_NUM];
int G_role_num = 0;
char G_role_path[MAX_PATH_LEN] = "";

int app_init(void)
{
    read_role();
    return TK_OK;
}

int app_final(void)
{
    return TK_OK;
}

int set_role_path(const char* path)
{
    if(path == NULL || strlen(path) == 0){
        log_error("role path is NULL");
        return TK_ERROR;
    }
    log_info("role path:%s", path);
    memcpy(G_role_path, path, strlen(path));
    return TK_OK;
}

int read_role(void)
{
    const char* p = NULL;
    char* role_str = file_to_string(G_role_path);
    if(role_str == NULL) {
        log_error("could not open file to read role file");
        goto read_error;
    }
    cfg_context_comment_remove(role_str);

    //分割不同角色的数据
    char role_context[MAX_ROLE_DATA_SIZE];
    p = role_str;
    while(1){
        memset(role_context, 0, MAX_ROLE_DATA_SIZE);
        p = parse_cfg_label(p, "@role", role_context);
        if(p == NULL){
            break;
        }

        log_text("role_context:\n%s\n", role_context);
        G_role_num++;
    }

    tk_print("role_num:%d", G_role_num);

    return TK_OK;

read_error:
    tk_free(role_str);
    TK_ABORT();
    return TK_ERROR;
}
