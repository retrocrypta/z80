/* ed:set tabstop=8 noexpandtab: */
/*********************************************************************
 *
 * system.h	System includes and global functions
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 **********************************************************************/
#include "system.h"
#include "timer.h"

#if	DEBUG
void logprintf(int ll, const char *tag, const char *fmt, ...)
{
	va_list ap;

	if (ll < 2)
		return;
	fprintf(stdout, "%4.7fs %-8s ",
		tmr_time_to_double(time_now()), tag);
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
}
#endif
