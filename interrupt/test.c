#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rp_irq.h"

#define TIMEOUT_MSEC    10000

int main(int argc,char *argv[])
{
    uint8_t pin_no;
    rp_irq_handle_t handle;

    if (argc != 2) {
        fprintf(stderr, "USAGE: %s  PIN_NO\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pin_no = atoi(argv[1]);

    rp_irq_enable(pin_no, RP_IRQ_EDGE_FALLING);
    rp_irq_init(pin_no, &handle);
    while (1) {
        rp_irq_stat_t stat = rp_irq_wait(&handle, TIMEOUT_MSEC);
        
        switch (stat) {
        case RP_IRQ_STAT_L:
            printf("state: 0\n"); break;
        case RP_IRQ_STAT_H:
            printf("state: 1\n"); break;
        case RP_IRQ_STAT_TIMEOUT:
            printf("timeout\n"); break;
        default:
        fprintf(stderr, "ERROR: invalid stat (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
        }
    }
    rp_irq_disable(pin_no);

    return EXIT_SUCCESS;
}

// Local Variables:
// coding: shift_jis-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
