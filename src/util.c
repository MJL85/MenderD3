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

/*
 *	Miscellaneous helper functions.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "definitions.h"
#include "util.h"

/*
 *	Get the path to the given file.
 *	Returns an allocated string.
 *
 *	If alloc is 1 then allocate the return string.
 *	Otherwise modify the given string.
 */	
char* get_path(char* file, int alloc) {
	char* buf = NULL;
	char* last_delim = NULL;
	char* s = NULL;
	
	if (alloc)
		buf = strdup(file);
	else
		buf = file;
	s = buf;
	
	/* convert path delimiters to for this OS */
	while (*s) {
		if ((*s == '\\') || (*s == '/')) {
			*s = OS_PATH_DELIM;
			if (*(s + 1))
				last_delim = s;
		}
		++s;
	}
	
	/* set last delim in buf to null */
	if (*(last_delim + 1)) {
		*(last_delim + 1) = '\0';
		*last_delim = OS_PATH_DELIM;
	} else
		*last_delim = '\0';
	
	return buf;
}


/*
 *	Remove a \r\n from the given string.
 *	Return the same pointerr.
 */
char* strip_lf(char* s) {
	int len;

	if (!s)
		return NULL;

	len = strlen(s);
	if (s[len - 1] == '\n')
		*(s + (len - 1)) = 0;
	
	/* if windows, a \r\n is a line feed */
	if (s[len - 2] == '\r')
		*(s + (len - 2)) = 0;
		
	return s;
}


/*
 *	Set t to the current time.
 */
void get_time(struct timeval* t) {
	#ifdef _WIN32
		long y = GetTickCount();
		t->tv_usec = ((y % 1000) * 1000);
		t->tv_sec = (y / 1000);
	#else
		gettimeofday(t, NULL);
	#endif
}

		
/*
 *	Calculate the duration between start and end
 *	and store the result in end.
 */
void get_duration(struct timeval* start, struct timeval* end) {
	end->tv_sec -= start->tv_sec;
	end->tv_usec -= start->tv_usec;
	if (end->tv_usec < 0) {
		end->tv_sec -= 1;
		end->tv_usec += 1000000;
	}
}


/*
 *	Get time in milliseconds.
 */
double get_time_in_ms() {
	struct timeval t;
	get_time(&t);
	return ((t.tv_sec * 1000.0) + (t.tv_usec / 1000.0));
}


/*
 *	Lowercase a full string.
 */
char* str_to_lower(char* str) {
	char* s = str;
	while (*s) {
		*s = tolower(*s);
		++s;
	}
	return str;
}


/*
 *	Format a given path for the current OS.
 */
char* format_path_for_os(char* path) {
	if (!path)
		return NULL;
	while (*path) {
		if ((*path == '/') || (*path == '\\'))
			*path = OS_PATH_DELIM;
		++path;
	}
	return path;
}

