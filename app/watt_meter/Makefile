# Raspberry Pi Watt Meter
#
# Copyright (C) 2015 Tetsuya Kimata <kimata@green-rabbit.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, version 2.

APP	        := watt_meter
APP_SRCS        := watt_meter.c

DEP_LIB_PATH    := ../../driver
DEP_LIBS	:= $(DEP_LIB_PATH)/dev/sc2004cs/libsc2004c_ctrl.a \
		   $(DEP_LIB_PATH)/dev/ina226prc/libina226prc_ctrl.a\
		   $(DEP_LIB_PATH)/common/i2c/librp_i2c.a $(DEP_LIB_PATH)/common/i2c_ioctl/librp_i2c_ioctl.a \
		   $(DEP_LIB_PATH)/common/interrupt/librp_irq.a $(DEP_LIB_PATH)/common/gpio/librp_gpio.a

INCLUDES        := $(patsubst %,-I %, $(dir $(DEP_LIBS)))
CFLAGS 		:= -Wall -Wextra -std=c99 -O3 -fPIC -pipe
LDFLAGS 	:= -Wall

app: $(APP)

$(APP): $(subst .c,.o,$(APP_SRCS)) $(DEP_LIBS)
	gcc -o $@ $< $(CFLAGS) $(LDFLAGS) $(patsubst %,-L %,$(dir $(DEP_LIBS))) $(patsubst lib%.a,-l%,$(notdir $(DEP_LIBS)))

%.o: %.c
	gcc $(CFLAGS) -c $(INCLUDES) -o $@ $< 

clean:
	rm -f *.o *.a *~ $(APP)

.SUFFIXES: .c .h .o .a .test
.PHONY: all lib test clean

# Local Variables:
# coding: utf-8-unix
# mode: makefile
# End: