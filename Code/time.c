/*
 * A collection of functions for handling time for use with a timer.
 *
 * For http://github.com/Zalodu/Quadcontrol-2
 * Author: Jesper Larsson (mrlarssonjr)
 * Date: 15/12/16
 */

#define LENGTH_OF_TICK 0.0025;

unsigned int count = 0;

/*
 * Tick time.
 */
void time_tick() {
	++count;
}

/*
 * Wait for specified amount of ticks.
 *
 * Parameter(s):
 * ticks - amount of ticks to wait.
 */
void time_blockFor(unsigned int ticks) {
	unsigned int limit = ticks + count;

	while(limit > count) {}
}

/*
 * Get the amount of elapsed ticks.
 * Note that the ticks will wrap around when they overflow.
 */
unsigned int time_getElapsedTicks() {
	return count;
}

double time_getLengthOfTick() {
	return LENGTH_OF_TICK;
}
