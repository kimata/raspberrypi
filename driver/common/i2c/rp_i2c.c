/*
 * Raspberry Pi I2C Driver
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

#include "rp_i2c.h"

#define BUS_ID      1
#define RETRY_COUNT 5
#define SLEEP_CYCLE 1000000

int rp_i2c_init()
{
    return 0;
}

int rp_i2c_open()
{
    char i2c_dev_path[64];

    sprintf(i2c_dev_path, "/dev/i2c-%d", BUS_ID);

    return open(i2c_dev_path, O_RDWR);
}

int rp_i2c_read(uint8_t dev_addr, uint8_t reg_addr,
                uint8_t *read_buf, uint8_t read_size)
{
    int fd;
    int ret;

    if ((fd = rp_i2c_open()) < 0) {
        return fd;
    }

    if ((ret = ioctl(fd, I2C_SLAVE, dev_addr)) < 0) {
        return ret;
    }

    if ((ret = write(fd, &reg_addr, sizeof(reg_addr))) != sizeof(reg_addr)) {
        return ret;
    }
    if ((ret = read(fd, read_buf, read_size)) != read_size) {
        return ret;
    }

    close(fd);

    return 0;
}

int rp_i2c_write(uint8_t dev_addr, uint8_t reg_addr,
                 uint8_t *write_buf, uint8_t write_size)
{
    int fd;
    int ret;
    uint8_t *buf;

    buf = alloca(write_size + 1);
    buf[0] = reg_addr;
    for (uint8_t i = 0; i < write_size; i++) {
        buf[i + 1] = write_buf[i];
    }

    if ((fd = rp_i2c_open()) < 0) {
        return fd;
    }

    if ((ret = ioctl(fd, I2C_SLAVE, dev_addr)) < 0) {
        return ret;
    }

    if ((ret = write(fd, buf, write_size+1)) != (write_size+1)) {
        return ret;
    }

    close(fd);

    return 0;
}

int rp_i2c_write_verify(uint8_t dev_addr, uint8_t reg_addr,
                        uint8_t *write_buf, uint8_t write_size)
{
    uint8_t *read_buf;
    int ret;

    read_buf = alloca(write_size);
  
    ret = 0;
    ret |= rp_i2c_write(dev_addr, reg_addr, write_buf, write_size);
    ret |= rp_i2c_read(dev_addr, reg_addr, read_buf, write_size);

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

int rp_i2c_read16(uint8_t dev_addr, uint8_t reg_addr,
                  uint16_t *read_buf, uint8_t read_size)
{
    uint16_t *buf;
    int ret;

    buf = (uint16_t *)alloca(read_size * sizeof(uint16_t));

    ret = rp_i2c_read(dev_addr, reg_addr, (uint8_t *)buf, read_size * sizeof(uint16_t));    
    for (uint8_t i = 0; i < read_size; i++) {
        read_buf[i] = be16toh(buf[i]);
    }

    return ret;
}

int rp_i2c_write16(uint8_t dev_addr, uint8_t reg_addr,
                   uint16_t *write_buf, uint8_t write_size)
{
    uint16_t *buf;

    buf = (uint16_t *)alloca(write_size * sizeof(uint16_t));
    
    for (uint8_t i = 0; i < write_size; i++) {
        buf[i] = htobe16(write_buf[i]);
    }

    return rp_i2c_write(dev_addr, reg_addr, (uint8_t *)buf, write_size * sizeof(uint16_t));
}

int rp_i2c_write_verify16(uint8_t dev_addr, uint8_t reg_addr,
                          uint16_t *write_buf, uint8_t write_size)
{
    uint16_t *read_buf;
    int ret;

    read_buf = alloca(write_size);

    ret = 0;
    ret |= rp_i2c_write16(dev_addr, reg_addr, write_buf, write_size);
    ret |= rp_i2c_read16(dev_addr, reg_addr, read_buf, write_size);

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
