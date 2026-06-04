#pragma once

extern int pin_select[7];

void device_init();

void led_function(char *arg);
void cds_function(int threshold, int client_sock);
void seg_function(int start_num);
void buz_function(char *arg);