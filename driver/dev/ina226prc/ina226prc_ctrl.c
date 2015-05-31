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

#ifdef I2C_IOCTL
#include "rp_i2c_ioctl.h"
#define rp_i2c_init(...) 			rp_i2c_init_ioctl(__VA_ARGS__)
#define rp_i2c_read(...) 			rp_i2c_read_ioctl(__VA_ARGS__)
#define rp_i2c_write(...) 			rp_i2c_write_ioctl(__VA_ARGS__)
#define rp_i2c_write_verify(...) 	rp_i2c_write_verify_ioctl(__VA_ARGS__)
#define rp_i2c_read16(...)			rp_i2c_read16_ioctl(__VA_ARGS__)
#define rp_i2c_write16(...)			rp_i2c_write16_ioctl(__VA_ARGS__)
#define rp_i2c_write_verify16(...)	rp_i2c_write_verify16_ioctl(__VA_ARGS__)
#else
#include "rp_i2c.h"
#endif
#include "ina226prc_ctrl.h"

static float ina226prc_calc_voltage(uint16_t value)
{
    return ((float)value) * 1.25 / 1000.0;
}

static float ina226prc_calc_current(uint16_t value)
{
    return ((float)value) / 10000.0;
}

static float ina226prc_calc_power(uint16_t value)
{
    return ((float)value) * 0.0025;
}

int ina226prc_init(uint8_t dev_addr, ina226prc_conf_t *conf)
{
    uint16_t reg_value;
    int ret = 0;

    rp_i2c_init();

    reg_value = (uint16_t)(51200 / conf->shunt_mohm);
    ret |= rp_i2c_write_verify16(dev_addr, INA226PRC_REG_CALIB, &reg_value, 1);

    reg_value = (1 << 14) |
        (conf->average_number << 9) |
        (conf->bus_conv_time << 6) | (conf->shunt_conv_time << 3) |
        (conf->mode << 0);
    ret |= rp_i2c_write_verify16(dev_addr, INA226PRC_REG_CONF, &reg_value, 1);

    return ret;
}

int ina226prc_sense(uint8_t dev_addr, ina226prc_value_t *value)
{
    uint16_t reg_value;    
    float v;
    float i;
    float p;
    int ret = 0;

    ret |= rp_i2c_read16(dev_addr, INA226PRC_REG_BUS_VOL, &reg_value, 1);
    v = ina226prc_calc_voltage(reg_value);

    ret |= rp_i2c_read16(dev_addr, INA226PRC_REG_CURRENT, &reg_value, 1);
    i = ina226prc_calc_current(reg_value);

    ret |= rp_i2c_read16(dev_addr, INA226PRC_REG_POWER, &reg_value, 1);
    p = ina226prc_calc_power(reg_value);

    if (ret == 0) {
        value->voltage = v;
        value->current = i;
        value->power = p;
    }

    return ret;
}

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
