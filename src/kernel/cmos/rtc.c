#include <kernel/io/io.h>
#include <kernel/graphics/vesa.h>
#include <kernel/acpi/acpi.h>

unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char day;
unsigned char month;
unsigned int year;

int get_update_in_progress_flag()
{
    outb(0x70, 0x0A);
    return (inb(0x71) & 0x80);
}

unsigned char get_rtc_register(int reg)
{
    outb(0x70, reg);
    return inb(0x71);
}

void read_rtc()
{
    unsigned int century = 20; // ...
    unsigned char last_second;
    unsigned char last_minute;
    unsigned char last_hour;
    unsigned char last_day;
    unsigned char last_month;
    unsigned char last_year;
    unsigned char last_century;
    unsigned char registerB;

    while (get_update_in_progress_flag());
    second = get_rtc_register(0x00);
    minute = get_rtc_register(0x02);
    hour = get_rtc_register(0x04);
    day = get_rtc_register(0x07);
    month = get_rtc_register(0x08);
    year = get_rtc_register(0x09);
    // century = get_rtc_register(fadt->century); // TODO: Fix fadt table (page fault!)

    // Try until the values are the same (fix for RTC updates)
    do {
        last_second = second;
        last_minute = minute;
        last_hour = hour;
        last_day = day;
        last_month = month;
        last_year = year;
        last_century = century;

        while (get_update_in_progress_flag());
        second = get_rtc_register(0x00);
        minute = get_rtc_register(0x02);
        hour = get_rtc_register(0x04);
        day = get_rtc_register(0x07);
        month = get_rtc_register(0x08);
        year = get_rtc_register(0x09);
        // century = get_rtc_register(fadt->century);
    } while ((last_second != second) || (last_minute != minute) || (last_hour != hour) ||
             (last_day != day) || (last_month != month) || (last_year != year) ||
             (last_century != century));

    registerB = get_rtc_register(0x0B);

    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);
        // century = (century & 0x0F) + ((century / 16) * 10);
    }

    year += century * 100;

    // Convert to 24h if necessary
    if (!(registerB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }
}

void write_time()
{
    read_rtc();
    vesa_draw_string("Current time: ");
    vesa_draw_number(hour);
    vesa_draw_string(":");
    vesa_draw_number(minute);
    vesa_draw_string(":");
    vesa_draw_number(second);
    vesa_draw_string(" ");
    vesa_draw_number(month);
    vesa_draw_string("/");
    vesa_draw_number(day);
    vesa_draw_string("/");
    vesa_draw_number(year);
    vesa_draw_string("\n");
}