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

#ifndef _MD3_RENDER_H
#define _MD3_RENDER_H

/*
 *	Linearly interpolate a vertex.
 *
 *	v1, v2 are interpolated and the result is stored in v3.
 *	t is the time scale.
 */
#define LERP_VERTEX(v1, v2, t, v3)		do {											\
											v3->x = (v1->x + (t * (v2->x - v1->x)));	\
											v3->y = (v1->y + (t * (v2->y - v1->y)));	\
											v3->z = (v1->z + (t * (v2->z - v1->z)));	\
										} while (0)

#define LERP_NORMAL(v1, v2, t, v3)		do {																									\
											v3->normalxyz[0] = (v1->normalxyz[0] + (t * (vptr2->normalxyz[0] - vptr1->normalxyz[0])));			\
											v3->normalxyz[1] = (v1->normalxyz[1] + (t * (vptr2->normalxyz[1] - vptr1->normalxyz[1])));			\
											v3->normalxyz[2] = (v1->normalxyz[2] + (t * (vptr2->normalxyz[2] - vptr1->normalxyz[2])));			\
										} while (0)

#define SCALE_VERTEX(v, factor)			do {					\
											v->x *= factor;		\
											v->y *= factor;		\
											v->z *= factor;		\
										} while (0)
										
/*
 *	The jitter values provided by the red book are too small for our scale system.
 */
#define JITTER_SCALE				2.0
										
#ifdef __cplusplus
extern "C"
{
#endif

void render();
void render_primitives(int apply_names);

void render_flashlight();

void md3_render(struct md3_model_t* model, int apply_names, struct md3_tag_t* link_tag);
void md3_render_single(struct md3_model_t* model, int apply_names);

unsigned int make_bounding_box();
unsigned int make_tes_plane();

void draw_mirrors(struct mirror_t* m);

#ifdef __cplusplus
}
#endif

#endif /* _MD3_RENDER_H */
