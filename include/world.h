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
 
#ifndef _WORLD_H
#define _WORLD_H

#include "md3_parse.h"
#include "tga.h"

#define X_AXIS		0
#define Y_AXIS		1
#define Z_AXIS		2


/*
 *	Rendering options.
 */
#define RENDER_TEXTURES			0x001
#define RENDER_WIREFRAME		0x002
#define ENGINE_LIGHTING			0x004
#define RENDER_MIRRORS			0x008
#define RENDER_ANIM_LOOP		0x010
#define RENDER_FLASHLIGHT		0x020
#define ENGINE_INTERPOLATE		0x040
#define ENGINE_AA				0x080
#define ENGINE_DEPTH_OF_FIELD	0x100

#define WORLD_DEFAULT_FLAGS		(RENDER_TEXTURES | ENGINE_LIGHTING | ENGINE_INTERPOLATE)

#define WORLD_IS_SET(flag)		((g_world->flags & flag) == flag)

#define DEFAULT_CAMERA_TROT			45
#define DEFAULT_CAMERA_PROT			15
#define DEFAULT_CAMERA_DISTANCE		100.0f

#define DEFAULT_LIGHT_TROT			0
#define DEFAULT_LIGHT_PROT			0
#define DEFAULT_LIGHT_DISTANCE		100.0f

#define DEFAULT_LIGHT_DIR_TROT		0
#define DEFAULT_LIGHT_DIR_PROT		0

/*
 *	Default animations.
 */
#define DEFAULT_TORSO_ANIM		TORSO_STAND
#define DEFAULT_LEGS_ANIM		LEGS_IDLE

#define SET_DEFAULT_ANIMATIONS()	do {												\
										set_model_animation(LEGS_IDLE);					\
										set_model_animation(TORSO_STAND);				\
									} while(0)

/*
 *	Linked list of models.
 */
struct world_link_models_t {
	struct world_link_models_t* next;
	struct md3_model_t* model;
};


/*
 *	Linked list of TGA textures.
 */
struct world_texture_t {
	struct world_texture_t* next;
	struct tga_t* text;
	char* name;
	int binds;						/* how many models are using this texture								*/
	unsigned int gl_text_id;		/* the GL texture identifier; md3_surface_t.gl_text_id points to this	*/
	int gl_text_bound;				/* is texture bound?; md3_surface_t.gl_text_bound points to this		*/
};


/* camera stuff */
struct camera_t {
	GLdouble trot;
	GLdouble prot;
	GLfloat r;
	GLdouble center_xyz[3];
};


/* environment stuff */
struct env_t {
	GLfloat bg_rgba[4];
	GLfloat fov;
	GLfloat vnear, vfar;
};


/* mouse information */
struct mouse_t {
	int depressed;		/* 1=pressed, 0=not pressed						*/
	int old_pos[2];		/* the xy position where the button was pressed	*/
	float sensitivity;	/* mouse sensitivity; 1=low, 5=high				*/
};


/* light information */
struct light_t {
	GLfloat position[4];
	GLfloat spotDirection[3];
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat spotCutoff;
	GLfloat spotExponent;
	GLfloat spotAttenuation[3];  /*  [0] = constant, [1] = linear, [2] = quadratic	*/
	
	/* positional angles */
	double trot;
	double prot;
	float r;
	
	/* directional angles */
	double dir_trot;
	double dir_prot;
	
	struct md3_model_t* model;	/* model for this light - if needed */
};


/* material information */
struct material_t {
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat emission[4];
	GLfloat shininess;
};


struct mirror_t {
	struct mirror_t* next;

	double clip[4];
	float origin[3];
	float axis[3];
	float normal[3];
	float angle;
};


/*
 *	The world structure holds everything relevant
 *	to the current state.
 */
struct world_t {
	struct md3_model_t* root_model;			/* root model - start of render tree				*/
	struct world_link_models_t* models;		/* array of model parts	(not needed for rendering)	*/
	struct world_texture_t* texts;			/* array of textures								*/
		
	struct md3_anim_t anims[MD3_MAX_ANIMS];	/* animation data				*/
		
	struct camera_t camera;					/* camera position				*/
	struct env_t env;						/* environment settings			*/
	struct light_t light[2];				/* currently only use 1 light	*/

	int flags;								/* rendering options			*/
	
	/* hackish */
	unsigned int gl_box_id;					/* call list id for bounding box	*/
	unsigned int gl_plane_id;				/* call list id for tes plane		*/
	struct mirror_t* mirrors;				/* list of mirrors					*/
	int model_triangles;					/* total triangles for a model		*/
	int aa_factor;							/* Anti-Aliasing factor				*/
	float depth_focus;						/* Focal point for depth of field	*/
};


#ifdef __cplusplus
extern "C"
{
#endif

struct world_t* world_init();
void world_free(struct world_t* wptr);

void world_add_model(struct world_t* wptr, struct md3_model_t* mptr, int root);
void world_del_model(struct world_t* wptr, struct md3_model_t* mptr);

void world_link_model(struct world_t* wptr, struct md3_model_t* mptr);
void world_delink_model(struct world_t* wptr, struct md3_model_t* mptr);

void world_add_texture(struct world_t* wptr, struct tga_t* tptr, char* name, struct md3_shader_t* sptr);
void world_del_texture(struct world_t* wptr, struct tga_t* text);
void world_using_texture(struct world_t* wptr, struct tga_t* text);
void world_not_using_texture(struct world_t* wptr, struct tga_t* text);

struct tga_t* world_texture_cached(struct world_t* wptr, char* name, struct md3_shader_t* sptr);

struct md3_model_t* world_get_model_by_name(char* name);
struct md3_model_t* world_get_model_by_type(enum MD3_BODY_PARTS type);

void set_model_animation(enum MD3_ANIMATIONS id);
void world_stop_model_animation(int model_types);

void world_tick_model(struct md3_model_t* m);

void rotate_model(enum MD3_BODY_PARTS type, int axis, float degree);
void rotate_model_absolute(enum MD3_BODY_PARTS type, int axis, float degree);
void rotate_all_models_absolute(int axis, float degree, unsigned int exclude);

void scale_model(enum MD3_BODY_PARTS type, float factor);
void scale_all_models(float factor, unsigned int exclude);

void world_set_options(struct world_t* wptr, int enable, int disable);

void world_set_camera_distance(struct world_t* wptr, float distance);

void apply_light(GLenum gllight, struct light_t* light);
void apply_material(struct material_t* material);
void apply_texture(struct md3_shader_t* sptr);

void init_light(struct light_t* light);
void init_camera(struct camera_t* camera);
void init_env(struct env_t* env);
	
void world_register_mirror(struct world_t* w, double* clip_plane, float* origin, float* normal, float* rot_axis, float rot_angle);

#ifdef __cplusplus
}
#endif


#endif /* _WORLD_H */
