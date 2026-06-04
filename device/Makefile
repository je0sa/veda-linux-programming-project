TARGET = ../bin/libdevice.so
CROSS = aarch64-linux-gnu-gcc
CFLAGS = -fPIC -Wall -I.
LDFLAGS = -shared
LIBS = -lwiringPi -lcrypt -lpthread

SRCS = src/setup.c src/led.c src/cds.c src/seg.c src/buz.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p ../bin
	$(CROSS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CROSS) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean