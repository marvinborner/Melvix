#include <acpi/acpi.h>
#include <io/io.h>
#include <lib/stdio.h>
#include <system.h>

u8 second;
u8 minute;
u8 hour;
u8 day;
u8 month;
u32 year;

int get_update_in_progress_flag()
{
	outb(0x70, 0x0A);
	return (inb(0x71) & 0x80);
}

u8 get_rtc_register(int reg)
{
	outb(0x70, (u8)reg);
	return inb(0x71);
}

void read_rtc()
{
	u32 century = 20;
	u8 last_second;
	u8 last_minute;
	u8 last_hour;
	u8 last_day;
	u8 last_month;
	u8 last_year;
	u8 last_century;
	u8 registerB;

	while (get_update_in_progress_flag()) {
	};
	second = get_rtc_register(0x00);
	minute = get_rtc_register(0x02);
	hour = get_rtc_register(0x04);
	day = get_rtc_register(0x07);
	month = get_rtc_register(0x08);
	year = get_rtc_register(0x09);
	century = get_rtc_register(fadt->century);

	// Try until the values are the same (fix for RTC updates)
	do {
		last_second = second;
		last_minute = minute;
		last_hour = hour;
		last_day = day;
		last_month = month;
		last_year = (u8)year;
		last_century = (u8)century;

		while (get_update_in_progress_flag()) {
		};
		second = get_rtc_register(0x00);
		minute = get_rtc_register(0x02);
		hour = get_rtc_register(0x04);
		day = get_rtc_register(0x07);
		month = get_rtc_register(0x08);
		year = get_rtc_register(0x09);
		century = get_rtc_register(fadt->century);
	} while ((last_second != second) || (last_minute != minute) || (last_hour != hour) ||
		 (last_day != day) || (last_month != month) || (last_year != year) ||
		 (last_century != century));

	registerB = get_rtc_register(0x0B);

	if (!(registerB & 0x04)) {
		second = (u8)((second & 0x0F) + ((second / 16) * 10));
		minute = (u8)((minute & 0x0F) + ((minute / 16) * 10));
		hour = (u8)(((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80));
		day = (u8)((day & 0x0F) + ((day / 16) * 10));
		month = (u8)((month & 0x0F) + ((month / 16) * 10));
		year = (year & 0x0F) + ((year / 16) * 10);
		century = (century & 0x0F) + ((century / 16) * 10);
	}

	year += century * 100;

	// Convert to 24h if necessary
	if (!(registerB & 0x02) && (hour & 0x80)) {
		hour = (u8)(((hour & 0x7F) + 12) % 24);
	}
}

void rtc_print()
{
	read_rtc();
	info("Current time: %d:%d:%d %d/%d/%d", hour, minute, second, month, day, year);
}