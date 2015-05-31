#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ina226prc_ctrl.h"

int sense(uint8_t dev_addr)
{
    ina226prc_value_t value;
    int ret;

    if ((ret = ina226prc_sense(dev_addr, &value)) == 0) {
        printf("V: %.3f, I: %.3f, P: %.3f\n",
               value.voltage, value.current, value.power);
    }

    return ret;
}

int main(int argc, char *argv[])
{
    uint8_t dev_addr;
    char *err_char;
    ina226prc_conf_t conf;
    uint32_t count;
    uint32_t i;

    if (argc < 2) {
        fprintf(stderr, "USAGE: %s  DEV_ADDR [COUNT]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    dev_addr = strtol(argv[1], &err_char, 0);
    if (argc == 3) {
        count = strtol(argv[2], &err_char, 0);
    } else {
        count = 0;
    }

    conf.shunt_mohm		= 25; // 25mƒ¶
    conf.mode			= INA226PRC_MODE_BOTH_CONT;
    conf.average_number	= INA226PRC_AVG_16;
    conf.bus_conv_time	= INA226PRC_CONV_332US;
    conf.shunt_conv_time= INA226PRC_CONV_332US;

    if (ina226prc_init(dev_addr, &conf) != 0) {
        exit(EXIT_FAILURE);
    }

    i = 0;
    while (1) {
        if (sense(dev_addr) !=0) {
            return EXIT_FAILURE;
        }
        if (++i == count) {
            break;
        }
        sleep(1);
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
