# Raspberry Pi SC2004C Utility
#
# Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, version 2.

LIB     	:= libsc2004c_ctrl.a
LIB_SRCS  	:= sc2004c_ctrl.c rp_gpio.c

TEST_APP	:= sc2004cs.test
TEST_SRC	:= test.c

CFLAGS 		:= -Wall -Wextra -std=c99 -O3 -fPIC -pipe
LDFLAGS 	:= -Wall

lib: $(LIB)
test: $(TEST_APP)

$(LIB): $(subst .c,.o,$(LIB_SRCS)) 
	ar r $@ $^

$(TEST_APP): $(TEST_SRC) $(LIB)
	gcc -o $@ $< $(CFLAGS) $(LDFLAGS) -L. -l$(patsubst lib%.a,%,$(LIB))

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $< 

clean:
	rm -f *.o *.a *~ $(TEST_APP)

.SUFFIXES: .c .h .o .a .test
.PHONY: all lib test clean

# Local Variables:
# coding: utf-8-unix
# mode: makefile
# End: