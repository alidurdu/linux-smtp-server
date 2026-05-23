# Use this makefile

CC=gcc
CFLAGS=-Wall

all:
	gcc $(CFLAGS) main.c -o task5

clean:
	rm -f task5
