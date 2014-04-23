/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * tmr.h	Timer functions
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_TMR_H_INCLUDED_)
#define _TMR_H_INCLUDED_

#include "system.h"
#include "z80.h"

typedef int64_t tmr_time_t;

typedef struct {
	/** @brief time when the timer was fired */
	tmr_time_t fired;
	/** @brief time when the timer expires */
	tmr_time_t expire;
	/** @brief time reset value */
	tmr_time_t restart;
	/** @brief callback paramter */
	uint32_t param;
	/** @brief callback function */
	void (*callback)(uint32_t param);
}	tmr_t;

#ifdef	__cplusplus
extern "C" {
#endif

/** @brief cycles to run in the current timeslice */
extern int cycles;

/** @brief cycles to run in the current timeslice */
extern int cycles_this_frame;

#define	TIME_IN_NSEC(n) ((double)(n)/1e9)
#define	TIME_IN_USEC(n) ((double)(n)/1e6)
#define	TIME_IN_MSEC(n) ((double)(n)/1e3)
#define	TIME_IN_SEC(n) ((double)(n))
#define	TIME_IN_HZ(n) (1.0/(n))

#define tmr_double_to_time(v) ((tmr_time_t)1000000000ull * (v))
#define tmr_time_to_double(v) ((double)(v)/1e9)

#define	time_zero	((tmr_time_t)0)
#define	time_never	((tmr_time_t)0x7fffffffffffffffull)

/** @brief current time */
extern tmr_time_t time_now(void);

/** @brief allocate a new timer */
extern tmr_t *tmr_alloc(void (*callback)(uint32_t), tmr_time_t t, uint32_t param, tmr_time_t r);

/** @brief reset the time when a timer expires */
extern int tmr_reset(tmr_t *timer, tmr_time_t t);

/** @brief remove a timer from the list */
extern int tmr_remove(tmr_t *timer);

/** @brief return the elapsed time for a timer, i.e. time until it fires */
extern tmr_time_t tmr_elapsed(tmr_t *timer);

/** @brief adjust the time when a timer expires, the parameter, and the reset time */
extern int tmr_adjust(tmr_t *timer, tmr_time_t t, uint32_t param, tmr_time_t r);

/** @brief return the next timer event */
extern tmr_t *tmr_next_event(void);

/** @brief expire timers by an amount of time */
extern void tmr_expire(tmr_time_t dt);

/** @brief run the CPU(s) for the next time slice */
extern void tmr_run_cpu(void *context, double clock);

#ifdef	__cplusplus
}
#endif

#endif /* !defined(_TMR_H_INCLUDED_) */


