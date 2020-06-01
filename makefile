# Specify shell to execute recipes
SHELL=/bin/bash

CC=gcc
CFLAGS=-O0 -std=c99 -Wall -W -ggdb3

assembler: assemble.c parser.o tables.o convert.o
	$(CC) $(CFLAGS) -o assemble assemble.c parser.o tables.o convert.o

parser: parser.c parser.h token.h tables.o convert.o
	$(CC) $(CFLAGS) -c parser.c tables.o convert.o

convert: convert.c convert.h tables.o
	$(CC) $(CFLAGS) -c convert.c tables.o

tables: tables.c tables.h
	$(CC) $(CFLAGS) -c tables.c

clean: 
	rm *.o
