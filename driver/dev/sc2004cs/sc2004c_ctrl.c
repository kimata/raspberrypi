/*
 * Raspberry Pi SC2004C Utility
 *
 * Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/time.h>

#include "rp_gpio.h"
#include "sc2004c_ctrl.h"

#define  DB4        (1 << 0)
#define  DB5        (1 << 1)
#define  DB6        (1 << 2)
#define  DB7        (1 << 3)

static sc2004c_gpio_assign_t setting;
static uint32_t              gpio_mask;

void sc2004c_wait(struct timespec *time)
{
    uint32_t nsec = (uint32_t)time->tv_nsec;
    for (uint32_t i = 0; i < nsec; i++) {
        __asm__ volatile("nop");
    }
}

void sc2004c_exec_cmd4(uint8_t rs, uint8_t code, bool is_wait)
{
    struct timespec setup_time      = { 0, 80 };                // TDSW = 80ns
    struct timespec hold_time       = { 0, 10 };                // TH = 10ns
    struct timespec cmd_wait_time   = { 0, (long)(37   * 1E3) };// 37us
    uint32_t cmd;

    cmd = ((rs & 0x1) << setting.rs) |
        (((code >> 0) & 0x1 ) << setting.d4) | 
        (((code >> 1) & 0x1 ) << setting.d5) | 
        (((code >> 2) & 0x1 ) << setting.d6) | 
        (((code >> 3) & 0x1 ) << setting.d7);

    rp_gpio_set_output_bits((1 << setting.en) | cmd, gpio_mask);
    sc2004c_wait(&setup_time);
    rp_gpio_set_output_bits(cmd, gpio_mask);
    sc2004c_wait(&hold_time);

    if (is_wait) {
        sc2004c_wait(&cmd_wait_time);
    }
}

void sc2004c_exec_cmd8(uint8_t rs, uint8_t code)
{
    sc2004c_exec_cmd4(rs, code >> 4, false);
    sc2004c_exec_cmd4(rs, code & 0xf, true);
}

void sc2004c_putc(const unsigned char c)
{
    sc2004c_exec_cmd8(1, c);
}

void sc2004c_init(sc2004c_gpio_assign_t *assign)
{
    struct timespec init_wait1_time = { 0, (long)(4.1  * 1E6) };    // 4.1ms
    struct timespec init_wait2_time = { 0, (long)(100  * 1E3) };    // 100us
    struct timespec clear_wait_time = { 0, (long)(1.53 * 1E6) };    // 1.53ms

    setting = *assign;
    gpio_mask = (0x1 << setting.rs) | (0x1 << setting.en) |
        (0x1 << setting.d4) | (0x1 << setting.d5) |
        (0x1 << setting.d6) | (0x1 << setting.d7);

    rp_gpio_init();
    rp_gpio_set_output_bits(0, gpio_mask);

    rp_gpio_set_mode(setting.rs, RP_GPIO_OUTPUT);
    rp_gpio_set_mode(setting.en, RP_GPIO_OUTPUT);
    rp_gpio_set_mode(setting.d4, RP_GPIO_OUTPUT);
    rp_gpio_set_mode(setting.d5, RP_GPIO_OUTPUT);
    rp_gpio_set_mode(setting.d6, RP_GPIO_OUTPUT);
    rp_gpio_set_mode(setting.d7, RP_GPIO_OUTPUT);

    sc2004c_exec_cmd4(0, DB5 | DB4, true);
    sc2004c_wait(&init_wait1_time);

    sc2004c_exec_cmd4(0, DB5 | DB4, true);
    sc2004c_wait(&init_wait2_time);

    sc2004c_exec_cmd4(0, DB5 | DB4, true);

    // 4 bit mode
    sc2004c_exec_cmd4(0, DB5, true);
    // 2 lines, 5x11 dots
    sc2004c_exec_cmd8(0, ((DB5) << 4) | (DB7 | DB6));
    // display off
    sc2004c_exec_cmd8(0, ((0) << 4) | (DB7));
    // display clear
    sc2004c_exec_cmd8(0, ((0) << 4) | (DB4));
    sc2004c_wait(&clear_wait_time);
    // entry mode set
    sc2004c_exec_cmd8(0, ((0) << 4) | (DB6 | DB5));
    // display on
    sc2004c_exec_cmd8(0, ((0) << 4) | (DB7 | DB6));
}

void sc2004c_print(const char *str)
{
    uint8_t i;
    unsigned char c;

    i = 0;
    while ((c = (unsigned char)str[i++]) != '\0') {
        sc2004c_putc(c);
    }
}

void sc2004c_set_line(uint8_t line)
{
    sc2004c_set_pos(line, 0);
}

void sc2004c_set_pos(uint8_t line, uint8_t offset)
{
    uint8_t addr;
    switch (line) {
    case 0: addr = 0x00; break;
    case 1: addr = 0x40; break;
    case 2: addr = 0x14; break;
    case 3: addr = 0x54; break;
    default:
        fprintf(stderr, "ERROR: line exceeds 3 (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    addr += offset;

    sc2004c_exec_cmd8(0, (1 << 7) | addr);
}

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
