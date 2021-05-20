// MIT License, Copyright (c) 2021 Marvin Borner

#include <drivers/cpu.h>
#include <def.h>
#include <fs.h>
#include <mem.h>
#include <drivers/rtc.h>
#include <str.h>
#include <drivers/timer.h>

static u8 rtc_busy(void)
{
	outb(0x70, 0x0A);
	return (inb(0x71) & 0x80) == 0x80;
}

static u8 rtc_register(u8 reg)
{
	outb(0x70, reg);
	return inb(0x71);
}

static void rtc_fill(struct rtc *rtc)
{
	while (rtc_busy())
		;

	rtc->second = rtc_register(0x00);
	rtc->minute = rtc_register(0x02);
	rtc->hour = rtc_register(0x04);
	rtc->day = rtc_register(0x07);
	rtc->month = rtc_register(0x08);
	rtc->year = rtc_register(0x09);
	//obj.century = 20; //rtc_register(0x00);
}

struct rtc rtc_read(void)
{
	struct rtc rtc = { 0 };
	rtc_fill(&rtc);

	struct rtc last = { 0 };
	do {
		rtc_fill(&last);
	} while (memcmp(&rtc, &last, sizeof(rtc)) != 0);

	u32 reg_b = rtc_register(0x0B);

	if (!(reg_b & 0x04)) {
		rtc.second = (u8)((rtc.second & 0x0F) + ((rtc.second / 16) * 10));
		rtc.minute = (u8)((rtc.minute & 0x0F) + ((rtc.minute / 16) * 10));
		rtc.hour = (u8)(((rtc.hour & 0x0F) + (((rtc.hour & 0x70) / 16) * 10)) |
				(rtc.hour & 0x80));
		rtc.day = (u8)((rtc.day & 0x0F) + ((rtc.day / 16) * 10));
		rtc.month = (u8)((rtc.month & 0x0F) + ((rtc.month / 16) * 10));
		rtc.year = (rtc.year & 0x0F) + ((rtc.year / 16) * 10);
		/* rtc.century = (century & 0x0F) + ((century / 16) * 10); */
	}

	if (!(reg_b & 0x02) && (rtc.hour & 0x80))
		rtc.hour = (u8)(((rtc.hour & 0x7F) + 12) % 24);

	return rtc;
}

u32 rtc_stamp(void)
{
	struct rtc rtc = rtc_read();
	return rtc.second + rtc.minute * 60 + rtc.hour * 360 + rtc.day * 360 * 24 +
	       rtc.year * 360 * 24 * 365;
}
