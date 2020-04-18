#include <kernel/system.h>
#include <kernel/io/io.h>
#include <kernel/acpi/acpi.h>
#include <kernel/lib/stdio.h>

uint8_t second;
uint8_t minute;
uint8_t hour;
uint8_t day;
uint8_t month;
uint32_t year;

int get_update_in_progress_flag()
{
	outb(0x70, 0x0A);
	return (inb(0x71) & 0x80);
}

uint8_t get_rtc_register(int reg)
{
	outb(0x70, (uint8_t)reg);
	return inb(0x71);
}

void read_rtc()
{
	unsigned int century = 20; // ...
	uint8_t last_second;
	uint8_t last_minute;
	uint8_t last_hour;
	uint8_t last_day;
	uint8_t last_month;
	uint8_t last_year;
	uint8_t last_century;
	uint8_t registerB;

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
		last_year = (uint8_t)year;
		last_century = (uint8_t)century;

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
		second = (uint8_t)((second & 0x0F) + ((second / 16) * 10));
		minute = (uint8_t)((minute & 0x0F) + ((minute / 16) * 10));
		hour = (uint8_t)(((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80));
		day = (uint8_t)((day & 0x0F) + ((day / 16) * 10));
		month = (uint8_t)((month & 0x0F) + ((month / 16) * 10));
		year = (year & 0x0F) + ((year / 16) * 10);
		century = (century & 0x0F) + ((century / 16) * 10);
	}

	year += century * 100;

	// Convert to 24h if necessary
	if (!(registerB & 0x02) && (hour & 0x80)) {
		hour = (uint8_t)(((hour & 0x7F) + 12) % 24);
	}
}

void rtc_print()
{
	read_rtc();
	info("Current time: %d:%d:%d %d/%d/%d", hour, minute, second, month, day, year);
}
