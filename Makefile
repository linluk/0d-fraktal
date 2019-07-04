
#
# Makefile for 0d-fraktal
#
# Author: Copyright (C) 2019 Lukas Singer
#
# Created: 2019/07/04
#


CC=gcc
CFLAGS=-Wall -std=c11 -lm -pthread -O3
LFLAGS=-Wall -std=c11 -lm -pthread
OBJS=main.o
BIN=0d-fraktal

BIN: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(BIN)

%.o: %.c
	$(CC) -c $(CFLAGS) $<


clean:
	rm $(BIN)
	rm $(OBJS)

