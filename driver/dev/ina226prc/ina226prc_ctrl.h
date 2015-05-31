/*
 * Raspberry Pi INA226PRC Utility
 *
 * Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 */

#ifndef INA226PRC_CTRL_H
#define INA226PRC_CTRL_H

#include <stdint.h>

typedef enum {
    INA226PRC_REG_CONF          = 0,
    INA226PRC_REG_SHUNT_VOL     = 1,
    INA226PRC_REG_BUS_VOL       = 2,
    INA226PRC_REG_POWER         = 3,
    INA226PRC_REG_CURRENT       = 4,
    INA226PRC_REG_CALIB         = 5,
    INA226PRC_REG_MASK          = 6,
    INA226PRC_REG_ALERT         = 7
} ina226prc_reg_t;


typedef enum {
    INA226PRC_MODE_OFF          = 0,
    INA226PRC_MODE_SHUNT_TRIG   = 1,
    INA226PRC_MODE_BUS_TRIG     = 2,
    INA226PRC_MODE_BOTH_TRIG    = 3,
    // INA226PRC_MODE_OFF       = 4,
    INA226PRC_MODE_SHUNT_CONT   = 5,
    INA226PRC_MODE_BUS_CONT     = 6,
    INA226PRC_MODE_BOTH_CONT    = 7
} ina226prc_mode_t;

typedef enum {
    INA226PRC_AVG_1             = 0,
    INA226PRC_AVG_4             = 1,
    INA226PRC_AVG_16            = 2,
    INA226PRC_AVG_64            = 3,
    INA226PRC_AVG_128           = 4,
    INA226PRC_AVG_256           = 5,
    INA226PRC_AVG_512           = 6,
    INA226PRC_AVG_1024          = 7
} ina226prc_avg_num_t;

typedef enum {
    INA226PRC_CONV_140US        = 0,
    INA226PRC_CONV_204US        = 1,
    INA226PRC_CONV_332US        = 2,
    INA226PRC_CONV_588US        = 3,
    INA226PRC_CONV_1100US       = 4,
    INA226PRC_CONV_2116US       = 5,
    INA226PRC_CONV_4156US       = 6,
    INA226PRC_CONV_8244US       = 7
} ina226prc_conv_time_t;

typedef struct {
    uint16_t                shunt_mohm;     // mΩ of shunt register
    ina226prc_mode_t        mode;
    ina226prc_avg_num_t     average_number;
    ina226prc_conv_time_t   bus_conv_time;
    ina226prc_conv_time_t   shunt_conv_time;
} ina226prc_conf_t;

typedef struct {
    float                   voltage;
    float                   current;
    float                   power;
} ina226prc_value_t;

int ina226prc_init(uint8_t dev_addr, ina226prc_conf_t *conf);
int ina226prc_sense(uint8_t dev_addr, ina226prc_value_t *value);

#endif

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
