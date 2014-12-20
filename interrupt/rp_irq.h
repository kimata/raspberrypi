/*
 * Raspberry Pi IRQ utility
 *
 * Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 */

#ifndef RASPBERRY_PI_IRQ_H
#define RASPBERRY_PI_IRQ_H

#include <stdint.h>
#include <poll.h>

typedef enum {
    RP_IRQ_EDGE_FALLING = 0x1,
    RP_IRQ_EDGE_RISING  = 0x2,
    RP_IRQ_EDGE_BOTH    = 0x3,
} rp_irq_edge_mode_t;

typedef enum {
    RP_IRQ_STAT_L  		= 0,
    RP_IRQ_STAT_H 		= 1,
    RP_IRQ_STAT_TIMEOUT = -1,
} rp_irq_stat_t;

typedef struct {
    int fd;
    struct pollfd pfd;
} rp_irq_handle_t;

void rp_irq_enable(uint8_t pin_no, rp_irq_edge_mode_t mode);
void rp_irq_disable(uint8_t pin_no);
void rp_irq_init(uint8_t pin_no, rp_irq_handle_t *handle);
rp_irq_stat_t rp_irq_wait(rp_irq_handle_t *handle, uint32_t wait_msec);

#endif

// Local Variables:
// coding: utf-8-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
