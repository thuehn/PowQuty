CC:=gcc
LD=ld
CFLAGS+=-g -Wall
CFLAGS+=-I./lib/ -I. -I./include/ -I/usr/local/include/uci

#CFLAGS+=-DDEBUG_PRINT

ifeq ($(ARCH),aarch64)
LD_LIBS+= ./lib/libpqlib_aarch64_cortex-a53.a
endif

ifeq ($(ARCH),i386)
LD_LIBS+= ./lib/libpqlib_i386_pentium4.a
endif

ifeq ($(ARCH),x86_64)
LD_LIBS+= ./lib/libpqlib_x86_64.a
#LD_LIBS+= ./lib/libpqlib.a
endif

ifeq ($(ARCH),powerpc)
LD_LIBS+= ./lib/libpqlib_powerpc_8540.a
endif

ifeq ($(ARCH),mips)
  ifeq ($(CC_TYPE),"24kc")
    LD_LIBS+= ./lib/libpqlib_mips_24kc.a
  else ifeq ($(CC_TYPE),"34kc")
    LD_LIBS+= ./lib/libpqlib_mips_34kc.a
  endif
endif


#LD_LIBS+= ./lib/libpqlib.a		## this did work 
#LD_LIBS+=-lpqlib.a
LD_LIBS+=-lm
LD_LIBS+=-lconfig
LD_LIBS+=-luci

OBJS= src/emulator.o \
        src/file_checks.o \
	src/helper.o \
	src/config.o \
	src/uci_config.o \
	src/event_handling.o \
	src/file_handling.o \
	src/retrieval.o \
	src/calculation.o \
	src/raw_dump.o \
	src/main.o

ifeq ($(MQTT),y)
  CFLAGS+=-DMQTT
  LD_LIBS+=-lmosquitto
  OBJS+=src/mqtt.o
else
endif

ifeq ($(SLACK),y)
  CFLAGS+=-lcurl -DSLACK
  LD_LIBS+=-lcurl
  OBJS+=src/libwebslack.o
else
endif

LD_LIBS+=-lpthread
#LD_LIBS+=-lusb
#LD_LIBS+=-lusb-1.0

#LDFLAGS+=-static 
#LD_PATH+=-L../
#LD_PATH+=-L/usr/local/lib/


MQTT_OBJS= src/mqtt.o \
	test/mqtt_test.o 

USB_TEST_OBJS= test/usb_test.o

VSERIAL_TEST_OBJS= test/vserial_test.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

powqutyd: $(OBJS) 
#	@echo ""
#	@echo ""
#	@echo ""
#	@echo "############################## Building powqutyd #########################"
#	@echo ""
#	@echo ""
#	@echo ""
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
