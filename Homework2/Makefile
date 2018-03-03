CC = gcc
CFLAGS = -g -c -ansi

all: miProxy

miProxy: 	miProxy.o
		$(CC) -o miProxy miProxy.o

clean:
		rm -f *.o

%.o:	%.c
		$(CC) $(CFLAGS) $*.c


