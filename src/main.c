#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "../lib/data_handle.h"
#include "../lib/common.h"
#include "struct.h"
#include "app-init.h"

#define DEFAULT_CFG_PATH "cfg/config.cfg"

static char G_cfg_path[MAX_PATH_LEN] = {};

void read_program_cfg(const char* cfg_path);

void set_default_path(const char* path)
{
    if(path == NULL || strlen(path) == 0){
        tk_print("No config file path");
        return;
    }
    strncpy(G_cfg_path, path, MAX_PATH_LEN);
}

void init(void)
{
    if(strlen(G_cfg_path) == 0){
        tk_print("No config file path");
        exit(1);
    }
    read_program_cfg(G_cfg_path);

    log_init();
    app_init();
}

void print_help(const char* app_name)
{
    printf("Usage: %s [OPTION] argument\n", app_name);
    printf("Options:\n");
    printf("  -h, --help\n");
    printf("  -c, --config [path]   Program configuration file path\n");
}

void read_program_cfg(const char* cfg_path)
{
    char* cfg_str = file_to_string(cfg_path);
    if(cfg_str == NULL) {
        log_error("Failed to read config file");
        return;
    }
    cfg_context_comment_remove(cfg_str);

    data_t log_file_path = {};
    if(parse_key_value(cfg_str, "log_file_path", &log_file_path, type_string) == TK_OK) {
        set_log_file(log_file_path.str);
    }

    data_t log_level = {};
    if(parse_key_value(cfg_str, "log_level", &log_level, type_string) == TK_OK) {
        log_levle_t level = log_string_to_level(log_level.str);
        if(level > LOG_LEVEL_ALL){
            log_error("Illegal log level");
        }else{
            set_log_level(level);
        }
    }

    data_t role_path = {};
    if(parse_key_value(cfg_str, "role_path", &role_path, type_string) == TK_OK) {
        set_role_path(role_path.str);
    }
    
    tk_free(cfg_str);
    tk_free(log_file_path.str);
    tk_free(log_level.str);
    tk_free(role_path.str);
}

int parse_params(int argc, char **argv)
{
    if(argc < 2){
        return TK_OK;
    }

    struct option opts[] = {
        {"help",     no_argument,       NULL, 'h'},
        {"config",   required_argument, NULL, 'c'},
        {0, 0, 0, 0}
    };

    int opt;
    while((opt = getopt_long(argc, argv, "hc:", opts, NULL)) != -1){
        switch(opt) {
            case 'h':
                print_help(argv[0]);
                exit(0);
            case 'c':
                tk_print("cfg path: %s", optarg);
                set_default_path(optarg);
                break;
            case '?':
                return TK_ERROR;
        }
    }
}

int main(int argc, char **argv)
{
    set_default_path(DEFAULT_CFG_PATH);
    parse_params(argc, argv);
    init();

    return 0;
}