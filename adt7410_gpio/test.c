#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adt7410_ctrl.h"

void test(uint8_t dev_addr)
{
    float v = adt7410_sense(dev_addr);
    printf("temp: %.2f\n", v);
}

int main(int argc, char *argv[])
{
    adt7410_init(0x48);

    test(0x48);

    return EXIT_SUCCESS;
}

// Local Variables:
// coding: shift_jis-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
