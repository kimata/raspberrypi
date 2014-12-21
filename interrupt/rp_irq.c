/*
 * Raspberry Pi IRQ utility
 *
 * Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 */

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "rp_irq.h"

#define GPIO_PIN_COUNT          28
#define WATCH_INTERVAL_MSEC     500

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
    char pin_val_path[sizeof("/sys/class/gpio/gpioXX/value")];

    if (pin_no >= GPIO_PIN_COUNT) {
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

    if (pin_no >= GPIO_PIN_COUNT) {
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

    if (pin_no >= GPIO_PIN_COUNT) {
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
    int ret;

    lseek(handle->fd, 0, SEEK_SET);
    
    ret = poll(&(handle->pfd), 1, wait_msec);
    if (ret == 0) {
        return RP_IRQ_STAT_TIMEOUT;
    }

    return rp_irq_read_stat(handle->fd);
}

rp_irq_stat_t rp_irq_get_stat(uint8_t pin_no)
{
    int fd;

    fd = rp_irq_open_stat(pin_no);
    rp_irq_stat_t stat = rp_irq_read_stat(fd);
    close(fd);
    
    return stat;
}

void rp_irq_watch_stat(uint8_t pin_no, pid_t parent)
{
    struct timespec check_interval = { 0, 1 * 1E6 }; // 1ms 
    static uint8_t stat_cur_count = 0;
    rp_irq_handle_t handle;
    rp_irq_stat_t stat_prev;

    rp_irq_init(pin_no, &handle);    
 WATCH_START:
    while (1) {
        stat_prev = rp_irq_wait(&handle, TIMEOUT_MSEC);
        if (stat_prev == RP_IRQ_STAT_TIMEOUT) {
            continue;
        }
        while (1) {
            rp_irq_stat_t stat = rp_irq_get_stat(pin_no);
            if (stat == stat_prev) {
                stat_cur_count++;
                if (stat_cur_count == NOTIFY_THRESHOLD) {
                    g_stat = stat;
                    if (kill(parent, SIGUSR1) != 0) {
                        fprintf(stderr, "ERROR: kill (at %s:%d)\n", __FILE__, __LINE__);
                        exit(EXIT_FAILURE);
                    }
                    stat_cur_count = 0;
                    goto WATCH_START;
                }
            } else {
                stat_prev = stat;
                stat_cur_count = 0;
            }
            nanosleep(&check_interval, NULL);
        }
    }

    return EXIT_SUCCESS;
}

// Local Variables:
// coding: shift_jis-unix
// mode: c
// c-basic-offset: 4
// tab-width: 4
// indent-tabs-mode: nil
// End:
