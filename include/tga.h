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
 
#ifndef _TGA_H
#define _TGA_H

#include "definitions.h"

/*
 *	The pragma will tell the compiler not to align the structure.
 */

/*
 *	TGA header.
 */
#pragma pack(1)
struct tga_header_t {
	byte ident_size;			/* size of id field												*/
	byte color_map_type;		/* type of color map [ 0=none, 1=palette ]						*/
	byte image_type;			/* type of image [ 0=none, 1=indexed, 2=rgb, 3=grey, >8=rle ]	*/

	short color_map_start;		/* starting entry in color map			*/
	short color_map_len;		/* total number of color map entries	*/
	byte color_map_bits;		/* number of color bits per entry		*/

	short xorig;				/* x-origin, lower left		*/
	short yorig;				/* y-origin, lower left		*/

	short width;				/* image width	*/
	short height;				/* image height	*/

	byte depth;					/* color depth, bits per pixel	*/
	byte desc;					/* image flags					*/
};
#pragma pack(8)


/*
 *	Main TGA structure.
 */
struct tga_t {
	struct tga_header_t header;
	byte* img;
	int gl_format;
	int gl_compontents;
	int vflip;
	int hflip;
};


#ifdef __cplusplus
extern "C"
{
#endif

struct tga_t* load_tga(char* file);
void free_tga(struct tga_t* tga);

#ifdef __cplusplus
}
#endif

#endif /* _TGA_H */
