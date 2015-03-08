# Raspberry Pi SC2004C Utility
#
# Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, version 2.

DRV     	:= libina226prc_ctrl.a
DRV_SRCS  	:= ina226prc_ctrl.c

TEST_APP	:= ina226prc.test
TEST_SRC	:= test.c

LIB_PATH        := ../lib
DEP_LIB		:= $(LIB_PATH)/i2c/librp_i2c.a

INCLUDES        := $(patsubst %,-I %, $(dir $(DEP_LIB)))
CFLAGS 		:= -Wall -Wextra -std=c99 -pipe -g -O3 $(INCLUDES)
LDFLAGS 	:= -Wall

lib: $(DRV)
test: $(TEST_APP)

$(DEP_LIB):
	make -C $(LIB_PATH)

$(DRV): $(subst .c,.o,$(DRV_SRCS)) 
	ar r $@ $^

$(TEST_APP): $(TEST_SRC) $(DRV) $(DEP_LIB)
	gcc -o $@ $< $(CFLAGS) $(LDFLAGS) -L. $(patsubst lib%.a,-l%,$(DRV) $(DEP_LIB))

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $< 

clean:
	make -C $(LIB_PATH) $@
	rm -f *.o *.a *~ $(TEST_APP)

.SUFFIXES: .c .h .o .a .test
.PHONY: all lib test clean

# Local Variables:
# coding: utf-8-unix
# mode: makefile
# End: