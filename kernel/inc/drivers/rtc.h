// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef RTC_H
#define RTC_H

#include <def.h>

struct rtc {
	u8 second;
	u8 minute;
	u8 hour;
	u8 day;
	u8 month;
	u32 year;
};

struct rtc rtc_read(void);
u32 rtc_stamp(void);
CLEAR void rtc_install(void);

#endif
