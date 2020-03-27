#ifndef MELVIX_RTC_H
#define MELVIX_RTC_H

unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char day;
unsigned char month;
unsigned int year;

void read_rtc();

void write_time();

#endif