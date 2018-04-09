CC = gcc
CFLAGS = -g -c

all: miProxy nameserver

miProxy: 	miProxy.o
		$(CC) -o miProxy miProxy.o

nameserver: 	nameserver.o
		$(CC) -o nameserver nameserver.o

clean:
		rm -f *.o
		rm -f miProxy
		rm -f nameserver

%.o:	%.c
		$(CC) $(CFLAGS) $*.c


