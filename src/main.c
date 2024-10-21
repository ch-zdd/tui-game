#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "../lib/common.h"
#include "app-context.h"
#include "app-init.h"

#define DEFAULT_CFG_PATH "cfg/config.cfg"

void init(void)
{
    char* cfg_path = get_app_context()->cfg_path;

    if(strlen(cfg_path) == 0){
        tk_print("No config file path");
        TK_ABORT();
    }
    load_cfg(cfg_path);

    log_init();
    log_text("\tMY THREE KINGDOM!\n");
    log_info("cfg path:%s", get_app_context()->cfg_path);
    app_init();
}

void final(void)
{
    app_final();
    log_final();
}

void print_help(const char* app_name)
{
    printf("Usage: %s [OPTION] argument\n", app_name);
    printf("Options:\n");
    printf("  -h, --help\n");
    printf("  -c, --config [path]   Program configuration file path\n");
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
                set_cfg_path(optarg);
                break;
            case '?':
                return TK_ERROR;
        }
    }

    return TK_OK;
}

int main(int argc, char **argv)
{
    set_cfg_path(DEFAULT_CFG_PATH);
    parse_params(argc, argv);
    init();

    final();
    return 0;
}