# Makefile prototype for configure
# Copyright 2004 Phil Karn, KA9Q
# May be used under the terms of the GNU Lesser General Public License (LGPL)

CC=gcc

CFLAGS=-g -O0 -I./include -Wall

.PHONY: all

all: bin | rstest.elf

bin:
	mkdir bin

rstest.elf: bin/rstest.o bin/alpha_to.o bin/genpoly.o bin/index_of.o
	gcc $(CFLAGS) -o $@ $^

bin/rstest.o: rstest.c include/reed_solomon.h
	gcc $(CFLAGS) -c $< -o $@

bin/alpha_to.o: src/alpha_to.c include/reed_solomon.h
	gcc $(CFLAGS) -c $< -o $@

bin/genpoly.o: src/genpoly.c include/reed_solomon.h
	gcc $(CFLAGS) -c $< -o $@

bin/index_of.o: src/index_of.c include/reed_solomon.h
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -f bin/*.o rstest.elf

