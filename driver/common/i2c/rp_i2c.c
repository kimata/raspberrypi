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

#define BUS_ID  1

void rp_i2c_init()
{

}

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

void rp_i2c_write_verify(uint8_t dev_addr, uint8_t reg_addr,
                         uint8_t *write_buf, uint8_t write_size)
{
    uint8_t *read_buf;

    read_buf = alloca(write_size);
  
    rp_i2c_write(dev_addr, reg_addr, write_buf, write_size);
    rp_i2c_read(dev_addr, reg_addr, read_buf, write_size);

    for (uint8_t i = 0; i < write_size; i++) {
        if (read_buf[i] != write_buf[i]) {
            fprintf(stderr, "ERROR: verify (write: %02x, read: %02x (at %s:%d)\n",
                    write_buf[i], read_buf[i], __FILE__, __LINE__);
            exit(EXIT_FAILURE);
        }
    }
}

void rp_i2c_read16(uint8_t dev_addr, uint8_t reg_addr,
                   uint16_t *read_buf, uint8_t read_size)
{
    uint8_t *buf;

    buf = alloca(read_size * sizeof(uint16_t));

    rp_i2c_read(dev_addr, reg_addr, (uint8_t *)buf, read_size * sizeof(uint16_t));    
    for (uint8_t i = 0; i < read_size; i++) {
        read_buf[i] = be16toh(buf[i]);
    }
}

void rp_i2c_write16(uint8_t dev_addr, uint8_t reg_addr,
                    uint16_t *write_buf, uint8_t write_size)
{
    uint8_t *buf;

    buf = alloca(write_size * sizeof(uint16_t));
    
    for (uint8_t i = 0; i < write_size; i++) {
        buf[i] = htobe16(write_buf[i]);
    }
    rp_i2c_write(dev_addr, reg_addr, (uint8_t *)buf, write_size * sizeof(uint16_t));
}

void rp_i2c_write_verify16(uint8_t dev_addr, uint8_t reg_addr,
                           uint16_t *write_buf, uint8_t write_size)
{
    uint16_t *read_buf;

    read_buf = alloca(write_size);
  
    rp_i2c_write16(dev_addr, reg_addr, write_buf, write_size);
    rp_i2c_read16(dev_addr, reg_addr, read_buf, write_size);

    for (uint8_t i = 0; i < write_size; i++) {
        if (read_buf[i] != write_buf[i]) {
            fprintf(stderr, "ERROR: verify (write: %04x, read: %04x (at %s:%d)\n",
                    write_buf[i], read_buf[i], __FILE__, __LINE__);
            exit(EXIT_FAILURE);
        }
    }
}

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
