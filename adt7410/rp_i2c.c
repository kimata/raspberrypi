/*
 * Raspberry Pi I2C Utility
 *
 * Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "rp_i2c.h"

#define BUS_ID  1

int rp_i2c_open()
{
    int fd;
    char i2c_dev_path[64];

    sprintf(i2c_dev_path, "/dev/i2c-%d", BUS_ID);
    if ((fd = open(i2c_dev_path, O_RDWR)) < 0) {
        fprintf(stderr, "ERROR: open i2c port (at %s:%d)\n",
                __FILE__, __LINE__);        
        exit(EXIT_FAILURE);
    }

    return fd;
}

void rp_i2c_read(uint8_t dev_addr, uint8_t reg_addr,
                 uint8_t *read_buf, uint8_t read_size)
{
    int fd;

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

    fd = rp_i2c_open();
    if (ioctl(fd, I2C_RDWR, &read_data) < 0) {
        fprintf(stderr, "ERROR: ioctl (at %s:%d)\n",
                __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void rp_i2c_write(uint8_t dev_addr, uint8_t reg_addr,
                  uint8_t *write_buf, uint8_t write_size)
{
    int fd;
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
    fd = rp_i2c_open();
    if (ioctl(fd, I2C_RDWR, &write_data) < 0) {
        fprintf(stderr, "ERROR: ioctl (at %s:%d)\n",
                __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
