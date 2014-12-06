#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rp_gpio.h"
#include "sc2004c_ctrl.h"

#define LINE_LEN 	20

int main(int argc,char *argv[])
{
    uint8_t line;
    char buf[LINE_LEN+2];	// 2 = { \n, \0 }

    sc2004c_gpio_assign_t assign;
    assign.rs = 21;
    assign.en = 22;
    assign.d4 = 24;
    assign.d5 = 25;
    assign.d6 = 26;
    assign.d7 = 27;

    sc2004c_init(&assign);

    line = 0;
    while (fgets(buf, sizeof(buf) , stdin) != NULL) {
        uint8_t i = strlen(buf);
        if (buf[i-1] == '\n') {
            i--;
        }
        for (; i < (sizeof(buf)-2); i++) {
            buf[i] = ' ';
        }
        buf[sizeof(buf)-2] = '\0';

        sc2004c_print(buf);
        sc2004c_set_line(++line % 4);
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
