CC=gcc -Wall #-g

CFLAGS=-O3 -I. -I/usr/include -I/usr/local/include

LIBS=

LDFLAGS=-lpng -lm -lpthread

PROG=sky-cam

OBJS = sky-cam.o file_io.o tools.o init.o

all : $(PROG)

$(PROG) : $(OBJS)
	$(CC) $(OBJS) -o $(PROG) $(CFLAGS) $(LIBS) $(LDFLAGS)

sky-cam.o : sky-cam.c
	$(CC) -o sky-cam.o -c sky-cam.c $(CFLAGS)

file_io.o : file_io.c
	$(CC) -o file_io.o -c file_io.c $(CFLAGS)

tools.o : tools.c
	$(CC) -o tools.o -c tools.c $(CFLAGS)

init.o : init.c
	$(CC) -o init.o -c init.c $(CFLAGS)

clean:
	rm -f $(OBJS) $(PROG)

