#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <kernel/rtc.h>

// note how it's 0x80! That means the first bit will be 1, disabling NMIs
// rtc_read and rtc_write will NOT disable interrupts (except NMIs)! functions
// calling them should disable and reenable interrupts
// failure to do so can put the RTC in an "undefined" state. (bad stuff)
#define RTC_BASE 0x80

// info about registers from http://wiki.osdev.org/CMOS#Getting_Current_Date_and_Time_from_RTC
#define RTC_SECONDS		(RTC_BASE+0)
#define RTC_MINUTES		(RTC_BASE+2)
#define RTC_HOURS		(RTC_BASE+4)
#define RTC_DAY			(RTC_BASE+7)
#define RTC_MONTH		(RTC_BASE+8)
#define RTC_YEAR		(RTC_BASE+9)

// this might not be set right b/c some BIOSes ignore it, do not use
#define RTC_WEEKDAY		(RTC_BASE+6)

// "oh look we can store years with two digits to save space! that's so cool,
// let's do that!" - people in the 1900s
//
// "oh wait computers won't be able to tell if it's 2000 and think it's 1900!
// we should be storing the century somewhere" - people in the 1990s
//
// "yeah! we'll agree on one universal standard to place the century!" - what
// should have happened
//
// "yeah! let's just have each RTC manufacturer pick a place to put the century
// and make OS developers have to guess!" - what actually happened
#define RTC_CENTURY		(RTC_BASE+50) // don't use this, it's probably wrong

#define RTC_STATUS_A	(RTC_BASE+10)
#define RTC_STATUS_B	(RTC_BASE+11)

#define RTC_ADDRESS_PORT 0x70
#define RTC_DATA_PORT 0x71

uint8_t rtc_read(uint16_t addr) {
	outb_slow(RTC_ADDRESS_PORT, addr);
	return inb_slow(RTC_DATA_PORT);
}

void rtc_write(uint16_t addr, uint8_t val) {
	outb_slow(RTC_ADDRESS_PORT, addr);
	outb_slow(RTC_DATA_PORT, val);
}

bool rtc_update_in_progress() {
	outb_slow(RTC_ADDRESS_PORT, 0x0A);
	return (inb_slow(RTC_DATA_PORT) & 0x80);
}

uint8_t rtc_bcd_convert(uint8_t bcd) {
	// formula from wiki.osdev.org/CMOS
	return ((bcd & 0xF0) >> 1) + ( ((bcd & 0xF0) >> 3) + (bcd & 0xf) );
}

rtc_time rtc_get_time() {
	rtc_time retVal;

	disable_interrupts(); // undefined state == bad
	while (rtc_update_in_progress()) {} // wait for there to be an update
	while (rtc_update_in_progress()) {} // wait for that update to finish

	retVal.seconds = rtc_read(RTC_SECONDS);
	retVal.minutes = rtc_read(RTC_MINUTES);
	retVal.hours = rtc_read(RTC_HOURS);
	retVal.month = rtc_read(RTC_MONTH);
	retVal.day = rtc_read(RTC_DAY);
	retVal.year = rtc_read(RTC_YEAR);

	// we have the raw values. *however* there are 4 different formats they
	// could be in because OF COURSE THERE ARE
	uint8_t statusB = rtc_read(RTC_STATUS_B); // status register b says what format
	if (!(statusB & 0x04)) {
		// it's bcd mode, convert it
		retVal.seconds = rtc_bcd_convert(retVal.seconds);
		retVal.minutes = rtc_bcd_convert(retVal.minutes);
		retVal.hours = rtc_bcd_convert(retVal.hours);
		retVal.month = rtc_bcd_convert(retVal.month);
		retVal.day = rtc_bcd_convert(retVal.day);
		retVal.year = rtc_bcd_convert(retVal.year);
	}
	if (!(statusB & 0x02) && (retVal.hours & 0x80)) {
		// it's in 12 hour mode, make it 24
		retVal.hours = ((retVal.hours & 0x7F) + 12) % 24;
	}

	// FIXME: Y3K bug
	retVal.year = retVal.year + 2000;

	enable_interrupts();
	return retVal;
}

void rtc_init() {
	rtc_time currentTime = rtc_get_time();
	printf("The current time and date is: %d-%d-%d %d:%d:%d\n", currentTime.year, currentTime.month, currentTime.day, currentTime.hours, currentTime.minutes, currentTime.seconds);
}
