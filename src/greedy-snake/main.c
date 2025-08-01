/**
 * 最简项目
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "../../lib/tui.h"
#include "../../lib/common.h"

extern void game_run(void);
extern void game_final(void);
extern void game_run_2(void);
extern void game_final_2(void);
#define app_run() game_run_2()
#define app_final() game_final_2()

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

void init(void)
{
    log_init();
    set_log_level(LOG_LEVEL_DEBUG);
    tk_signal_init();
    tui_init();
}

extern void game_final(void);
void final(void)
{
    app_final();
    tui_final();
    log_final();
}

extern void run(void);
int main(int argc, char **argv)
{
    init();

    atexit(final);

    app_run();

    return 0;
}