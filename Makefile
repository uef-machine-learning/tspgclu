CC=gcc
GPP=g++

DISABLEWARN = -Wno-unused-but-set-variable -Wno-unused-result -Wno-unused-variable
ARCHFLAGS =  -march=x86-64

CXXFLAGS +=  -O3 $(ARCHFLAGS)  -Wall -I.  -std=c++11 
DEBUGFLAGS +=  -O0 $(ARCHFLAGS)  -Wall -I.  -std=c++11 -g -DDEBUG
# DEBUGFLAGS +=  -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
 
LIBS = -lpthread

CFLAGS = -O3

DEPS = *.cpp *.h *hpp
OBJS=argtable3.o
.PHONY:	all clean

PROGS= tspg

all: tspg

#argtable should support compiling with g++, but there was an error message.
argtable3.o:
	$(CC) -c $(CFLAGS) contrib/argtable3.c

tspg: $(DEPS) $(OBJS)
	$(GPP) $(CXXFLAGS) $(DISABLEWARN) main.cpp $(LIBS) $(OBJS) -o tspg

tspg_debug: $(DEPS) $(OBJS)
	$(GPP) $(DEBUGFLAGS) $(DISABLEWARN) main.cpp $(LIBS) $(OBJS) -o tspg_debug

clean:
	rm -f $(PROGS) *.o


