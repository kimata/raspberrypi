/*
 * Raspberry Pi I2C Utility (using GPIO)
 *
 * Copyright (C) 2015 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "rp_i2c.h"
#include "rp_gpio.h"

#define PIN_SDA             4
#define PIN_SCL             5

#if !defined(ARRAY_SIZE_OF)
#define ARRAY_SIZE_OF(A)    (sizeof(A)/sizeof((A)[0]))
#endif

// copy from Linux/include/uapi/linux/i2c-dev.h
struct i2c_msg {
    uint16_t addr;                  /* slave address                        */
    uint16_t flags;
#define I2C_M_TEN           0x0010  /* this is a ten bit chip address       */
#define I2C_M_RD            0x0001  /* read data, from slave to master      */
#define I2C_M_STOP          0x8000  /* if I2C_FUNC_PROTOCOL_MANGLING        */
#define I2C_M_NOSTART       0x4000  /* if I2C_FUNC_NOSTART                  */
#define I2C_M_REV_DIR_ADDR  0x2000  /* if I2C_FUNC_PROTOCOL_MANGLING        */
#define I2C_M_IGNORE_NAK    0x1000  /* if I2C_FUNC_PROTOCOL_MANGLING        */
#define I2C_M_NO_RD_ACK     0x0800  /* if I2C_FUNC_PROTOCOL_MANGLING        */
#define I2C_M_RECV_LEN      0x0400  /* length will be first received byte   */
    uint16_t len;                   /* msg length                           */
    uint8_t *buf;                   /* pointer to msg data                  */
};
struct i2c_rdwr_ioctl_data {
    struct i2c_msg *msgs;           /* pointers to i2c_msgs                 */
    uint32_t nmsgs;                 /* number of i2c_msgs                   */
};
// copy from Linux/arch/arm/include/asm/posix_types.h
typedef long __kernel_time_t;
// copy from Linux/include/linux/time.h
struct timespec {
    __kernel_time_t tv_sec;         /* seconds */
    long            tv_nsec;        /* nanoseconds */
};

void rp_i2c_gpio_sleep(struct timespec *time)
{
    uint32_t nsec = (uint32_t)time->tv_nsec / 10; // FIXME:
    for (uint32_t i = 0; i < nsec; i++) {
        __asm__ volatile("nop");
    }
}

void rp_i2c_gpio_wait_quarter_clock()
{
    struct timespec clock_time          = { 0, 10 * 1E3 };  // 10us = 100kHz
    struct timespec clock_quarter_time  = { 0, clock_time.tv_nsec  / 4 };

    rp_i2c_gpio_sleep(&clock_quarter_time);
}

void rp_i2c_gpio_wait_half_clock()
{
    rp_i2c_gpio_wait_quarter_clock();
    rp_i2c_gpio_wait_quarter_clock();
}

void rp_i2c_gpio_wait_clock()
{
    rp_i2c_gpio_wait_half_clock();
    rp_i2c_gpio_wait_half_clock();
}

void rp_i2c_gpio_set_signal(uint8_t pin_no, rp_gpio_level_t level) {
    if (level == RP_GPIO_L) {
        // output L
        rp_gpio_set_output(pin_no, level);
        rp_gpio_set_mode(pin_no, RP_GPIO_OUTPUT);
    } else {
        // Hi-Z
        rp_gpio_set_mode(pin_no, RP_GPIO_INPUT);
    }
}

void rp_i2c_gpio_set_sda(rp_gpio_level_t level) {
    rp_i2c_gpio_set_signal(PIN_SDA, level);
}

void rp_i2c_gpio_set_scl(rp_gpio_level_t level) {
    rp_i2c_gpio_set_signal(PIN_SCL, level);
}

void rp_i2c_gpio_get_sda(rp_gpio_level_t *level) {
    rp_gpio_set_mode(PIN_SDA, RP_GPIO_INPUT);
    rp_gpio_get_input(PIN_SDA, level);
}

void rp_i2c_gpio_get_scl(rp_gpio_level_t *level) {
    rp_gpio_set_mode(PIN_SCL, RP_GPIO_INPUT);
    rp_gpio_get_input(PIN_SCL, level);
}

void rp_i2c_gpio_write_bit(uint32_t bit) {
    if (bit) {
        rp_i2c_gpio_set_sda(RP_GPIO_H);
    } else {
        rp_i2c_gpio_set_sda(RP_GPIO_L);
    }
    rp_i2c_gpio_wait_quarter_clock();
    rp_i2c_gpio_set_scl(RP_GPIO_H);
    rp_i2c_gpio_wait_half_clock();
    rp_i2c_gpio_set_scl(RP_GPIO_L);
    rp_i2c_gpio_wait_quarter_clock();
}

void rp_i2c_gpio_read_bit(uint32_t *bit) {
    struct timespec wait_time = { 0, 100 };
    rp_gpio_level_t level;

    rp_i2c_gpio_wait_quarter_clock();
    rp_i2c_gpio_set_scl(RP_GPIO_H);
    rp_i2c_gpio_wait_quarter_clock();
    // check clock stretch
    while (1) {
        rp_i2c_gpio_get_scl(&level);
        if (level != RP_GPIO_L) {
            break;
        }
        rp_i2c_gpio_sleep(&wait_time);
    }
    rp_i2c_gpio_get_sda(&level);
    rp_i2c_gpio_wait_quarter_clock();
    rp_i2c_gpio_set_scl(RP_GPIO_L);
    rp_i2c_gpio_wait_quarter_clock();

    if (level == RP_GPIO_H) {
        *bit = 1;
    } else {
        *bit = 0;
    }
}

