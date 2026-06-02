CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -ldl

# 경로 설정
LIB_DIR = ./lib_dev
SERVER_DIR = ./server
CLIENT_DIR = ./client
TARGET_LIB = libdevice.so

all: $(TARGET_LIB) server_app client_app

# 1. 공유 라이브러리 빌드 (-shared -fPIC)
$(TARGET_LIB): $(LIB_DIR)/device_control.c
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<

# 2. 서버 빌드
server_app: $(SERVER_DIR)/main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# 3. 클라이언트 빌드
client_app: $(CLIENT_DIR)/main.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET_LIB) server_app client_app

.PHONY: all clean