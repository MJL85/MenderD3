/*
 *	This file is part of MenderD3
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _UTIL_H
#define _UTIL_H

#ifdef WIN32
	#include <time.h>
#else
	#include <sys/time.h>
#endif

#define IS_NUMERIC(x)		((x >= '0') && (x <= '9'))
#define IS_ALPHA(x)			(((x >= 'a') && (x <= 'z')) || ((x >= 'A') && (x <= 'Z')))
#define IS_ALPHANUMERIC(x)	(IS_ALPHA(x) || IS_NUMERIC(x))

#define TIMEVAL_TO_MS_DOUBLE(tv)		(((tv)->tv_sec * 1000.0) + ((tv)->tv_usec / 1000.0))

/*
 *	Floating point modulus arithmetic.
 */
#define FLOAT_MOD(i, m)		(i - ((int)(i / m) * m))

/*
 *	General abs() function.
 */
#define ABS(x)				((x >= 0) ? x : (x * -1))

/*
 *	Get the sign of a value.
 *	Ie: SIGN(-34.6) = -1
 *	    SIGN(322.3) = 1
 */
#define SIGN(x)				((x >= 0) ? 1 : -1)

#ifdef __cplusplus
extern "C"
{
#endif

char* get_path(char* file, int alloc);
char* strip_lf(char* s);

void get_time(struct timeval* t);
void get_duration(struct timeval* start, struct timeval* end);
double get_time_in_ms();

char* str_to_lower(char* str);

char* format_path_for_os(char* path);

#ifdef __cplusplus
}
#endif

#endif /* _UTIL_H */
