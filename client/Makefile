TARGET = ../bin/client
CC = gcc
CFLAGS = -Wall

all: $(TARGET)

$(TARGET): src/client.c
	@mkdir -p ../bin
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGET)

.PHONY: all clean