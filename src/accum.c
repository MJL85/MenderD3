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
 
//#include <GL/gl.h>
#include <stdio.h>
#include <math.h>
#include "definitions.h"
#include "accum.h"

/*
 *	Replacement for gluPerspective() that can offset the viewport by a given value.
 *	This is useful when using the accumulator.
 *
 *	void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
 */
void acc_Perspective(double fov, double aspect, double znear, double zfar, double x_offset, double y_offset,
	double cx_offset, double cy_offset, double focus) {

	double top, bottom, left, right;
		
	top = (znear * tan(((fov * PI) / 180.0) / 2.0));
	bottom = (-1 * top);
		
	left = (aspect * bottom);
	right = (aspect * top);
		
	acc_Frustum(left, right, bottom, top, znear, zfar, x_offset, y_offset, cx_offset, cy_offset, focus);
}


/*
 *	Replacement for glFrustum() that can offset the viewport by a given value.
 *	This is useful when using the accumulator.
 *
 *	void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far)
 */
void acc_Frustum(double left, double right, double bottom, double top, double znear, double zfar,
	double x_offset, double y_offset, double cx_offset, double cy_offset, double focus) {
	
	double width, height;
	double vwidth, vheight;
	GLdouble viewport[4];
		
	glGetDoublev(GL_VIEWPORT, viewport);
	vwidth = viewport[2];
	vheight = viewport[3];
		
	width = (right - left);
	height = (top - bottom);
		
	x_offset = -(x_offset*width/vwidth + cx_offset*znear/focus);
	y_offset = -(y_offset*height/vheight + cy_offset*znear/focus);
		
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
		
	glFrustum(left + x_offset, right + x_offset, bottom + y_offset, top + y_offset, znear, zfar);
		
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(-1 * cx_offset, -1 * cy_offset, 0.0f);
}
