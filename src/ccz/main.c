#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>

#include "../../lib/common.h"
#include "app-context.h"
#include "app-init.h"
#include "app.h"

#define DEFAULT_CFG_PATH "cfg/app.cfg"

static void tk_signal_init(void);
int init(void)
{
    if(TG_OK != log_init()){
        return TG_ERROR;
    }

    tk_signal_init();

    if(TG_OK != app_init()){
        return TG_ERROR;
    }

    return TG_OK;
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

static void tk_signal_init(void)
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
    tg_print("Usage: %s [OPTION] [argument]", app_name);
    tg_print("Options:");
    tg_print("  -h, --help");
    tg_print("  -c, --config [path]   Program configuration file path");
}

int parse_params(int argc, char **argv)
{
    if(argc < 2){
        //命令行没有配置参数，使用默认配置文件
        set_cfg_path(DEFAULT_CFG_PATH);
        return TG_OK;
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
                exit(EXIT_SUCCESS);
            case 'c':
                if(TG_OK != set_cfg_path(optarg)){
                    tg_print("Invalid config path");
                    exit(EXIT_FAILURE);
                }
                break;
            case '?':
                return TG_ERROR;
        }
    }

    return TG_OK;
}

int main(int argc, char **argv)
{
    parse_params(argc, argv);

    atexit(final);

    if(TG_OK != init()){
        tg_print("Init failed");
        return -1;
    }

    run();

    return 0;
}