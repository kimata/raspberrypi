#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#include "rp_irq.h"

static uint8_t g_stat_change 	= 0;

static uint8_t no = 0;

static void sigusr1_handler(int sig)
{
    if (sig != SIGUSR1) {
        return;
    }
    g_stat_change = 1;
    printf("change! %d\n", no++);
    signal(SIGUSR1, sigusr1_handler);
}


static int listen_change()
{
    signal(SIGUSR1, sigusr1_handler);    

    while (1) {
        sleep(1);
        if (g_stat_change == 1) {
            // DO SOMETHING
            g_stat_change = 0;
        }
    }

    return EXIT_SUCCESS;
}

int main(int argc,char *argv[])
{
    uint8_t pin_no;

    int pid;

    if (argc != 2) {
        fprintf(stderr, "USAGE: %s  PIN_NO\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pin_no = atoi(argv[1]);

    rp_irq_enable(pin_no, RP_IRQ_EDGE_BOTH);

    pid = fork();
    switch(pid) {
    case -1:
        fprintf(stderr, "ERROR: fork (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    case 0:
        rp_irq_watch_stat(pin_no, getppid());
    default:
        listen_change();
        rp_irq_disable(pin_no);
    }

    return EXIT_SUCCESS;
}

// Local Variables:
// coding: shift_jis-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:

