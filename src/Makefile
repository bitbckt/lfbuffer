PROG = buffer
OPTIMIZATION ?= -O2
CFLAGS ?= -std=c99 -pedantic -pipe $(OPTIMIZATION) -Wall -W -D_THREAD_SAFE
CCLINK ?= -pthread
DEBUG  ?= -g -rdynamic -ggdb3

CCOPT = $(CFLAGS) $(CCLINK)

OBJ = buffer.o main.o

all: $(PROG)

buffer.o: buffer.c buffer.h
main.o: main.c buffer.h

$(PROG): $(OBJ)
	$(CC) -o $(PROG) $(CCOPT) $(DEBUG) $(OBJ)

.c.o:
	$(CC) -c $(CFLAGS) $(DEBUG) $<

noopt:
	$(MAKE) OPTIMIZATION=""

dep:
	$(CC) -MM *.c

clean:
	rm -rf $(PROG) *.o
