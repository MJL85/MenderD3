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
 
#ifndef _DEFINITIONS_H
#define _DEFINITIONS_H


/*
 *	Global world structure.
 */
#ifdef __cplusplus
extern "C" {
#endif
extern struct world_t* g_world;
#ifdef __cplusplus
}
#endif


/*
 *	Version.
 */
#define MENDERD3_VERSION	"v0.5"


/*
 *	Uncomment this line to enable debug mode.
 */
#define _DEBUG


/*
 *	Comment this to disable interpolation.
 */
#define USE_INTERPOLATION


typedef unsigned char byte;


/*
 *	Math stuff.
 */
#define PI 3.14159265358979323846
#define PI_DIV_180 0.017453292519943296
#define deg PI_DIV_180


/*
 *	No alignment for structures.
 */
#ifdef _WIN32
	#define NO_ALIGN
#else
	#define NO_ALIGN		__attribute__((packed))
#endif


/*
 *	Various misc OS specific macros.
 */
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	
	/* windows defines timeval in winsock.h */
	#include <winsock.h>

	#define sinf(x)		(float)sin(x)
	#define cosf(x)		(float)cos(x)

	#define OS_PATH_DELIM		'\\'
#else
	#define OS_PATH_DELIM		'/'
#endif


/*
 *	OpenGL stuff.
 */
#ifdef WIN32
	#include <gl\gl.h>
	#include <gl\glu.h>
	#include <gl\glaux.h>
#else
	#include <GL/gl.h>
#endif

/*
 *	It is possible GL_BGR and GL_BGRA are not defined.
 *	I noticed this while compiling on Windows.
 */
#ifdef GL_BGR_EXT
	#ifndef GL_BGR
		#define GL_BGR		GL_BGR_EXT
	#endif
	#ifndef GL_BGRA
		#define GL_BGRA		GL_BGRA_EXT
	#endif
#endif


#endif /* _DEFINITIONS_H */
