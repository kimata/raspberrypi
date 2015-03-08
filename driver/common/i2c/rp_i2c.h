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

#include <stdint.h>

void rp_i2c_init();

void rp_i2c_read(uint8_t dev_addr, uint8_t reg_addr,
                 uint8_t *read_buf, uint8_t read_size);

void rp_i2c_write(uint8_t dev_addr, uint8_t reg_addr,
                  uint8_t *write_buf, uint8_t write_size);

void rp_i2c_write_verify(uint8_t dev_addr, uint8_t reg_addr,
                         uint8_t *write_buf, uint8_t write_size);

void rp_i2c_read16(uint8_t dev_addr, uint8_t reg_addr,
                   uint16_t *read_buf, uint8_t read_size);

void rp_i2c_write16(uint8_t dev_addr, uint8_t reg_addr,
                    uint16_t *write_buf, uint8_t write_size);

void rp_i2c_write_verify16(uint8_t dev_addr, uint8_t reg_addr,
                           uint16_t *write_buf, uint8_t write_size);

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:

