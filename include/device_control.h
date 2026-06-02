#ifndef DEVICE_CONTROL_H
#define DEVICE_CONTROL_H

// 장치 초기화 및 해제
int init_devices(void);
void release_devices(void);

// 1. LED 제어 (mode: 0-Off, 1-최저, 2-중간, 3-최대)
int control_led(int mode);

// 2. 부저 제어 (state: 0-Off, 1-On)
int control_buzzer(int state);

// 3. 조도센서 값 읽기
int read_light_sensor(void);

// 4. 7세그먼트 숫자 표시
int display_7segment(int number);

#endif // DEVICE_CONTROL_H