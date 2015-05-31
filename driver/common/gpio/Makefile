# Raspberry Pi I2C Driver
#
# Copyright (C) 2015 Tetsuya Kimata <kimata@green-rabbit.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, version 2.

LIB     	:= librp_gpio.a
LIB_SRCS  	:= rp_gpio.c

CFLAGS 		:= -Wall -Wextra -std=c99 -O3 -fPIC -pipe
LDFLAGS 	:= -Wall

lib: $(LIB)

$(LIB): $(subst .c,.o,$(LIB_SRCS)) 
	ar r $@ $^

%.o: %.c
	gcc $(CFLAGS) -c $(INCLUDES) -o $@ $<

clean:
	rm -f *.o *.a *~ $(TEST_APP)

.SUFFIXES: .c .h .o .a .test
.PHONY: all lib test clean

# Local Variables:
# coding: utf-8-unix
# mode: makefile
# End: