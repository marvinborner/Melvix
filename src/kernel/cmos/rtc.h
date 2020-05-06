#ifndef MELVIX_RTC_H
#define MELVIX_RTC_H

#include <stdint.h>

u8 second;
u8 minute;
u8 hour;
u8 day;
u8 month;
u32 year;

void read_rtc();

void rtc_print();

#endif