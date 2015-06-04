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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <float.h>
#include <sys/types.h>

#include "rp_gpio.h"
#include "rp_irq.h"

#include "ina226prc_ctrl.h"
#include "sc2004c_ctrl.h"

// NOTE: device address
#define DEV_ADDR_INA226PRC  0x45

// NOTE: pin assign
#define SWITCH_PIN_NO 		13

#define SWITCH_LED_PIN_NO 	19

#define LCD_RS_PIN_NO 		21
#define LCD_EN_PIN_NO 		22
#define LCD_D4_PIN_NO 		23
#define LCD_D5_PIN_NO 		24
#define LCD_D6_PIN_NO 		25
#define LCD_D7_PIN_NO 		26

#define LCD_LINE_LEN 	    20

#define DISP_INTERVAL       1000

static volatile uint8_t g_sw_change = 0;
static uint8_t g_log_enable = 0;

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

static int init_dev()
{
    int ret = 0;

    ret |= init_sc2004cs();
    ret |= init_ina226prc();

    return ret;
}

static void calc_stat(ina226prc_value_t *measure_hist, uint32_t hist_size, 
                      ina226prc_value_t *measure_min, ina226prc_value_t *measure_max,
                      ina226prc_value_t *measure_ave)
{
    for (uint32_t i = 0; i < hist_size; i++) {
        if (measure_min->voltage > measure_hist[i].voltage) {
            measure_min->voltage = measure_hist[i].voltage;
        }
        if (measure_min->current > measure_hist[i].current) {
            measure_min->current = measure_hist[i].current;
        }
        if (measure_min->power > measure_hist[i].power) {
            measure_min->power = measure_hist[i].power;
        }
        if (measure_max->voltage < measure_hist[i].voltage) {
            measure_max->voltage = measure_hist[i].voltage;
        }
        if (measure_max->current < measure_hist[i].current) {
            measure_max->current = measure_hist[i].current;
        }
        if (measure_max->power < measure_hist[i].power) {
            measure_max->power = measure_hist[i].power;
        }
        measure_ave->voltage += measure_hist[i].voltage;
        measure_ave->current += measure_hist[i].current;
        measure_ave->power += measure_hist[i].power;
    }
    measure_ave->voltage /= hist_size;
    measure_ave->current /= hist_size;
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
}

static int disp_stat(ina226prc_value_t *measure_hist, uint32_t hist_size)
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

    snprintf(lcd_line_buf, sizeof(lcd_line_buf), "I[A]: %2.2f %2.2f %2.2f",
             measure_ave.current, measure_max.current, measure_min.current);
    sc2004c_set_line(2);
    sc2004c_print(lcd_line_buf);

    snprintf(lcd_line_buf, sizeof(lcd_line_buf), "V[V]: %2.2f %2.2f %2.2f",
             measure_ave.voltage, measure_max.voltage, measure_min.voltage);
    sc2004c_set_line(3);
    sc2004c_print(lcd_line_buf);

    indi_idx++;
}

static int sense_main()
{
    rp_gpio_level_t sw_level;
    ina226prc_value_t measure_hist[DISP_INTERVAL];

    signal(SIGUSR1, sigusr1_handler);    

    rp_gpio_set_mode(SWITCH_LED_PIN_NO, RP_GPIO_OUTPUT);
    rp_gpio_get_input(SWITCH_PIN_NO, &sw_level);

    init_dev();
    disp_greeting();

    while (1) {
        int i;

        if (g_sw_change == 1) {
            rp_gpio_get_input(SWITCH_PIN_NO, &sw_level); // ON: L, OFF: H
            rp_gpio_set_output(SWITCH_LED_PIN_NO, sw_level);
            g_sw_change = 0;
        }

        i = 0;
        while (i < DISP_INTERVAL) {
            ina226prc_value_t measured_val;

            if (ina226prc_sense(DEV_ADDR_INA226PRC, &measured_val) != 0) {
                continue;
            }
            measure_hist[i++] = measured_val;
        }
        disp_stat(measure_hist, DISP_INTERVAL);
    }

    return EXIT_SUCCESS;
}

int main(int argc,char *argv[])
{
    uint8_t pin_no;

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
        fprintf(stderr, "B\n");
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
