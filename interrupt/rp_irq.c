/*
 * Raspberry Pi IRQ utility
 *
 * Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "rp_irq.h"

static int rp_irq_file_open(const char *path, int flags)
{
    int fd;

    fd = open(path, flags);
    if (fd < 0) {
        fprintf(stderr, "ERROR: open file %s (at %s:%d)\n", path, __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    return fd;
}

static int rp_irq_open_stat(uint8_t pin_no)
{
    int fd;
    char pin_val_path[sizeof("/sys/class/gpio/gpioXX/value")];

    if (pin_no > 27) {
        fprintf(stderr, "ERROR: pin number out of range (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    snprintf(pin_val_path, sizeof(pin_val_path), "/sys/class/gpio/gpio%d/value", pin_no);

    return rp_irq_file_open(pin_val_path, O_RDONLY);
}

static rp_irq_stat_t rp_irq_read_stat(int fd)
{
    char stat;
    int ret;

    ret = read(fd, &stat, 1);
    if (ret != 1) {
        fprintf(stderr, "ERROR: read (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    switch (stat) {
    case '0': return RP_IRQ_STAT_L;
    case '1': return RP_IRQ_STAT_H;
    default:
        fprintf(stderr, "ERROR: invalid stat (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
}


void rp_irq_enable(uint8_t pin_no, rp_irq_edge_mode_t mode)
{
    int fd;
    char pin_no_str[sizeof("XX")];
    char pin_dir_path[sizeof("/sys/class/gpio/gpioXX/direction")];
    char pin_edge_path[sizeof("/sys/class/gpio/gpioXX/edge")];

    if (pin_no > 27) {
        fprintf(stderr, "ERROR: pin number out of range (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    snprintf(pin_no_str, sizeof(pin_no_str), "%d", pin_no);
    snprintf(pin_dir_path, sizeof(pin_dir_path), "/sys/class/gpio/gpio%d/direction", pin_no);
    snprintf(pin_edge_path, sizeof(pin_edge_path), "/sys/class/gpio/gpio%d/edge", pin_no);

    fd = rp_irq_file_open("/sys/class/gpio/export", O_WRONLY);
    write(fd, pin_no_str, strlen(pin_no_str));
    close(fd);

    fd = rp_irq_file_open(pin_dir_path, O_WRONLY);
    write(fd, "in", 2);
    close(fd);

    fd = rp_irq_file_open(pin_edge_path, O_WRONLY);
    switch (mode) {
    case RP_IRQ_EDGE_FALLING:
        write(fd, "falling", 7);
        break;
    case RP_IRQ_EDGE_RISING:
        write(fd, "rising", 6);
        break;
    case RP_IRQ_EDGE_BOTH:
        write(fd, "both", 4);
        break;
    default:
        fprintf(stderr, "ERROR: invalid mode (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void rp_irq_disable(uint8_t pin_no)
{
    int fd;
    char pin_no_str[sizeof("XX")];

    if (pin_no > 27) {
        fprintf(stderr, "ERROR: pin number out of range (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    snprintf(pin_no_str, sizeof(pin_no_str), "%d", pin_no);

    fd = rp_irq_file_open("/sys/class/gpio/unexport", O_WRONLY);
    write(fd, pin_no_str, strlen(pin_no_str));
    close(fd);
}

void rp_irq_init(uint8_t pin_no, rp_irq_handle_t *handle)
{
    int fd;

    fd = rp_irq_open_stat(pin_no);

    handle->fd = fd;
    handle->pfd.fd = fd;
    handle->pfd.events = POLLPRI;
}

rp_irq_stat_t rp_irq_wait(rp_irq_handle_t *handle, uint32_t wait_msec)
{
    char stat;
    int ret;

    lseek(handle->fd, 0, SEEK_SET);
    
    ret = poll(&(handle->pfd), 1, wait_msec);
    if (ret == 0) {
        return RP_IRQ_STAT_TIMEOUT;
    }

    return rp_irq_read_stat(handle->fd);
}

rp_irq_stat_t rp_irq_get_state(uint8_t pin_no)
{
    int fd;

    fd = rp_irq_open_stat(pin_no);
    return rp_irq_read_stat(handle->fd);
}

// Local Variables:
// coding: shift_jis-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
