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
 
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "definitions.h"
#include "world.h"
#include "tga.h"

/*
 *	Load a tga file.
 */
struct tga_t* load_tga(char* file) {
	struct tga_t* tga = NULL;
	FILE* fptr = NULL;
	int size = 0;
	
	fptr = fopen(file, "rb");
	if (!fptr)
		return NULL;

	tga = (struct tga_t*)malloc(sizeof(struct tga_t));
	memset(tga, 0, sizeof(struct tga_t));

	/* read the header */
	fread((void*)&(tga->header), 18, 1, fptr);
	
	tga->header.depth /= 8;

	/* allocate memory for the image body */
	size = (tga->header.width * tga->header.height * tga->header.depth);
	tga->img = (byte*)malloc(sizeof(byte) * size);

	/* read the body in */
	fread((void*)tga->img, (sizeof(byte) * size), 1, fptr);

	fclose(fptr);
	
	/*
	 *	Check for horizontal/vertical flipping.
	 *	This value should be used to determine if
	 *	the texture coordinate needs to be 1-coord
	 */
	tga->vflip = 1;
	tga->hflip = 0;
	if (tga->header.desc & 0x20)
		tga->vflip = 0;
	if (tga->header.desc & 0x10)
		tga->hflip = 1;

	/* select bit-mode for opengl */
	switch ((int)tga->header.depth) {
		case 1:
			tga->gl_format = GL_LUMINANCE;
			break;
		case 3:
			tga->gl_format = GL_BGR;
			break;
		case 4:
			tga->gl_format = GL_BGRA;
			break;
	};
	tga->gl_compontents = tga->header.depth;

	return tga;
};


/*
 *	Free a tga file.
 */
void free_tga(struct tga_t* tga) {
	if (!tga)
		return;
	free(tga->img);
	free(tga);
}
