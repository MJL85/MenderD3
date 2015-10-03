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
 *	References used for the research of this code include:
 *
 *	http://www.j3d.org/matrix_faq/matrfaq_latest.html
 *	http://www.cprogramming.com/tutorial/3d/quaternions.html
 *	"Computer Graphics" by Hearn and Baker
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "definitions.h"
#include "md3_parse.h"
#include "quaternion.h"


/*
 *	Initialize a quaternion.
 *
 *	This should be done before any multiplication is
 *	performed _upon_ the quaternion.
 */
void quat_init(struct quat_t* q) {
	struct quat_t i = { 0.0f, 0.0f, 0.0f, 1.0f };
	*q = i;
}


/*
 *	Normalize a quaternion.
 */
void quat_normalize(struct quat_t* q) {
	float r = ((q->x * q->x) + (q->y * q->y) + (q->z * q->z) + (q->w * q->w));
	if (abs(1.0f - r) <= 0.0000001)
		/* already nomalized */
		return;
	r = sqrt(r);
	q->x /= r;
	q->y /= r;
	q->z /= r;
	q->w /= r;
}


/*
 *	Multiply quaternions.
 *
 *	A * B = C
 *
 *	C can be the same as A or B since the values are set
 *	after the calculation is done.
 *
 *	Multiplication is not commutative.
 */
void quat_mult(struct quat_t* a, struct quat_t* b, struct quat_t* c) {
	struct quat_t r;
	r.x = ((a->w * b->x) + (a->x * b->w) + (a->y * b->z) - (a->z * b->y));
	r.y = ((a->w * b->y) - (a->z * b->z) + (a->y * b->w) + (a->z * b->x));
	r.z = ((a->w * b->z) + (a->x * b->y) - (a->y * b->x) + (a->z * b->w));
	r.w = ((a->w * b->w) - (a->x * b->x) - (a->y * b->y) - (a->z * b->z));
	*c = r;
}


/*
 *	Apply a rotation to a quaternion.
 *
 *	The angle is assumed to be in degrees.
 */
void quat_rotate(struct quat_t* q, float ang, float x, float y, float z) {
	float rad = (ang * (PI / 180.0f));
	float ha = sinf(rad / 2.0f);
	q->x = x * ha;
	q->y = y * ha;
	q->z = z * ha;
	q->w = cosf(rad / 2.0f);
}


/*
 *	Create a 4x4 matrix from the quaternion.
 *	This is column-major (OpenGL style)
 *
 *	If origin is NULL no translation is applied.
 *
 *	The resulting matrix is stored at m.
 */
void quat_to_matrix_4x4(struct quat_t* q, struct vec3_t* origin, float* m) {
	float x = q->x;
	float y = q->y;
	float z = q->z;
	float w = q->w;
	
	/* first column */	
	m[0] = 1 - 2*(y*y + z*z);
	m[1] = 2*(x*y - z*w);
	m[2] = 2*(x*z + y*w);
	m[3] = 0;
	
	/* second column */
	m[4] = 2*(x*y + z*w);
	m[5] = 1 - 2*(x*x + z*z);
	m[6] = 2*(y*z - x*w);
	m[7] = 0;
	
	/* third column */
	m[8] = 2*(x*z - y*w);
	m[9] = 2*(y*z + x*w);
	m[10] = 1 - 2*(x*x + y*y);
	m[11] = 0;
	
	/* fourth column */
	if (origin) {
		m[12] = origin->x;
		m[13] = origin->y;
		m[14] = origin->z;
	} else {
		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
	}
	m[15] = 1;
}


/*
 *	Generate a quaternion from a 4x4 matrix.
 */
