# Raspberry Pi SC2004C Utility
#
# Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, version 2.

LIB     	:= libina226prc_ctrl.a
LIB_SRCS        := ina226prc_ctrl.c

TEST_APP	:= ina226prc.test
TEST_SRC	:= test.c
TEST_LIB_PATH   := 
TEST_LIBS	:= 

DEP_LIB_PATH    := ../../common
DEP_LIBS	:= $(DEP_LIB_PATH)/lib/librp_lib.a

ifdef I2C_IOCTL
DEP_LIBS        := $(DEP_LIBS) $(DEP_LIB_PATH)/i2c_ioctl/librp_i2c_ioctl.a
else
DEP_LIBS        := $(DEP_LIBS) $(DEP_LIB_PATH)/i2c/librp_i2c.a
endif

INCLUDES        := $(patsubst %,-I %, $(dir $(DEP_LIBS)))
CFLAGS 		:= -Wall -Wextra -std=c99 -pipe -g -O3
LDFLAGS 	:= -Wall

ifdef I2C_IOCTL
CFLAGS          := $(CFLAGS) -DI2C_IOCTL
endif

lib: $(LIB)
test: $(TEST_APP)

$(LIB): $(subst .c,.o,$(LIB_SRCS)) 
	ar r $@ $^

$(TEST_APP): $(TEST_SRC) $(LIB) $(DEP_LIBS) $(TEST_LIBS)
	@LIB_DIRS="$(dir $(DEP_LIBS) $(TEST_LIBS))"; \
	for i in $$LIB_DIRS; do \
	    make -C $$i; \
	done
	gcc -o $@ $< $(CFLAGS) $(LDFLAGS) -L. $(patsubst lib%.a,-l%,$(LIB)) $(patsubst %,-L %,$(dir $(DEP_LIBS) $(TEST_LIBS))) $(patsubst lib%.a,-l%, $(notdir $(DEP_LIBS) $(TEST_LIBS)))

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