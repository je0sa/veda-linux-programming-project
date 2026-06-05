#include <stdio.h>
#include <wiringPi.h>
#include <softTone.h>
#include <softPwm.h>
#include "device.h"

#define LED  1    // BCM GPIO18
#define SPKR 6    // BCM GPIO25

int pin_select[7] = {21, 22, 23, 24, 25, 27, 28};

/*[하드웨어 공통 초기화]*/ 
void device_init()
{   
    /*중복 실행을 막기 위한 정적 플래그(is_global_init) 사용*/
    static int is_global_init = 0;
    if (is_global_init) return;

    wiringPiSetup();    // wiringPi 초기화 
    softPwmCreate(LED, 0, 100); // LED 핀 초기화
    softToneCreate(SPKR);   // 부저(스피커) 채널 개설 및 초기화
    
    for(int i = 0; i < 7; i++){ // 7세그먼트 핀들 일괄 출력 모드(OUTPUT) 지정
        pinMode(pin_select[i], OUTPUT);
    }

    is_global_init = 1;
}