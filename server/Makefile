TARGET = ../bin/server
CROSS = aarch64-linux-gnu-gcc

CFLAGS = -Wall -I../device
LIBS = -ldl -lpthread -lwiringPi -lcrypt

all: $(TARGET)

$(TARGET): src/server.c
	@mkdir -p ../bin
	$(CROSS) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean