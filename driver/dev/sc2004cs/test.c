#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sc2004c_ctrl.h"

#define LINE_LEN 	20

/* int main(int __attribute__((unused)) argc ,char __attribute__((unused)) *argv[]) */
int main(int argc , char *argv[])
{
    uint8_t i;

    if (argc < 2) {
        fprintf(stderr, "USAGE: %s MSG1 [MSG2] [MSG3] [MSG4]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sc2004c_gpio_assign_t assign;
    
    // NOTE: pin assign
    assign.rs = 21;
    assign.en = 22;
    assign.d4 = 23;
    assign.d5 = 24;
    assign.d6 = 25;
    assign.d7 = 26;

    sc2004c_init(&assign);

    for (i = 0; i+1 < argc; i++) {
        sc2004c_set_line(i);
        sc2004c_print(argv[i+1]);
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