void rp_i2c_gpio_wait_ack() {
    rp_gpio_level_t level;

    rp_i2c_gpio_set_sda(RP_GPIO_H);
    rp_i2c_gpio_wait_quarter_clock();
    rp_i2c_gpio_set_scl(RP_GPIO_H);
    rp_i2c_gpio_wait_quarter_clock();
    rp_i2c_gpio_get_sda(&level);
    rp_i2c_gpio_wait_quarter_clock();
    rp_i2c_gpio_set_scl(RP_GPIO_L);
    rp_i2c_gpio_wait_quarter_clock();
}

void rp_i2c_gpio_write_msg(struct i2c_msg *i2c_msg) {
    for (uint32_t i = 0; i < i2c_msg->len; i++) {
        // write data
        uint8_t data = i2c_msg->buf[i];
        for (uint32_t j = 0; j < 8; j++) {
            rp_i2c_gpio_write_bit(data & 0x80);
            data <<= 1;
        }
        rp_i2c_gpio_wait_ack();
    }
}

void rp_i2c_gpio_read_msg(struct i2c_msg *i2c_msg) {
    for (uint32_t i = 0; i < i2c_msg->len; i++) {
        // read data
        uint8_t data = 0;
        for (uint32_t j = 0; j < 8; j++) {
            uint32_t bit;
            rp_i2c_gpio_read_bit(&bit);
            data = (data << 1) | (bit & 0x1);
        }
        rp_i2c_gpio_wait_ack();
        i2c_msg->buf[i] = data;
    }
}

void rp_i2c_gpio_send_msg(struct i2c_msg *i2c_msg, uint8_t is_last) {
    // send START condition
    rp_i2c_gpio_set_sda(RP_GPIO_L);
    rp_i2c_gpio_wait_half_clock();
    rp_i2c_gpio_set_scl(RP_GPIO_L);
    rp_i2c_gpio_wait_quarter_clock();

    // send device Address
    uint16_t addr = i2c_msg->addr;
    for (uint32_t i = 0; i < 7; i++) {
        // NOTE: addr is 7bit format, so mask is 0x40
        rp_i2c_gpio_write_bit(addr & 0x40);
        addr <<= 1;
    }
    rp_i2c_gpio_write_bit(i2c_msg->flags & I2C_M_RD);
    
    rp_i2c_gpio_wait_ack();

    if (i2c_msg->flags & I2C_M_RD) {
        rp_i2c_gpio_read_msg(i2c_msg);
    } else {
        rp_i2c_gpio_write_msg(i2c_msg);
    }
    if (is_last)  {
        // send STOP condition
        rp_i2c_gpio_set_sda(RP_GPIO_L);
        rp_i2c_gpio_wait_quarter_clock();
        rp_i2c_gpio_set_scl(RP_GPIO_H);
        rp_i2c_gpio_wait_quarter_clock();
        rp_i2c_gpio_set_sda(RP_GPIO_H);
        rp_i2c_gpio_wait_quarter_clock();
    } else {
        rp_i2c_gpio_set_sda(RP_GPIO_H);
        rp_i2c_gpio_wait_quarter_clock();
        rp_i2c_gpio_set_scl(RP_GPIO_H);
        rp_i2c_gpio_wait_quarter_clock();
    }
}

void rp_i2c_init() {
    rp_gpio_init();
    rp_gpio_set_mode(PIN_SDA, RP_GPIO_INPUT);
    rp_gpio_set_mode(PIN_SCL, RP_GPIO_INPUT);
}

void rp_i2c_read(uint8_t dev_addr, uint8_t reg_addr,
                 uint8_t *read_buf, uint8_t read_size)
{
    struct i2c_msg read_msgs[2] = {
        {
            .addr = dev_addr,
            .flags = 0,
            .len = 1,
            .buf = &reg_addr
        },
        {
            .addr = dev_addr,
            .flags = I2C_M_RD,
            .len = read_size,
            .buf = read_buf
        }
    };
    struct i2c_rdwr_ioctl_data read_data = {
        .msgs = read_msgs,
        .nmsgs = sizeof(read_msgs)/sizeof(struct i2c_msg)
    };

    for (uint32_t i = 0; i < read_data.nmsgs; i++) {
        rp_i2c_gpio_send_msg(&(read_data.msgs[i]), i == read_data.nmsgs);
    }
}

void rp_i2c_write(uint8_t dev_addr, uint8_t reg_addr,
                  uint8_t *write_buf, uint8_t write_size)
{
    uint8_t buf[3];

    struct i2c_msg write_msgs[1] = {
        {
            .addr = dev_addr,
            .flags = 0,
            .len = write_size + 1,
            .buf = buf
        }
    };
    struct i2c_rdwr_ioctl_data write_data = {
        .msgs = write_msgs,
        .nmsgs = sizeof(write_msgs)/sizeof(struct i2c_msg)
    };

    if (write_size > 2) {
        fprintf(stderr, "FATAL: unexpected write size (at %s:%d)\n",
                __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    buf[0] = reg_addr;
    for (uint8_t i = 0; i < write_size; i++) {
        buf[i + 1] = write_buf[i];
    }

    for (uint32_t i = 0; i < write_data.nmsgs; i++) {
        rp_i2c_gpio_send_msg(&(write_data.msgs[i]), i == write_data.nmsgs);
    }
}

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
