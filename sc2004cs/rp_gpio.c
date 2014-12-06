/*
 * Raspberry Pi GPIO utility
 *
 * Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 * original: http://www.myu.ac.jp/~xkozima/lab/resource/raspberry/IOKit/raspGPIO.cpp
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "rp_gpio.h"

// BCM2835 ARM Peripherals 
// 1.2 Address map
#define PERI_BASE   0x20000000
#define GPIO_OFF    0x200000
#define GPIO_BASE   (PERI_BASE + GPIO_OFF)

static volatile uint32_t *gpio_reg = NULL;

void rp_gpio_init()
{
    int fd;
    void *gpio_map;

    if (gpio_reg != NULL) return;

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        fprintf(stderr, "ERROR: cannot open /dev/mem (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    gpio_map = mmap(NULL, sysconf(_SC_PAGE_SIZE),
                    PROT_READ | PROT_WRITE, MAP_SHARED,
                    fd, GPIO_BASE);
    close(fd);

    if ((int32_t)gpio_map == -1) {
        fprintf(stderr, "ERROR: cannot map /dev/mem (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    gpio_reg = (uint32_t *) gpio_map;
}

void rp_gpio_set_mode(uint8_t pin_no, rp_gpio_mode_t mode)
{
    if (pin_no > 27) {
        fprintf(stderr, "ERROR: pin number out of range (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    uint8_t reg_index = pin_no / 10;
    uint8_t reg_offs = (pin_no % 10) * 3;
    uint32_t reg_mask = ~(0x7 << reg_offs);

    gpio_reg[reg_index] = (gpio_reg[reg_index] & reg_mask) | ((mode & 0x7) << reg_offs);
}

void rp_gpio_set_output(uint8_t pin_no, uint8_t level)
{
    if (pin_no > 27) {
        fprintf(stderr, "ERROR: pin number out of range (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    if (level == 0) {
        gpio_reg[10] = 0x1 << pin_no;
    } else {
        gpio_reg[7] = 0x1 << pin_no;
    }
}

void rp_gpio_set_output_bits(uint32_t level_bits)
{
    gpio_reg[10] = (~level_bits) & 0x0FFFFFFF;
    gpio_reg[7] = level_bits & 0x0FFFFFFF;
}

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
