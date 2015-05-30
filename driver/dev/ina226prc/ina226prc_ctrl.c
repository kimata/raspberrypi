/*
 * Raspberry Pi INA226PRC Utility
 *
 * Copyright (C) 2015 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 */

#define _BSD_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <endian.h>

#include "rp_lib.h"
#include "rp_i2c.h"
#include "ina226prc_ctrl.h"

void ina226prc_init(uint8_t dev_addr, ina226prc_conf_t *conf)
{
    uint16_t reg_value;
    int ret;

    ret = 0;

    rp_i2c_init();

    reg_value = (1 << 14) |
        (conf->average_number << 9) |
        (conf->bus_conv_time << 6) | (conf->shunt_conv_time << 3) |
        (conf->mode << 0);
    ret |= rp_i2c_write_verify16(dev_addr, INA226PRC_REG_CONF, &reg_value, 1);

    /* rp_sleep(10000000000); */
    rp_sleep(100000);
    reg_value = (uint16_t)(51200 / conf->shunt_mohm);
    rp_i2c_write_verify16(dev_addr, INA226PRC_REG_CALIB, &reg_value, 1);
    
    printf("ret = %d\n", ret);
}


void ina226prc_sense(uint8_t dev_addr, ina226prc_value_t *value)
{
    uint16_t reg_value;    
    /* rp_i2c_read16(dev_addr, INA226PRC_REG_BUS_VOL, &reg_value, 1); */

    /* printf("vol = %d\n", reg_value); */

    /* @i2c.write(0x02) */
    /* @v_val = conver_signed(@i2c.read(2)) */

    /* # @i2c.write(0x04) */
    /* @i2c.write(0x04) */
    /* @c_val = conver_signed(@i2c.read(2)) */

    /* @i2c.write(0x03) */
    /* @p_val = conver_signed(@i2c.read(2)) */

}



/* float adt7410_sense(uint8_t dev_addr) */
/* { */
/*     uint16_t buf; */
/*     int16_t val; */

/*     rp_i2c_read(dev_addr, ADT7410_REG_TEMP, (uint8_t *)&buf, sizeof(buf)); */

/*     val = (int16_t)be16toh(buf); */
/*     if (val & 0x8000) { */
/*         val -= 65536; */
/*     } */

/*     return val / 128.0; */
/* } */

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
