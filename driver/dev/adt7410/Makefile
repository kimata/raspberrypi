# Raspberry Pi ADT7410 Driver
#
# Copyright (C) 2014 Tetsuya Kimata <kimata@green-rabbit.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, version 2.

LIB     	:= libadt7410_ctrl.a
LIB_SRCS        := adt7410_ctrl.c

TEST_APP	:= adt7410.test
TEST_SRC	:= test.c
TEST_LIB_PATH   := 
TEST_LIB	:= 

DEP_LIB_PATH    := ../../common
DEP_LIB		:= $(DEP_LIB_PATH)/i2c/librp_i2c.a


INCLUDES        := $(patsubst %,-I %, $(dir $(DEP_LIB)))
CFLAGS 		:= -Wall -Wextra -std=c99 -pipe -g -O3
LDFLAGS 	:= -Wall


TEST_LIBS  	:= rp_i2c.c adt7410_ctrl.c

CFLAGS 		:= -Wall -Wextra -std=c99 -pipe -g -O0
LDFLAGS 	:= -Wall

lib: $(LIB)
test: $(TEST_APP)

$(LIB): $(subst .c,.o,$(LIB_SRCS)) 
	ar r $@ $^

$(TEST_APP): $(TEST_SRC) $(LIB) $(TEST_LIB)
	gcc -o $@ $< $(CFLAGS) $(LDFLAGS) -L. $(patsubst -llib%.a,%,$(LIB)) $(patsubst %,-L %,$(dir $(DEP_LIB) $(TEST_LIB))) $(patsubst -llib%.a,%,$(DEP_LIB) $(TEST_LIB))

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