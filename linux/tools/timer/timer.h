/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Architecture specific code
 *
 *   project:      ReconOS
 *   author:       Christoph Rüthing, University of Paderborn
 *   description:  Functions needed for ReconOS which are architecure
 *                 specific and are implemented here.
 *
 * ======================================================================
 */

#ifndef TIMER_H
#define TIMER_H

/* == Timer functions ================================================== */

/*
 * Initializes the timer. Must be called before any other function can
 * be used.
 *
 *   no parameters
 */
void timer_init();

/*
 * Resets the timer to zero.
 *
 *   no parameters
 */
void timer_reset();

/*
 * Sets the step of the counter, i.e. the number of clock cycles to wait
 * between incrementing the counter. Therefore 0 means to count every clock cycle.
 */
void timer_setstep(unsigned int step);

/*
 * Gets the current timestamp.
 *
 *   no parameters
 */
unsigned int timer_get();

/*
 * Cleans up the driver.
 *
 *   no parameters
 */
void timer_cleanup();

/*
 * Converts the timer value to milliseconds.
 *
 *   t - timer value or difference
 */
float timer_toms(unsigned int t);

/*
 * Calculates the difference of two timestamps.
 *
 *   ts0 - first timestamp
 *   ts1 - second simestamp
 */
unsigned int timer_diff(unsigned int ts0, unsigned int ts1);

/*
 * Calculates the difference of two timestamps in milliseconds.
 *
 *   ts0 - first timestamp
 *   ts1 - second simestamp
 */
float timer_diffms(unsigned int ts0, unsigned int ts1);

#endif /* TIMER_H */