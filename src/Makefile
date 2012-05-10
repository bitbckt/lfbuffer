PROG = buffer
OPTIMIZATION ?= -O2
CFLAGS ?= -pedantic -pipe $(OPTIMIZATION) -Wall -W
CCLINK ?= -pthread
DEBUG  ?= -g -rdynamic -ggdb3

CCOPT = $(CFLAGS) $(CCLINK)

OBJ = main.o buffer.o

all: $(PROG)

buffer.o: buffer.cpp buffer.hpp
main.o: main.cpp buffer.hpp

$(PROG): $(OBJ)
	g++ -o $(PROG) $(CCOPT) $(DEBUG) $(OBJ)

.cpp.o:
	g++ -c $(CFLAGS) $(DEBUG) $<

noopt:
	$(MAKE) OPTIMIZATION=""

dep:
	g++ -MM *.cpp

clean:
	rm -rf $(PROG) *.o
