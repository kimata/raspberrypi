# Raspberry Pi I2C Driver
#
# Copyright (C) 2015 Tetsuya Kimata <kimata@green-rabbit.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, version 2.

LIB     	:= librp_i2c.a
LIB_SRCS  	:= rp_i2c.c

TEST_APP	:= rp_i2c.test
TEST_SRC	:= test.c

DEP_LIB_PATH    := 
DEP_LIB		:= 

INCLUDES        := $(patsubst %,-I %, $(dir $(DEP_LIB)))
CFLAGS 		:= -Wall -Wextra -std=c99 -O3 -fPIC -pipe
LDFLAGS 	:= -Wall

lib: $(LIB)
test: $(TEST_APP)

$(LIB): $(subst .c,.o,$(LIB_SRCS)) 
	ar r $@ $^

$(TEST_APP): $(TEST_SRC) $(LIB) $(DEP_LIBS) $(TEST_LIB)
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