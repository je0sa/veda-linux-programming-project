#include <stdio.h>
#include "../include/device_control.h"

int init_devices(void) {
    printf("[Device Lib] 하드웨어 장치 초기화 완료\n");
    return 0;
}

void release_devices(void) {
    printf("[Device Lib] 하드웨어 장치 자원 해제\n");
}

int control_led(int mode) {
    printf("[Device Lib] LED 제어 명령 수신: 단계 %d\n", mode);
    return 0;
}

int control_buzzer(int state) {
    printf("[Device Lib] 부저 제어 명령 수신: %s\n", state ? "ON" : "OFF");
    return 0;
}

int read_light_sensor(void) {
    printf("[Device Lib] 조도 센서 값 읽는 중...\n");
    return 500; // 가상의 센서 데이터 반환
}

int display_7segment(int number) {
    printf("[Device Lib] 7세그먼트 표시: %d\n", number);
    return 0;
}