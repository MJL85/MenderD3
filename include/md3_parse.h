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
 
#ifndef _MD3_PARSE_H
#define _MD3_PARSE_H

#include <stdio.h>
#include "definitions.h"
#include "tga.h"

#ifdef __cplusplus
extern "C"
{
#endif


/*
 *	Uncomment this to enable MD3 debugging.
 */
#ifdef _DEBUG
	//#define MD3_DEBUG
#endif

	
/*
 *	Various MD3 constants.
 */
#define MD3_MAGIC_NUMBER_LITTLE_ENDIAN		0x33504449
#define MAX_QPATH							64
#define MD3_MAX_FRAMES						1024
#define MD3_MAX_TAGS						16
#define MD3_MAX_SURFACES					32
#define MD3_XYZ_SCALE						(1.0f/64.0f)

/*
 *	The size of various structures in the file -
 *	not sizeof(struct ...) on the implementation side.
 */
#define MD3_SIZEOF_HEADER	(MAX_QPATH + (sizeof(int) * 11))
#define MD3_SIZEOF_FRAME	((sizeof(struct vec3_t) * 3) + sizeof(float) + (sizeof(char) * 16))
#define MD3_SIZEOF_TAG		((sizeof(char) * MAX_QPATH) + (sizeof(struct vec3_t) * 4))
#define MD3_SIZEOF_SURFACE	((sizeof(int) * 11) + (sizeof(char) * MAX_QPATH))
#define MD3_SIZEOF_SHADER	((sizeof(char) * MAX_QPATH) + sizeof(int))
#define MD3_SIZEOF_TRIANGLE	(sizeof(int) * 3)
#define MD3_SIZEOF_TEXCOORD	(sizeof(float) * 2)
#define MD3_SIZEOF_VERTEX	(sizeof(short) * 4)

/*
 *	Handy macro used in md3_load_list() to load an array
 *	of objects from the MD3 file into the specified destination.
 *
 *	dest		- the pointer that will point to the array
 *	object_type	- the structure type of the array elements
 *	elements	- the number of elements in the array
 *	base		- added to offset to find where src is in memory
 *	offset		- added to base to find where src is in memory
 */
#define LOAD_ARRAY(_dest, _object_type, _elements, _base, _offset, _fptr)						\
				do {																			\
					_dest = (_object_type*)malloc(sizeof(_object_type) * _elements);			\
					fseek(_fptr, (_base + _offset), SEEK_SET);									\
					fread(_dest, (sizeof(_object_type) * _elements), 1, _fptr);					\
				} while (0)

/*
 *	Valid body parts.
 */
enum MD3_BODY_PARTS {
	MD3_HEAD = 1,
	MD3_TORSO = 2,
	MD3_LEGS = 4,
	MD3_WEAPON = 8,
	
	MD3_LIGHT = 16,
	
	ETHER
};
			

/* none of these should be aligned */
#pragma pack(1)
				
/*
 *	MD3 structures.
 */				 
struct vec3_t {
	float x, y, z;
} NO_ALIGN;


struct md3_frame_t {
	struct vec3_t min_bounds;
	struct vec3_t max_bounds;
	struct vec3_t local_origin;
	float radius;
	char name[16];
} NO_ALIGN;


struct md3_tag_t {
	char name[MAX_QPATH];			/* name of tag object		*/
	struct vec3_t origin;			/* coordinates				*/
	struct vec3_t axis[3];			/* orientation				*/
} NO_ALIGN;


struct md3_shader_t {
	char name[MAX_QPATH];			/* name of shader			*/
	int shader_index;				/* shader index number		*/
	
	struct tga_t* texture;			/* targa loaded texture													*/
	unsigned int* gl_text_id;		/* openGL texture id; points to world_texture_t.gl_text_id				*/
	int* gl_text_bound;				/* has the texture been bound?; points to world_texture_t.dl_text_bound	*/
} NO_ALIGN;


struct md3_triangle_t {
	int index[3];					/* index of offset values into vertex list
					   				   that constitute cornders of triangle		*/
} NO_ALIGN;


struct md3_texcoord_t {
	float st[2];					/* texture coordinates						*/
} NO_ALIGN;


struct md3_vertex_t {
	short x, y, z;					/* x-y-z vector normalized by MD3_XYZ_SCALE	*/
	short normal;					/* normal encoded vector					*/
	
	float normalxyz[3];
} NO_ALIGN;


struct md3_surface_t {
	struct md3_surface_t* next;		/* next surface in the list					*/

	int ident;						/* magic number								*/
	char name[MAX_QPATH];			/* surface name								*/
	int flags;						/* unknown flags							*/
	int num_frames;					/* number of animation frames (== md3_model_t)	*/
	int num_shaders;				/* number of shaders for this surface		*/
	int num_verts;					/* number of vertexs for surface			*/
	int num_triangles;				/* number of triangles for surface			*/
	int ofs_triangles;				/* triangles relative offset				*/
	int ofs_shaders;				/* shaders relative offset					*/
	int ofs_st;						/* ST (texture) relative offset				*/
	int ofs_xyznormal;				/* vertex relative offset					*/
	int ofs_end;					/* end of surface relative offset			*/
	
	struct md3_shader_t* shader;		/* array of shaders						*/
	struct md3_triangle_t* triangle;	/* array of triangles					*/
	struct md3_texcoord_t* st;			/* array of surface textures			*/
	struct md3_vertex_t* vertex;		/* array of vertexes					*/
} NO_ALIGN;

#pragma pack(8)


/*
 *	Valid animation types.
 */
enum MD3_ANIMATIONS {
	BOTH_DEATH1 = 0,
	BOTH_DEAD1,
	BOTH_DEATH2,
	BOTH_DEAD2,
	BOTH_DEATH3,
	BOTH_DEAD3,
	TORSO_GESTURE,
	TORSO_ATTACK,
	TORSO_ATTACK2,
	TORSO_DROP,
	TORSO_RAISE,
	TORSO_STAND,
	TORSO_STAND2,
	LEGS_WALKCR,
	LEGS_WALK,
	LEGS_RUN,
	LEGS_BACK,
	LEGS_SWIM,
	LEGS_JUMP,
	LEGS_LAND,
	LEGS_JUMPB,
	LEGS_LANDB,
	LEGS_IDLE,
	LEGS_IDLECR,
	LEGS_TURN
};
struct md3_anim_names_t {
	enum MD3_ANIMATIONS id;
	char* name;
	int flags;
};
#define MD3_MAX_ANIMS 25

/*
 *	These are used in md3_anim_names_t::flags that tell
 *	the render what body parts this animation applies to.
 */
#define ANIM_HEAD		0x01			/* should never really be used */
#define ANIM_BODY		0x02
#define ANIM_LEGS		0x04


/*
 *	Animation structure.
 */
struct md3_anim_t {
	char name[64];
	int first_frame;
	int last_frame;
	int frames;
	int loop;
	int fps;
	enum MD3_ANIMATIONS anim_id;
};


/*
 *	Model animation state.
 */
struct md3_anim_state_t {
	struct md3_anim_names_t* anim_info;
	int id;								/* redundent since it's in anim_info but convenient */
	int frame;
	int next_frame;
	float t;
	double last_time;
	int animated;						/* set to 1 if the model is in a state of animation */
};


struct md3_model_t {
	FILE* fptr;							/* file pointer							*/
	long file_len;						/* file length in bytes					*/
	byte* dptr;							/* beginning of file in memory			*/
	//byte* sdptr;						/* beginning of file in memory (do not alter)	*/
	
	int ident;							/* md3 magic number, endianness			*/
	int version;						/* version number of file format		*/
	char name[MAX_QPATH];				/* model name							*/
	int flags;							/* model flags							*/
	int num_frames;						/* number of frames						*/
	int num_tags;						/* number of tags						*/
	int num_surfaces;					/* number of surfaces					*/
	int num_skins;						/* number of skins						*/
	int ofs_frames;						/* frames relative offset				*/
	int ofs_tags;						/* tags relative offset					*/
	int ofs_surfaces;					/* surfaces relative offset				*/
	int ofs_eof;						/* EOF relative offset					*/

	struct md3_frame_t* frames;			/* list of frames						*/
	struct md3_tag_t* tags;				/* list of tags							*/
	struct md3_surface_t* surface_ptr;	/* list of surfaces						*/
		
	/* custom stuff */
	struct md3_model_t** links;			/* child model links					*/

	char* model_name;					/* custom model name					*/
	enum MD3_BODY_PARTS body_part;		/* the type of body part this model is	*/
	struct md3_anim_state_t anim_state;	/* current animation state				*/
	float rot[3];						/* user defined rotation on x/y/z		*/
	float scale_factor;					/* scaling factor (after MD3_XYZ_SCALE)	*/
	int draw_bounding_box;				/* should bounding box be rendered?		*/
	
	int total_triangles;				/* total number of triangles for model	*/
};


struct md3_model_t* md3_load_model(char* file, char* texture_path_prefix);
void md3_unload_model(struct md3_model_t* model);

struct md3_model_t* load_model(char* file);
void unload_model(struct md3_model_t* model, int unload_weapon_link);

struct md3_model_t* load_weapon(char* path, char* texture_path_prefix);
void unload_weapon(struct md3_model_t* w);

int md3_link_models(struct md3_model_t* parent, struct md3_model_t* child);
int md3_unlink_models(struct md3_model_t* parent, struct md3_model_t* child);

struct md3_anim_names_t* get_animation_by_id(enum MD3_ANIMATIONS id);
struct md3_anim_names_t* get_animation_by_name(char* name);

void load_light_model(char* file, int light_num);

#ifdef __cplusplus
}
#endif

#endif /* _MD3_PARSE_H */
