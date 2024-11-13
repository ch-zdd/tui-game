#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>

#include "../lib/common.h"
#include "app-context.h"
#include "app-init.h"
#include "app.h"

#define DEFAULT_CFG_PATH "cfg/app.cfg"

int init(void)
{
    char* cfg_path = get_app_context()->cfg_path;

    if(strlen(cfg_path) == 0){
        tk_print("No config file path");
        return TK_ERROR;
    }

    if(TK_OK != load_app_cfg(cfg_path)){
        return TK_ERROR;
    }

    if(TK_OK != log_init()){
        return TK_ERROR;
    }

    log_text("\tMY THREE KINGDOM!\n");
    log_info("cfg path:%s", get_app_context()->cfg_path);

    if(TK_OK != app_init()){
        return TK_ERROR;
    }

    return TK_OK;
}

void tk_signal_handle(int signo)
{
    switch(signo){
        case SIGINT:
            log_info("Received signal SIGINT, exiting...");
            exit(0);
            break;
        case SIGTERM:
            log_info("Received signal SIGTERM, exiting...");
            exit(0);
            break;
        case SIGWINCH:
            log_info("Received signal SIGWINCH, exiting...");
            exit(0);
            //todo:界面重绘
            break;
        default:
            log_info("Received signal %d, exiting...", signo);
            return;
    }
}

void tk_signal_init(void)
{
    signal(SIGINT, tk_signal_handle);
    signal(SIGTERM, tk_signal_handle);
    signal(SIGWINCH, tk_signal_handle);
}

void run(void)
{
    log_info("Starting...");
    app_run();
}

void final(void)
{
    app_final();
    log_info("Bye!");
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
    tk_signal_init();
    atexit(final);

    set_cfg_path(DEFAULT_CFG_PATH);
    parse_params(argc, argv);

    if(TK_OK != init()){
        log_error("Init failed");
        return -1;
    }

    run();

    return 0;
}