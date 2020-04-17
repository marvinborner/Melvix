#ifndef MELVIX_RTC_H
#define MELVIX_RTC_H

#include <stdint.h>

uint8_t second;
uint8_t minute;
uint8_t hour;
uint8_t day;
uint8_t month;
unsigned int year;

void read_rtc();

void rtc_print();

#endif
