#ifndef _KERNEL_RTC_H
#define _KERNEL_RTC_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct rtc_time {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t day;
	uint8_t month;
	uint16_t year; // has to be bigger to store century
} rtc_time;

rtc_time rtc_get_time();
void rtc_init();

#endif
