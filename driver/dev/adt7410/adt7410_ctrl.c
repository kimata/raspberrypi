/*
 * ADT7410 Driver
 *
 * Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
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

#include "rp_i2c_ioctl.h"
#include "adt7410_ctrl.h"

int adt7410_init(uint8_t dev_addr)
{
    uint8_t conf = 1 << 7;
    uint8_t buf;
    int ret = 0;

    ret |= rp_i2c_init_ioctl();
    ret |= rp_i2c_write_ioctl(dev_addr, ADT7410_REG_CONF, (uint8_t *)&conf, sizeof(conf));
    ret |= rp_i2c_read_ioctl(dev_addr, ADT7410_REG_CONF, (uint8_t *)&buf, sizeof(buf));

    // MEMO: verify
    if (buf != conf) {
        fprintf(stderr, "ERROR: verify conf reg (write, read) = (%x, %x) (at %s:%d)\n",
                conf, buf, __FILE__, __LINE__);
        return -1;
    }

    return ret;
}

int adt7410_sense(uint8_t dev_addr, float *value)
{
    uint16_t buf;
    int16_t val;
    int ret = 0;

    ret = rp_i2c_read_ioctl(dev_addr, ADT7410_REG_TEMP, (uint8_t *)&buf, sizeof(buf));

    val = (int16_t)be16toh(buf);
    if (val & 0x8000) {
        val -= 65536;
    }

    *value = val / 128.0;

    return ret;
}

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
