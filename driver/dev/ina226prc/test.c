#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ina226prc_ctrl.h"

/* void test(uint8_t dev_addr) */
/* { */
/*     float v = adt7410_sense(dev_addr); */
/*     printf("temp: %.2f\n", v); */
/* } */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char *argv[])
{
    uint8_t dev_addr = 0x45;
    ina226prc_conf_t conf;
    
    conf.shunt_mohm		= 25; // 25mƒ¶
    conf.mode			= INA226PRC_MODE_BOTH_CONT;
    conf.average_number	= INA226PRC_AVG_16;
    conf.bus_conv_time	= INA226PRC_CONV_332US;
    conf.shunt_conv_time= INA226PRC_CONV_332US;

    ina226prc_init(dev_addr, &conf);
    ina226prc_sense(dev_addr, NULL);

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
