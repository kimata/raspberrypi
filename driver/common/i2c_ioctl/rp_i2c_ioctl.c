/*
 * Raspberry Pi I2C Driver (using ioctl)
 *
 * Copyright (C) 2015 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 */

#define _BSD_SOURCE

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <endian.h>
#include <alloca.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "rp_i2c_ioctl.h"

#define BUS_ID      1
#define RETRY_COUNT 5
#define SLEEP_CYCLE 1000000

int rp_i2c_init_ioctl()
{
    return 0;
}

int rp_i2c_open_ioctl()
{
    char i2c_dev_path[64];

    sprintf(i2c_dev_path, "/dev/i2c-%d", BUS_ID);

    return open(i2c_dev_path, O_RDWR);
}

int rp_i2c_read_ioctl(uint8_t dev_addr, uint8_t reg_addr,
                      uint8_t *read_buf, uint8_t read_size)
{
    int fd;
    int ret;

    struct i2c_msg read_msgs[2] = {
        {
            .addr = dev_addr,
            .flags = I2C_M_IGNORE_NAK,
            .len = 1,
            .buf = &reg_addr
        },
        {
            .addr = dev_addr,
            .flags = I2C_M_RD|I2C_M_IGNORE_NAK,
            .len = read_size,
            .buf = read_buf
        }
    };
    struct i2c_rdwr_ioctl_data read_data = {
        .msgs = read_msgs,
        .nmsgs = sizeof(read_msgs)/sizeof(struct i2c_msg)
    };

    if ((fd = rp_i2c_open_ioctl()) < 0) {
        return fd;
    }
    ret = ioctl(fd, I2C_RDWR, &read_data);

    close(fd);

    return (ret < 0) ? ret : 0;
}

int rp_i2c_write_ioctl(uint8_t dev_addr, uint8_t reg_addr,
                       uint8_t *write_buf, uint8_t write_size)
{
    int fd;
    int ret;
    uint8_t *buf;

    buf = alloca(write_size + 1);

    struct i2c_msg write_msgs[1] = {
        {
            .addr = dev_addr,
            .flags = I2C_M_IGNORE_NAK,
            .len = write_size + 1,
            .buf = buf
        }
    };
    struct i2c_rdwr_ioctl_data write_data = {
        .msgs = write_msgs,
        .nmsgs = sizeof(write_msgs)/sizeof(struct i2c_msg)
    };

    buf[0] = reg_addr;
    for (uint8_t i = 0; i < write_size; i++) {
        buf[i + 1] = write_buf[i];
    }

    if ((fd = rp_i2c_open_ioctl()) < 0) {
        return fd;
    }
    ret = ioctl(fd, I2C_RDWR, &write_data);
    close(fd);

    return (ret < 0) ? ret : 0;
}

int rp_i2c_write_verify_ioctl(uint8_t dev_addr, uint8_t reg_addr,
                              uint8_t *write_buf, uint8_t write_size)
{
    uint8_t *read_buf;
    int ret;

    read_buf = alloca(write_size);
  
    ret = 0;
    ret |= rp_i2c_write_ioctl(dev_addr, reg_addr, write_buf, write_size);
    ret |= rp_i2c_read_ioctl(dev_addr, reg_addr, read_buf, write_size);

    if (ret != 0) {
        return -1;
    }

    for (uint8_t i = 0; i < write_size; i++) {
        if (read_buf[i] != write_buf[i]) {
            return -2;
        }
    }

    return 0;
}

int rp_i2c_read16_ioctl(uint8_t dev_addr, uint8_t reg_addr,
                        uint16_t *read_buf, uint8_t read_size)
{
    uint16_t *buf;
    int ret;

    buf = (uint16_t *)alloca(read_size * sizeof(uint16_t));

    ret = rp_i2c_read_ioctl(dev_addr, reg_addr, (uint8_t *)buf, read_size * sizeof(uint16_t));    
    for (uint8_t i = 0; i < read_size; i++) {
        read_buf[i] = be16toh(buf[i]);
    }

    return ret;
}

int rp_i2c_write16_ioctl(uint8_t dev_addr, uint8_t reg_addr,
                         uint16_t *write_buf, uint8_t write_size)
{
    uint16_t *buf;

    buf = (uint16_t *)alloca(write_size * sizeof(uint16_t));
    
    for (uint8_t i = 0; i < write_size; i++) {
        buf[i] = htobe16(write_buf[i]);
    }

    return rp_i2c_write_ioctl(dev_addr, reg_addr, (uint8_t *)buf, write_size * sizeof(uint16_t));
}

int rp_i2c_write_verify16_ioctl(uint8_t dev_addr, uint8_t reg_addr,
                                uint16_t *write_buf, uint8_t write_size)
{
    uint16_t *read_buf;
    int ret;

    read_buf = alloca(write_size);

    ret = 0;
    ret |= rp_i2c_write16_ioctl(dev_addr, reg_addr, write_buf, write_size);
    ret |= rp_i2c_read16_ioctl(dev_addr, reg_addr, read_buf, write_size);

    if (ret != 0) {
        return -1;
    }

    for (uint8_t i = 0; i < write_size; i++) {
        if (read_buf[i] != write_buf[i]) {
            return -1;
        }
    }

    return 0;
}

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
