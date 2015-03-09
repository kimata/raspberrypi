# Raspberry Pi Driver
#
# Copyright (C) 2015 Tetsuya Kimata <kimata@green-rabbit.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, version 2.

LIBS     	:= lib/librp_lib.a i2c/librp_i2c.a

all: $(LIBS)

%.a:
	make -C $(dir $@) lib

clean:
	for i in $(dir $(LIBS)); \
	do \
	    make -C $$i clean; \
	done

.SUFFIXES: .a
.PHONY: clean

# Local Variables:
# coding: utf-8-unix
# mode: makefile
# End: