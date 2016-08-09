CC:=gcc
LD=ld
CFLAGS+=-g -Wall
CFLAGS+=-I./lib/ -I. -I./include/
#CFLAGS+=-DDEBUG_PRINT

LD_LIBS+= ./lib/libpqlib.a		## this did work 
#LD_LIBS+=-lpqlib.a
LD_LIBS+=-lm
LD_LIBS+=-lmosquitto
LD_LIBS+=-lpthread
#LD_LIBS+=-lusb
LD_LIBS+=-lusb-1.0

#LDFLAGS+=-static 
#LD_PATH+=-L../
#LD_PATH+=-L.

OBJS= src/emulator.o \
	src/mqtt.o \
	src/main.o

MQTT_OBJS= src/mqtt.o \
	test/mqtt_test.o 

USB_TEST_OBJS= test/usb_test.o

VSERIAL_TEST_OBJS= test/vserial_test.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

powqutyd: $(OBJS) 
	$(CC) -o $@ $^ $(CFLAGS) $(LD_PATH) $(LDFLAGS) $(LD_LIBS)
#	$(LD) $(LD_PATH) $(LDFLAGS) -o $@ $(LD_LIBS)

mqtt_test: $(MQTT_OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LD_PATH) $(LDFLAGS) $(LD_LIBS)

usb_test: $(USB_TEST_OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LD_PATH) $(LDFLAGS) $(LD_LIBS)

vserial_test: $(VSERIAL_TEST_OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LD_PATH) $(LDFLAGS) $(LD_LIBS)



TARGETS=powqutyd

TEST_TARGETS=mqtt_test usb_test vserial_test

all: $(TARGETS)

clean:
	rm -f *.o src/*.o test/*.o
	rm -f $(TARGETS)
	rm -f $(TEST_TARGETS)
	rm -f *.txt
