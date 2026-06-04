#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>
#include "device.h"

#define LED 1 // WiringPi pin 1 = BCM GPIO18

void cds_function(int threshold, int client_sock)
{
    int fd;
    int a2dChannel = 0; // analog channel AIN0, CDS sensor
    int prev, a2dVal;
    char send_buf[50];

    device_init();

    printf("[ADC/DAC(YL-40) Module testing........]\n");

    if ((fd = wiringPiI2CSetupInterface("/dev/i2c-1", 0x48)) < 0)
    {
        printf("wiringPiI2CSetupInterface failed:\n");
    }

    wiringPiI2CWrite(fd, 0x00 | a2dChannel); // 0000_0000

    prev = wiringPiI2CRead(fd); // Previously byte, garvage
    a2dVal = wiringPiI2CRead(fd);

    printf("Single Read -> prev = %d, a2dVal = %d\n", prev, a2dVal);

    memset(send_buf, 0, sizeof(send_buf));
    sprintf(send_buf, "CDS_VAL: %d\n", a2dVal);

    if (send(client_sock, send_buf, strlen(send_buf), 0) == -1)
    {
        perror("client send");
        return;
    }
    if (a2dVal < threshold)
    {
        printf("Bright!!\n");
        softPwmWrite(LED, 0);
    }
    else
    {
        printf("Dark!!\n");
        softPwmWrite(LED, 100);
    }
    
}