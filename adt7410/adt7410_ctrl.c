/*
 * Raspberry Pi ADT7410 Utility
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

#include "rp_i2c.h"
#include "adt7410_ctrl.h"

void adt7410_init(uint8_t dev_addr)
{
    uint8_t conf = 1 << 7;
    uint8_t buf;

    rp_i2c_write(dev_addr, ADT7410_REG_CONF, (uint8_t *)&conf, sizeof(conf));
    rp_i2c_read(dev_addr, ADT7410_REG_CONF, (uint8_t *)&buf, sizeof(buf));

    // MEMO: verify
    if (buf != conf) {
        fprintf(stderr, "ERROR: write conf reg (at %s:%d)\n",
                __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
}

float adt7410_sense(uint8_t dev_addr)
{
    uint16_t buf;
    int16_t val;

    rp_i2c_read(dev_addr, ADT7410_REG_TEMP, (uint8_t *)&buf, sizeof(buf));

    val = (int16_t)be16toh(buf);
    if (val & 0x8000) {
        val -= 65536;
    }

    return val / 128.0;
}

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
