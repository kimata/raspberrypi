/*
 * Raspberry Pi GPIO utility
 *
 * Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 */

#ifndef ADT7410_CTRL_H
#define ADT7410_CTRL_H

#include <stdint.h>

#define ADT7410_REG_TEMP	0x00
#define ADT7410_REG_STAT	0x02
#define ADT7410_REG_CONF	0x03

int adt7410_init(uint8_t dev_addr);
int adt7410_sense(uint8_t dev_addr, float *value);

#endif

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
