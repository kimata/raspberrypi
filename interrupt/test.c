#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

#include "rp_irq.h"

#define TIMEOUT_MSEC    10000

static uint8_t state_change = 0;

static void sigusr1_handler(int sig)
{
    if (sig != SIGUSR1) {
        return;
    }
    stat_change = 1;
    signal(SIGUSR1, sigusr1_handler);
}

static int watch_state(uint8_t pin_no, pid_t parent)
{
    rp_irq_handle_t handle;

    rp_irq_init(pin_no, &handle);    
    while (1) {
        if (rp_irq_wait(&handle, TIMEOUT_MSEC) != RP_IRQ_STAT_TIMEOUT) {
            if (kill(parent, SIGUSR1) != 0) {
                fprintf(stderr, "ERROR: kill (at %s:%d)\n", __FILE__, __LINE__);
                exit(EXIT_FAILURE);
            }
        }

    }

    return EXIT_SUCCESS;
}

static int listen_change()
{
    signal(SIGUSR1, sigusr1_handler);    

    while (1) {
        sleep(1);
        if (state_change == 1) {
            
            state_change = 0;
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

    rp_irq_enable(pin_no, RP_IRQ_EDGE_FALLING);

    pid = fork();
    switch(pid) {
    case -1:
        fprintf(stderr, "ERROR: fork (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    case 0:
        return watch_state(pin_no, getppid());
    default:
        return listen_change();
    }
    rp_irq_disable(pin_no);

    fprintf(stderr, "EXIT (at %s:%d)\n", __FILE__, __LINE__);

    return EXIT_SUCCESS;
}

// Local Variables:
// coding: shift_jis-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:

