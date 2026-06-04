#include <stdio.h>
#include <wiringPi.h>
#include <dlfcn.h>
#include "device.h"

void seg_function(int num)
{
    int a, b;

    int sevenseg[10][7] = {
        {0, 0, 0, 0, 0, 0, 1}, // 0
        {1, 0, 0, 1, 1, 1, 1}, // 1
        {0, 0, 1, 0, 0, 1, 0}, // 2
        {0, 0, 0, 0, 1, 1, 0}, // 3
        {1, 0, 0, 1, 1, 0, 0}, // 4
        {0, 1, 0, 0, 1, 0, 0}, // 5
        {0, 1, 0, 0, 0, 0, 0}, // 6
        {0, 0, 0, 1, 1, 1, 1}, // 7
        {0, 0, 0, 0, 0, 0, 0}, // 8
        {0, 0, 0, 0, 1, 0, 0}  // 9
    };

    device_init();

    for(a = num; a >= 0; a--)
    {
        for(b = 0; b < 7; b++) 
        {
            digitalWrite(pin_select[b], sevenseg[a][b]);
        }

        if (a == 0) 
        {
            buz_function("ON");
        }

        delay(1000); // 1초 대기
    }
}