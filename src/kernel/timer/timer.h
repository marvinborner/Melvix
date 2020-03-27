#ifndef MELVIX_TIMER_H
#define MELVIX_TIMER_H

/**
 * Install the timer and set the timer phase to 100
 */
void timer_install();

/**
 * Stop processing for specific duration
 * @param ticks The duration of sleeping in ticks
 */
void timer_wait(int ticks);

/**
 * Get the current timer ticks
 * @return The current timer ticks (1000 ticks = 1 second)
 */
unsigned int get_time();

#endif