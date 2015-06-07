/*
 * watt_meter
 *
 * Copyright (C) 2015 Tetsuya Kimata <kimata@green-rabbit.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <float.h>
#include <time.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "rp_gpio.h"
#include "rp_irq.h"

#include "ina226prc_ctrl.h"
#include "adt7410_ctrl.h"
#include "sc2004c_ctrl.h"

// NOTE: device address
#define DEV_ADDR_INA226PRC  0x45
#define DEV_ADDR_ADT7410    0x49

// NOTE: pin assign
#define SWITCH_PIN_NO       13

#define SWITCH_LED_PIN_NO   19

#define LCD_RS_PIN_NO       21
#define LCD_EN_PIN_NO       22
#define LCD_D4_PIN_NO       23
#define LCD_D5_PIN_NO       24
#define LCD_D6_PIN_NO       25
#define LCD_D7_PIN_NO       26

#define SENSE_RETRY         5

#define LCD_LINE_LEN        20

#define DISP_INTERVAL       1000
#define LOG_DIR_PATH        "/var/www/watt_meter/log"

static volatile uint8_t g_sw_change = 0;
static uint8_t g_log_enable = 0;

typedef struct log_info {
    uint32_t log_no;
    char *log_file_path;
    FILE *log_file;
} log_info_t;

static void sigusr1_handler(int sig)
{
    if (sig != SIGUSR1) {
        return;
    }
    g_sw_change = 1;
    signal(SIGUSR1, sigusr1_handler);
}

static int init_sc2004cs()
{
    sc2004c_gpio_assign_t assign;
    
    assign.rs = LCD_RS_PIN_NO;
    assign.en = LCD_EN_PIN_NO;
    assign.d4 = LCD_D4_PIN_NO;
    assign.d5 = LCD_D5_PIN_NO;
    assign.d6 = LCD_D6_PIN_NO;
    assign.d7 = LCD_D7_PIN_NO;

    sc2004c_init(&assign);

    return 0;
}

static int init_ina226prc()
{
    ina226prc_conf_t conf;

    conf.shunt_mohm		= 25; // 25mƒ¶
    conf.mode			= INA226PRC_MODE_BOTH_CONT;
    conf.average_number	= INA226PRC_AVG_16;
    conf.bus_conv_time	= INA226PRC_CONV_332US;
    conf.shunt_conv_time= INA226PRC_CONV_332US;

    return ina226prc_init(DEV_ADDR_INA226PRC, &conf);
}

static int init_adt7410() 
{
    return adt7410_init(DEV_ADDR_ADT7410, ADT7410_RES_13BIT);
}

static int init_dev()
{
    int ret = 0;

    ret |= init_sc2004cs();
    ret |= init_ina226prc();
    ret |= init_adt7410();

    return ret;
}

static void init_log_info(log_info_t *log_info)
{
    log_info->log_no = 0;
    log_info->log_file_path = NULL;
    log_info->log_file = NULL;
}

static uint32_t get_log_no()
{
    DIR *dir;
    struct dirent *dp;
    uint32_t log_no;
    char digit[] = "   ";

    log_no = 0;
    if ((dir = opendir(LOG_DIR_PATH)) == NULL) {
        return 0;
    }

    for (dp = readdir(dir); dp != NULL; dp = readdir(dir)) {
        uint32_t n;
        if (strncmp(dp->d_name, "watt_meter_", strlen("watt_meter_")) != 0) {
            continue;
        }
        strncpy(digit, dp->d_name + strlen("watt_meter_"), 3);
        n = (uint32_t)atoi(digit);
        if (n >= log_no) {
            log_no = n + 1;
        }
    }

    return log_no;
}

static void open_log_file(log_info_t *log_info)
{
    char *log_file_path;

    log_info->log_no = get_log_no();

    asprintf(&log_file_path, "%s/watt_meter_%03d.log", LOG_DIR_PATH, log_info->log_no);

    log_info->log_file_path = log_file_path;
    log_info->log_file = fopen(log_file_path, "w");
}

static void calc_stat(ina226prc_value_t *measure_hist, uint32_t hist_size, 
                      ina226prc_value_t *measure_min, ina226prc_value_t *measure_max,
                      ina226prc_value_t *measure_ave)
{
    for (uint32_t i = 0; i < hist_size; i++) {
        /* if (measure_min->voltage > measure_hist[i].voltage) { */
        /*     measure_min->voltage = measure_hist[i].voltage; */
        /* } */
        /* if (measure_min->current > measure_hist[i].current) { */
        /*     measure_min->current = measure_hist[i].current; */
        /* } */
        if (measure_min->power > measure_hist[i].power) {
            measure_min->power = measure_hist[i].power;
        }
        /* if (measure_max->voltage < measure_hist[i].voltage) { */
        /*     measure_max->voltage = measure_hist[i].voltage; */
        /* } */
        /* if (measure_max->current < measure_hist[i].current) { */
        /*     measure_max->current = measure_hist[i].current; */
        /* } */
        if (measure_max->power < measure_hist[i].power) {
            measure_max->power = measure_hist[i].power;
        }
        /* measure_ave->voltage += measure_hist[i].voltage; */
        /* measure_ave->current += measure_hist[i].current; */
        measure_ave->power += measure_hist[i].power;
    }
    /* measure_ave->voltage /= hist_size; */
    /* measure_ave->current /= hist_size; */
    measure_ave->power /= hist_size;
}

