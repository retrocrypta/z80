/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * timer.c	Timer functions
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#include "timer.h"

/** @brief current time */
tmr_time_t now;

/** @brief cycles to run in the current timeslice */
int cycles = 0;

/** @brief sum of cycles during the last frame */
int cycles_this_frame = 0;

double tmr_clock = 0.0;

static tmr_t *timers[MAX_TMR];
static uint32_t ti;
static volatile int adjust = 0;

/** @brief sort the list of timers by expire */
int sort_time(const void *p1, const void *p2)
{
	const tmr_t *t1 = *(const tmr_t **)p1;
	const tmr_t *t2 = *(const tmr_t **)p2;
	if (t1->expire < t2->expire)
		return -1;
	if (t1->expire > t2->expire)
		return +1;
	return 0;
}

/** @brief return the current time */
tmr_time_t time_now(void)
{
	return now + tmr_double_to_time((double)z80_cc / tmr_clock);
}

/** @brief allocate a new timer */
tmr_t *tmr_alloc(void (*callback)(uint32_t), tmr_time_t t, uint32_t param, tmr_time_t r)
{
	tmr_t *timer;
	if (ti >= MAX_TMR)
		return NULL;
	timers[ti] = timer = calloc(1, sizeof(tmr_t));
	if (NULL == timer)
		return NULL;
	ti++;
	timer->expire = time_now() + t;
	timer->restart = r;
	timer->param = param;
	timer->callback = callback;
	cycles = z80_cc;
	return timer;
}

/** @brief remove a timer from the list */
int tmr_remove(tmr_t *timer)
{
	uint32_t i, j;

	for (i = 0; i < ti; i++)
		if (timer == timers[i])
			break;
	if (i >= ti)
		return -1;

	free(timer);
	for (j = i + 1; j < ti; j++)
		timers[j - 1] = timers[j];
	ti--;

	return 0;
}

/** @brief return the elapsed time for a timer */
tmr_time_t tmr_elapsed(tmr_t *timer)
{
	uint32_t i;

	for (i = 0; i < ti; i++)
		if (timer == timers[i])
			break;
	if (i >= ti)
		return -1;

	return time_now() - timer->fired;
}

/** @brief reset the time when a timer expires */
int tmr_reset(tmr_t *timer, tmr_time_t t)
{
	int i;
	for (i = 0; i < ti; i++)
		if (timer == timers[i])
			break;
	if (i >= ti)
		return -1;

	if (time_never == t) {
		timer->expire = time_never;
	} else {
		timer->expire = time_now() + t;
	}
	cycles = z80_cc;
	return 0;
}

/** @brief adjust the time when a timer expires */
int tmr_adjust(tmr_t *timer, tmr_time_t t, uint32_t param, tmr_time_t r)
{
	int i;
	for (i = 0; i < ti; i++)
		if (timer == timers[i])
			break;
	if (i >= ti)
		return -1;

	timer->expire = time_now() + t;
	timer->restart = r;
	timer->param = param;
	cycles = z80_cc;
	return 0;
}

tmr_t *tmr_next_event(void)
{
	if (0 == ti)
		return NULL;
	qsort(timers, ti, sizeof(tmr_t *), sort_time);
	return timers[0];
}

/** @brief adjust time by dt nanoseconds and expire timers */
void tmr_expire(tmr_time_t dt)
{
	tmr_t *timer;

	now += dt;
	while (ti > 0 && NULL != (timer = timers[0])) {

		if (timer->expire > now)
			return;

		timer->fired = now;

		if (timer->callback)
			(*timer->callback)(timer->param);

		if (time_zero == timer->restart) {
#if	0
			/* one shot timer (timer->expire may be modified) */
			if (timer->expire <= now) {
				LOG((3,"TIMER","remove one-shot %p\n", timer));
				tmr_remove(timer);
			}
#endif
			continue;
		}
		if (time_never == timer->restart) {
			/* one shot timer that never expires */
			timer->expire = time_never;
			continue;
		}
		/* repeated timer */
		timer->expire = now + timer->restart;
	}
}

/** @brief run the CPU(s) for the next time slice */
void tmr_run_cpu(void *context, double clock)
{
	z80_cpu_t *cpu = (z80_cpu_t *)context;
	tmr_t *slice = tmr_next_event();
	int ran;

	tmr_clock = clock;
	slice = tmr_next_event();
	if (NULL == slice)
		return;
	if (slice->expire <= now)
		cycles = 1;
	else
		cycles = (int)(tmr_time_to_double(slice->expire - now) * clock) - adjust;
#if	0
	z80_dump_state(cpu);
#endif
	ran = z80_execute(cpu);
	cycles_this_frame += ran;
	z80_cc = 0;
	adjust = (ran > cycles) ? ran - cycles : 0;
	tmr_expire(tmr_double_to_time((double)ran / clock));
}
