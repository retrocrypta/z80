/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * unicode.h	Unicode to ISO-8859-1 conversion
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#if !defined(_UNICODE_H_INCLUDED_)
#define _UNICODE_H_INCLUDED_

#include "system.h"

#if defined(__cplusplus)
extern "C" {
#endif

extern char *utf8_to_iso8859_1(const char *src, int size);

#if defined(__cplusplus)
}
#endif

#endif /* !defined(_UNICODE_H_INCLUDED_) */