static int disp_greeting()
{
   sc2004c_set_line(3);
   sc2004c_print("");
   sc2004c_set_line(2);
   sc2004c_print("");
   sc2004c_set_line(1);
   sc2004c_print("");
   sc2004c_set_line(0);
   sc2004c_print("Initializing...");

   return 0;
}

static int disp_data(ina226prc_value_t *measure_hist, uint32_t hist_size,
                     log_info_t *log_info, struct timespec *log_start,
                     float temp)
{
    static char indi_char[] = { ' ', '.' };
    static uint8_t indi_idx = 0;
    char lcd_line_buf[LCD_LINE_LEN+1];
    ina226prc_value_t measure_min = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
    ina226prc_value_t measure_max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
    ina226prc_value_t measure_ave = {  0, 0, 0 };

    calc_stat(measure_hist, hist_size, &measure_min, &measure_max, &measure_ave);

    snprintf(lcd_line_buf, sizeof(lcd_line_buf), "%c%c%c%c   ave  max  min", 
             indi_char[(indi_idx % 4) == 0], indi_char[(indi_idx % 4) == 1], 
             indi_char[(indi_idx % 4) == 2], indi_char[(indi_idx % 4) == 3]);
    sc2004c_set_line(0);
    sc2004c_print(lcd_line_buf);

    snprintf(lcd_line_buf, sizeof(lcd_line_buf), "P[W]: %2.2f %2.2f %2.2f",
             measure_ave.power, measure_max.power, measure_min.power);
    sc2004c_set_line(1);
    sc2004c_print(lcd_line_buf);

    /* snprintf(lcd_line_buf, sizeof(lcd_line_buf), "I[A]: %2.2f %2.2f %2.2f", */
    /*          measure_ave.current, measure_max.current, measure_min.current); */
    /* sc2004c_set_line(2); */
    /* sc2004c_print(lcd_line_buf); */

    if (temp != -FLT_MAX) {
        snprintf(lcd_line_buf, sizeof(lcd_line_buf), "TEMP: %2.1f %cC      ",
                 temp, 0xDF);
    } else {
        snprintf(lcd_line_buf, sizeof(lcd_line_buf), "TEMP: - (no sensor)");
    }
    sc2004c_set_line(2);
    sc2004c_print(lcd_line_buf);

    if (g_log_enable) {
        struct timespec now;
        uint32_t elapsed_sec;
        uint32_t min;
        uint32_t sec;

        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed_sec = now.tv_sec - log_start->tv_sec;
        min = elapsed_sec / 60;
        sec = elapsed_sec % 60;
        
        snprintf(lcd_line_buf, sizeof(lcd_line_buf), "LOG : no.%03d,%3dm%02ds",
                 log_info->log_no, min, sec);

        if (temp != -FLT_MAX) {
            fprintf(log_info->log_file, "%d, %.2f, %.2f, %.2f, %.1f\n",
                    elapsed_sec, measure_ave.power, measure_max.power, measure_min.power,
                    temp);
        } else {
            fprintf(log_info->log_file, "%d, %.2f, %.2f, %.2f, -\n",
                    elapsed_sec, measure_ave.power, measure_max.power, measure_min.power);
        }
    } else {
        snprintf(lcd_line_buf, sizeof(lcd_line_buf), "LOG : disabled      ");
    }
    sc2004c_set_line(3);
    sc2004c_print(lcd_line_buf);

    indi_idx++;

    return 0;
}

