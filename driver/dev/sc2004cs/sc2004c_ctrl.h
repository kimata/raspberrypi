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

#ifndef SC2004C_CTRL_H
#define SC2004C_CTRL_H

#include <stdint.h>

typedef struct sc2004c_gpio_assign {
    uint8_t rs;
    uint8_t en;
    uint8_t d4;
    uint8_t d5;
    uint8_t d6;
    uint8_t d7;
} sc2004c_gpio_assign_t;

void sc2004c_init(sc2004c_gpio_assign_t *assign);
void sc2004c_print(const char *str);
void sc2004c_set_line(uint8_t line);
void sc2004c_set_pos(uint8_t line, uint8_t offset);

#endif

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
