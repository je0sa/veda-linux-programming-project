#include <wiringPi.h>
#include <softTone.h>
#include <string.h>
#include "device.h"

#define SPKR 6      /* GPIO25 */
#define TOTAL 32    /* 학교종의 전체 계이름의 수 */

// 컴파일러가 volatile변수를 최적화 과정에서 생략하지 않고, 매번 메모리에서 새로 읽도록 강제함.
volatile int stop_flag = 0; // 부저 종료 플래그 설정

int notes[] =
{
    /* 학교종을 연주하기 위한 계이름 */
    391, 391, 440, 440, 391, 391, 329.63, 329.63,
    391, 391, 329.63, 329.63, 293.66, 293.66, 293.66, 0,
    391, 391, 440, 440, 391, 391, 329.63, 329.63,
    391, 329.63, 293.66, 329.63, 261.63, 261.63, 261.63, 0
};

int musicPlay()
{
    int i;
    for (i = 0; i < TOTAL; ++i)
    {
        if (stop_flag == 1) {
            break; 
        }
        softToneWrite(SPKR, notes[i]);  
        delay(280);                     /* 음의 전체 길이만큼 출력되도록 대기 */
    }
    softToneWrite(SPKR, 0);
    return 0;
}

void buz_function(char *arg)
{
    if(strcmp(arg, "ON") == 0)
    {
        device_init();
    
        stop_flag = 0;   // 부저 종료 플래그 OFF
        musicPlay(); 
    }
    else if(strcmp(arg, "OFF") == 0)
    {
        stop_flag = 1;   // 부저 종료 플래그 ON
        softToneWrite(SPKR, 0);
    }
}