static int sense_main()
{
    
    rp_gpio_level_t sw_level;
    ina226prc_value_t measure_hist[DISP_INTERVAL];
    log_info_t log_info;
    uint32_t is_first;
    struct timespec log_start;
    signal(SIGUSR1, sigusr1_handler);    

    rp_gpio_set_mode(SWITCH_LED_PIN_NO, RP_GPIO_OUTPUT);
    rp_gpio_get_input(SWITCH_PIN_NO, &sw_level);

    init_dev();
    disp_greeting();

    is_first = 1;
    init_log_info(&log_info);
    while (1) {
        uint32_t i;
        uint32_t j;
        float temp;
        float temp_prev;

        i = 0;
        while (i < DISP_INTERVAL) {
            ina226prc_value_t measured_val;

            if (ina226prc_sense(DEV_ADDR_INA226PRC, &measured_val) != 0) {
                continue;
            }
            measure_hist[i++] = measured_val;
        }
        temp_prev = -FLT_MAX;
        temp = -FLT_MAX;
        for (j = 0; j < SENSE_RETRY; j++) {
            if (adt7410_sense(DEV_ADDR_ADT7410, &temp) == 0) {
                if (fabs(temp - temp_prev) < 10) {
                    break;
                }
                temp_prev = temp;
            }
        }

        if (is_first || (g_sw_change == 1)) {
            is_first = 0;
            rp_gpio_get_input(SWITCH_PIN_NO, &sw_level); 
            if (sw_level == RP_GPIO_L) { // ON: L, OFF: H
                g_log_enable = 1;
                open_log_file(&log_info);
                clock_gettime(CLOCK_MONOTONIC, &log_start);
            } else {
                if (log_info.log_file != NULL) {
                    fclose(log_info.log_file);
                    // do something
                    free(log_info.log_file_path);
                }
                g_log_enable = 0;
            }

            rp_gpio_set_output(SWITCH_LED_PIN_NO,
                               (sw_level == RP_GPIO_L) ? RP_GPIO_H : RP_GPIO_L);
            g_sw_change = 0;
        }
        disp_data(measure_hist, DISP_INTERVAL, &log_info, &log_start, temp);
    }

    return EXIT_SUCCESS;
}

int main(int __attribute__((unused)) argc ,char __attribute__((unused)) *argv[])
{
    int pid;

    rp_gpio_init();
    rp_irq_enable(SWITCH_PIN_NO, RP_IRQ_EDGE_BOTH);

    pid = fork();
    switch(pid) {
    case -1:
        fprintf(stderr, "ERROR: fork (at %s:%d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    case 0:
        rp_irq_watch_stat(SWITCH_PIN_NO, getppid());
    default:
        sense_main();
        rp_irq_disable(SWITCH_PIN_NO);
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
