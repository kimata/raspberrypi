/*
 * Raspberry Pi Driver Library
 *
 * Copyright (C) 2015 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 */

#include <stdint.h>

#include "rp_lib.h"

void rp_sleep(uint32_t count)
{
    for (uint32_t i = 0; i < count; i++) {
        __asm__ volatile("nop");
    }
}

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
