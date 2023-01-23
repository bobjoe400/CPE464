# Example makefile for CPE464 program 1
#
# 

CC = gcc
CFLAGS = -Wall -std=c99 -pedantic -g -D_GNU_SOURCE
MAIN = trace
OBJS = trace.o utility.o checksum.o

all : $(MAIN)

$(MAIN) : $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS) -lpcap

trace.o : trace.c utility.h checksum.h
	$(CC) $(CFLAGS) -c trace.c -lpcap

checksum.o: checksum.c checksum.h
	$(CC) $(CFLAGS) -c checksum.c -lpcap

utility.o: utility.c utility.h 
	$(CC) $(CFLAGS) -c utility.c -lpcap

clean :
	rm -rf *.o core trace