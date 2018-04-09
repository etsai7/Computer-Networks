CC = gcc
CFLAGS = -g -c

all: miProxy

miProxy: 	miProxy.o
		$(CC) -o miProxy miProxy.o

clean:
		rm -f *.o
		rm -f miProxy

%.o:	%.c
		$(CC) $(CFLAGS) $*.c


