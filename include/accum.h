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

#ifndef _ACCUM_H
#define _ACCUM_H

#ifdef __cplusplus
extern "C"
{
#endif

void acc_Perspective(double fov, double aspect, double znear, double zfar, double x_offset, double y_offset,
	double cx_offset, double cy_offset, double focus);

void acc_Frustum(double left, double right, double bottom, double top, double znear, double zfar,
	double x_offset, double y_offset, double cx_offset, double cy_offset, double focus);

#ifdef __cplusplus
}
#endif

#endif /* _ACCUM_H */
