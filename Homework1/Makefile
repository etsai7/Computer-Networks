CC = gcc
CFLAGS = -g -c -ansi

all: iPerfer

iPerfer: 	iPerfer.o
		$(CC) -o iPerfer iPerfer.o

clean:
		rm -f *.o

%.o:	%.c
		$(CC) $(CFLAGS) $*.c


