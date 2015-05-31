#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "adt7410_ctrl.h"

void sense(uint8_t dev_addr)
{
    float v = adt7410_sense(dev_addr);
    printf("temp: %.2f\n", v);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char *argv[])
{
    uint8_t dev_addr;
    char *err_char;

    if (argc != 2) {
        fprintf(stderr, "USAGE: %s  DEV_ADDR\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    dev_addr = strtol(argv[1], &err_char, 0);

    adt7410_init(dev_addr);

    while (1) {
        sense(dev_addr);
        sleep(1);
    }

    return EXIT_SUCCESS;
}
#pragma GCC diagnostic pop

// Local Variables:
// coding: shift_jis-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End: