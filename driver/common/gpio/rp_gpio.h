/*
 * Raspberry Pi GPIO utility
 *
 * Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 * original: http://www.myu.ac.jp/~xkozima/lab/resource/raspberry/IOKit/raspGPIO.h
 */

#ifndef RASPBERRY_PI_GPIO_H
#define RASPBERRY_PI_GPIO_H

#include <stdint.h>

// BCM2835 ARM Peripherals 
// Table 6-3 – GPIO Alternate function select register 1 
typedef enum {
    RP_GPIO_INPUT	= 0x0,
    RP_GPIO_OUTPUT	= 0x1,
    RP_GPIO_ALT0  	= 0x4,
    RP_GPIO_ALT1  	= 0x5,
    RP_GPIO_ALT2  	= 0x6,
    RP_GPIO_ALT3  	= 0x7,
    RP_GPIO_ALT4  	= 0x3,
    RP_GPIO_ALT5  	= 0x2
} rp_gpio_mode_t;

typedef enum {
    RP_GPIO_L		= 0x0,
    RP_GPIO_H		= 0x1
} rp_gpio_level_t;

void rp_gpio_init();
void rp_gpio_set_mode(uint8_t pin_no, rp_gpio_mode_t mode);
void rp_gpio_get_input(uint8_t pin_no, rp_gpio_level_t *level);
void rp_gpio_set_output(uint8_t pin_no, rp_gpio_level_t level);
void rp_gpio_set_output_bits(uint32_t level_map, uint32_t mask);

#endif

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
