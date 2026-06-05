#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <softPwm.h>
#include "device.h"

#define LED 1   // WiringPi pin 1 = BCM GPIO18

void led_function(char *arg)
{

    device_init();

    if(strcmp(arg, "HIGH") == 0 || strcmp(arg, "ON") == 0)
    {
        softPwmWrite(LED, 100);   // 100%
    }
    else if(strcmp(arg, "MID") == 0)
    {
        softPwmWrite(LED, 50);    // 50%
    }
    else if(strcmp(arg, "LOW") == 0)
    {
        softPwmWrite(LED, 20);    // 20%
    }
    else if(strcmp(arg, "OFF") == 0)
    {
        softPwmWrite(LED, 0);     // 0%
    }
}