void quat_from_matrix_4x4(struct quat_t* q, float* m) {
	float trace = 0;
	float scale = 0;
	
	/* calculate the sum of the diagonal + 1 */
	trace = (m[0] + m[5] + m[10] + 1);
	
	/* if trace > 0 then we can calculate quat now */
    if (trace > 0.00000001) {
		scale = (2 * sqrt(trace));
		q->x = ((m[9] - m[6]) / scale);
		q->y = ((m[2] - m[8]) / scale);
		q->z = ((m[4] - m[1]) / scale);
		q->w = (0.25f * scale);
		return;
	}
	
	/* calculation depends on which diagonal has greatest value */
	if ((m[0] > m[5]) && (m[0] > m[10])) {
		/* column 0 */
		scale = (sqrt(1.0f + m[0] - m[5] - m[10]) * 2);
		q->x = (0.25f * scale);
		q->y = ((m[4] + m[1]) / scale);
		q->z = ((m[2] + m[8]) / scale);
		q->w = ((m[9] + m[6]) / scale);
	} else if (m[5] > m[10]) {
		/* column 1 */
		scale = (sqrt(1.0f + m[5] - m[0] - m[10]) * 2);
		q->x = ((m[4] + m[1]) / scale);
		q->y = (0.25f * scale);
		q->z = ((m[9] + m[6]) / scale);
		q->w = ((m[2] + m[8]) / scale);
	} else {
		/* column 2 */
		scale = (sqrt(1.0f + m[10] - m[0] - m[5]) * 2);
		q->x = ((m[2] + m[8]) / scale);
		q->y = ((m[9] + m[6]) / scale);
		q->z = (0.25f * scale);
		q->w = ((m[4] + m[1]) / scale);
	}
}


/*
 *	Convert a 3x3 matrix to a 4x4 matrix.
 *
 *	If origin is non-NULL it will be used for translation.
 */
void matrix_3x3_to_4x4(float* m3, float* m4, struct vec3_t* origin) {
	/*
	 *	0 3 6		0 4 8  12
	 *	1 4 7		1 5 9  13
	 *	2 5 8		2 6 10 14
	 *				3 7 11 15
	 */
	m4[0] = m3[0];
	m4[1] = m3[1];
	m4[2] = m3[2];
	m4[3] = 0;

	m4[4] = m3[3];
	m4[5] = m3[4];
	m4[6] = m3[5];
	m4[7] = 0;

	m4[8] = m3[6];
	m4[9] = m3[7];
	m4[10] = m3[8];
	m4[11] = 0;

	if (origin) {
		m4[12] = origin->x;
		m4[13] = origin->y;
		m4[14] = origin->z;
	} else {
		m4[12] = 0;
		m4[13] = 0;
		m4[14] = 0;
	}
	m4[15] = 1;
}


/*
 *	Generate a quaternion from a 3x3 matrix.
 */
void quat_from_matrix_3x3(struct quat_t* q, float* m) {
	float m4[16];
	matrix_3x3_to_4x4(m, m4, NULL);
	quat_from_matrix_4x4(q, m4);
}


/*
 *	Spherical linear interpolation of quaternions q1 and q2 by time factor t.
 *	The result is stored in q3.
 *
 *	q = (((q1.q0)^-1)^t) * q1
 *
 *	http://en.wikipedia.org/wiki/Slerp
 */
void quat_slerp(struct quat_t* q1, struct quat_t* q2, float t, struct quat_t* q3) {
	float dp = 0;
	float front_slerp;
	float back_slerp;
	
	/* obviously we don't need to slerp if q1 and q2 are the same */
	if ((q1->x == q2->x) && (q1->y == q2->y) && (q1->z == q2->z) && (q1->w == q2->w)) {
		q3->x = q1->x;
		q3->y = q1->y;
		q3->z = q1->z;
		q3->w = q1->w;
		return;
	}
	
	/* q1.q0 is a dot product */
	dp = ((q1->x * q2->x) + (q1->y * q2->y) + (q1->z * q2->z) + (q1->w * q2->w));
	
	/* the dot product can be negative, in which case the rotation is >90 degrees */
	if (dp < 0.0f) {
		q2->x *= -1;
		q2->y *= -1;
		q2->z *= -1;
		q2->w *= -1;
		dp *= -1;
	}
	
	/*
	 *	Optimization.
	 *
	 *	I found on the net someone suggested to spherically interpolate
	 *	only if the angle is large enough to visually notice it.
	 *	Otherwise linear interpolation is faster and looks similar.
	 *
	 *	The dot product is the cos() of the angle.
	 *	If it is really small, say 0.2 then to a normal lerp.
	 */
	if ((1 - dp) > 0.1f) {
		float theta = acos(dp);
		float st = sin(theta);
		back_slerp = (sin((1 - t) * theta) / st);
		front_slerp = (sin((t * theta)) / st);
	} else {
		front_slerp = t;
		back_slerp = (1 - t);
	}	

	q3->x = ((back_slerp * q1->x) + (front_slerp * q2->x));
	q3->y = ((back_slerp * q1->y) + (front_slerp * q2->y));
	q3->z = ((back_slerp * q1->z) + (front_slerp * q2->z));
	q3->w = ((back_slerp * q1->w) + (front_slerp * q2->w));
}
