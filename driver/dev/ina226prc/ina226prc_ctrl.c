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

#include "rp_i2c.h"
#include "ina226prc_ctrl.h"

void ina226prc_init(uint8_t dev_addr, ina226prc_conf_t *conf)
{
    uint16_t reg_value;

    rp_i2c_init();

    reg_value = (uint16_t)(51200 / conf->shunt_mohm);

    rp_i2c_write_verify16(dev_addr, 5, &reg_value, 1);


    /* # shunt resistor = 0.025Ω */
    /* exec_cmd("i2cset -y 1 0x#{dev_addr.to_s(16)} 0x05 0x08 0x00 i") */
    /* # conversion time = 332us, number of average = 16 */
    /* exec_cmd("i2cset -y 1 0x#{dev_addr.to_s(16)} 0x00 0x04 0x97 i") */


}


void ina226prc_sense(ina226prc_value_t *value)
{

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
