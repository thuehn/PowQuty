CC:=gcc
LD=ld
CFLAGS+=-g -Wall
CFLAGS+=-I./lib/ -I. -I./include/
#CFLAGS+=-DDEBUG_PRINT

LD_LIBS+= ./lib/libpqlib.a		## this did work 
#LD_LIBS+=-lpqlib.a
LD_LIBS+=-lm
#LDFLAGS+=-static 
#LD_PATH+=-L../
#LD_PATH+=-L.
OBJS= src/emulator.o \
	src/main.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

powqutyd: $(OBJS) 
#	$(LD) $(LD_PATH) $(LDFLAGS) -o $@ $(LD_LIBS)
	$(CC) -o $@ $^ $(CFLAGS) $(LD_PATH) $(LDFLAGS) $(LD_LIBS)


TARGETS=powqutyd

all: $(TARGETS)

clean:
	rm -f *.o src/*.o 
	rm -f $(TARGETS)
	rm -f *.txt
