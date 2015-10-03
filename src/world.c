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
 *	The world keeps everything clean.
 *	The world is all knowing.
 *
 *	TEXTURES
 *		Multipule loading of textures in memory can be avoided using the world.
 *		The number of models using a given texture can be kept track of by
 *		calling world_using_texture() and world_not_using_texture().
 *		When no more models are using a given texture, ie, world_not_using_texture()
 *		has been called one less time than world_using_texture() (since initially
 *		loading a texture sets the count to 1), then the texture is automatically
 *		unloaded as it is no longer referenced by anything in the world.
 *
 *	MODELS
 *		All models are given to the world.
 *
 *	The world will deallocate everything it is given, including models and textures.
 */
 
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "md3_parse.h"
#include "tga.h"
#include "util.h"
#include "world.h"


/* global world object */
struct world_t* g_world = NULL;

static int get_next_frame(struct md3_anim_state_t* as);
static void _rotate_model(enum MD3_BODY_PARTS type, int axis, float degree, int absolute);


/*
 *	Construct and return a new world object.
 */
struct world_t* world_init() {
	struct world_t* w = (struct world_t*)malloc(sizeof(struct world_t));
	memset(w, 0, sizeof(struct world_t));
		
	w->flags = WORLD_DEFAULT_FLAGS;
	
	/* setup the camera */
	init_camera(&w->camera);
		
	/* setup the environment */
	init_env(&w->env);

	/* setup the main light */
	init_light(&w->light[0]);
	
	return w;
}


/*
 *	Deallocate the world...
 */
void world_free(struct world_t* wptr) {
	struct world_link_models_t* mnext = NULL;
	struct world_texture_t* tnext = NULL; 
	
	/* free all the models */
	while (wptr->models) {
		mnext = wptr->models->next;
		md3_unload_model(wptr->models->model);
		wptr->models = mnext;
	}
	
	/* free all the textures */
	while (wptr->texts) {
		free_tga(wptr->texts->text);
		free(wptr->texts->name);
		
		tnext = wptr->texts->next;
		free(wptr->texts);
		wptr->texts = tnext;
	}
	
	free(wptr);
}


/*
 *	Add a model to the world.
 */
void world_add_model(struct world_t* wptr, struct md3_model_t* mptr, int root) {
	struct world_link_models_t* add = (struct world_link_models_t*)malloc(sizeof(struct world_link_models_t));
	memset(add, 0, sizeof(struct world_link_models_t));
	
	add->model = mptr;
	
	/* add to front of list */
	add->next = wptr->models;
	wptr->models = add;
	wptr->model_triangles += add->model->total_triangles;
	
	/* set as root model if needed */
	if (root)
		wptr->root_model = add->model;
}


/*
 *	Delete the model from the world.
 *	Actual deallocate of the model is done my md3_unload_model()
 */
void world_del_model(struct world_t* wptr, struct md3_model_t* mptr) {
	struct world_link_models_t* del = wptr->models;
	struct world_link_models_t* last = NULL;
	while (del) {
		if (del->model == mptr) {
			/* delink this one */
			if (last)
				last->next = del->next;
			
			/* if this thing was the root set the root to be the next */
			if (wptr->root_model == mptr)
				wptr->root_model = (del->next ? del->next->model : NULL);
			if (wptr->models == del)
				wptr->models = del->next;

			wptr->model_triangles -= del->model->total_triangles;
			
			free(del);
			return;
		}
		last = del;
		del = del->next;
	}
}


/*
 *	Link a model from the others.
 */
void world_link_model(struct world_t* wptr, struct md3_model_t* mptr) {
	struct world_link_models_t* lm = wptr->models;
	if (!mptr)
		return;
	while (lm) {
		if (md3_link_models(lm->model, mptr))
			break;
		lm = lm->next;
	}
}


/*
 *	Delink a model from the others.
 */
void world_delink_model(struct world_t* wptr, struct md3_model_t* mptr) {
	struct world_link_models_t* lm = wptr->models;
	if (!mptr)
		return;
	while (lm) {
		if (md3_unlink_models(lm->model, mptr))
			break;
		lm = lm->next;
	}
}


/*
 *	Cache a texture.
 */
void world_add_texture(struct world_t* wptr, struct tga_t* tptr, char* name, struct md3_shader_t* sptr) {
	struct world_texture_t* add = (struct world_texture_t*)malloc(sizeof(struct world_texture_t));
		
	add->text = tptr;
	add->name = strdup(name);
	add->binds = 1;
	add->gl_text_id = 0;
	add->gl_text_bound = 0;
	
	if (sptr) {
		sptr->gl_text_id = &add->gl_text_id;
		sptr->gl_text_bound = &add->gl_text_bound;
	}
	
	/* add to front of list */
	add->next = wptr->texts;
	wptr->texts = add;
}


/*
 *	Delete the texture from the world.
 */
void world_del_texture(struct world_t* wptr, struct tga_t* text) {
	struct world_texture_t* del = wptr->texts;
	struct world_texture_t* last = NULL;
	while (del) {
		if (del->text == text) {
			/* delink this one */
			if (last)
				last->next = del->next;
			if (wptr->texts == del)
				wptr->texts = del->next;

			#ifdef _DEBUG
			printf("Texture \"%s\" deleted (GL unbind id %i).\n", del->name, del->gl_text_id);
			#endif
			
			/* tell GL to unbind the texture */
			glDeleteTextures(1, &del->gl_text_id);
			
			/* unload the texture */
			free_tga(del->text);			
			free(del);
			
			return;
		}
		last = del;
		del = del->next;
	}
}


/*
 *	Tell the world another model needs this texture.
 */
void world_using_texture(struct world_t* wptr, struct tga_t* text) {
	struct world_texture_t* t = wptr->texts;
		
	while (t) {
		if (t->text == text) {
			/* texture found */
			t->binds++;

			#ifdef _DEBUG
			printf("Texture \"%s\" now being used by %i models.\n", t->name, t->binds);
			#endif

			return;
		}
		t = t->next;
	}
}


/*
 *	Tell the world some model that was using this texture no longer needs it.
 */
void world_not_using_texture(struct world_t* wptr, struct tga_t* text) {
	struct world_texture_t* t = wptr->texts;
		
	while (t) {
		if (t->text == text) {
			/* texture found */
			t->binds--;

			#ifdef _DEBUG
			printf("Texture \"%s\" now being used by %i models.\n", t->name, t->binds);
			#endif
			
			if (!t->binds)
				/* no models are using this texture anymore, kill it */
				world_del_texture(wptr, text);

			return;
		}
		t = t->next;
	}
}


/*
 *	Check to see if a texture has already been
 *	added to the world.
 *
 *	Returns pointer to tga_t structure if it exists.
 */
struct tga_t* world_texture_cached(struct world_t* wptr, char* name, struct md3_shader_t* sptr) {
	struct world_texture_t* t = wptr->texts;
		
	while (t) {
		if (!strcmp(name, t->name)) {
			/* texture found */
			if (sptr) {
				sptr->gl_text_id = &t->gl_text_id;
				sptr->gl_text_bound = &t->gl_text_bound;
			}
			
			return t->text;
		}
		t = t->next;
	}
	
	return NULL;
}


/*
 *	Return the model structure for the assoicated model name.
 */
struct md3_model_t* world_get_model_by_name(char* name) {
	struct world_link_models_t* wmodel = g_world->models;
	while (wmodel) {
		if (!strcmp(wmodel->model->model_name, name))
			return wmodel->model;
		wmodel = wmodel->next;
	}
	return NULL;
}


/*
 *	Return the model structure for the assoicated model type.
 *	Type can be:
 *		MD3_HEAD
 *		MD3_TORSO
 *		MD3_LEGS
 *		MD3_WEAPON
 */
struct md3_model_t* world_get_model_by_type(enum MD3_BODY_PARTS type) {
	struct world_link_models_t* wmodel = g_world->models;	
	while (wmodel) {
		if (wmodel->model->body_part == type)
			return wmodel->model;
		wmodel = wmodel->next;
	}
	return NULL;
}


/*
 *	Set the animation for the model.
 */
void set_model_animation(enum MD3_ANIMATIONS id) {
	struct md3_anim_names_t* inf = NULL;
	struct md3_model_t* m = NULL;
	
	#if 0
	if (id == NO_ANIM) {
		/* turn off animation */
		world_disable_model_animation();
		return;
	}
	#endif
	
	inf = get_animation_by_id(id);
	if (!inf)
		return;
	
	/* body */
	if ((inf->flags & ANIM_BODY) == ANIM_BODY) {
		/* get the model pointer */
		m = world_get_model_by_type(MD3_TORSO);
		
		if (!m)
			return;
		
		m->anim_state.animated = 1;
		m->anim_state.id = inf->id;
		
		/* set starting frame for the animation */
		m->anim_state.frame = g_world->anims[m->anim_state.id].first_frame;
		m->anim_state.next_frame = get_next_frame(&m->anim_state);
	}

	/* legs */
	if ((inf->flags & ANIM_LEGS) == ANIM_LEGS) {
		/* get the model pointer */
		m = world_get_model_by_type(MD3_LEGS);

		if (!m)
			return;

		m->anim_state.animated = 1;
		m->anim_state.id = inf->id;
		
		/* set starting frame for the animation */
		m->anim_state.frame = g_world->anims[m->anim_state.id].first_frame;
		m->anim_state.next_frame = get_next_frame(&m->anim_state);
	}
}


/*
 *	Disable animation for selected models.
 *	model_types can be any of the following (OR'ed togther):
 *		MD3_HEAD
 *		MD3_TORSO
 *		MD3_LEGS
 *		MD3_WEAPON
 */
void world_stop_model_animation(int model_types) {
	struct world_link_models_t* lm = NULL;
	struct md3_model_t* m = NULL;
		
	/* iterate through each model */
	lm = g_world->models;
	while (lm) {
		m = lm->model;
		
		if ((model_types & m->body_part) == m->body_part)
			/* this was selected for termination */
			m->anim_state.animated = 0;
				
		lm = lm->next;
	}	
}


/*
 *	Update the animation state for the given model.
 */
void world_tick_model(struct md3_model_t* m) {
	double now, elapsed, frame_duration;
	
	if (!m->anim_state.animated)
		/* if we are not in a state of animation t should not change */
		return;

	now = get_time_in_ms();
	elapsed = (now - m->anim_state.last_time);
	frame_duration = (1000.0 / g_world->anims[m->anim_state.id].fps);
	
	#ifdef USE_INTERPOLATION
	if (WORLD_IS_SET(ENGINE_INTERPOLATE))
		m->anim_state.t = (elapsed / frame_duration);
	#endif

	if (elapsed >= frame_duration) {
		/* tick the frame to the next key frame */
		m->anim_state.frame++;
		m->anim_state.frame = m->anim_state.next_frame;
		m->anim_state.next_frame = get_next_frame(&m->anim_state);
		m->anim_state.last_time = now;
		m->anim_state.t = 0;
	}
}


/*
 *	Get the next frame for the animation state.
 */
static int get_next_frame(struct md3_anim_state_t* as) {
	int next = (as->frame + 1);

	if (next > g_world->anims[as->id].last_frame)
		/* when looping we start at loop, not at the first frame unless explicitly told to */
		return (WORLD_IS_SET(RENDER_ANIM_LOOP) ? g_world->anims[as->id].first_frame : g_world->anims[as->id].loop);
	return next;
}


/*
 *	Rotate a given body part along the specified axis __by the given degree__.
 *
 *	axis can be one of:
 *		X_AXIS
 *		Y_AXIS
 *		Z_AXIS
 */
void rotate_model(enum MD3_BODY_PARTS type, int axis, float degree) {
	_rotate_model(type, axis, degree, 0);
}


/*
 *	Rotate a given body part along the specified axis __to an absolute degree__.
 *
 *	axis can be one of:
 *		X_AXIS
 *		Y_AXIS
 *		Z_AXIS
 */
void rotate_model_absolute(enum MD3_BODY_PARTS type, int axis, float degree) {
	_rotate_model(type, axis, degree, 1);
}


/*
 *	Rotate all models along the specified axis __to an absolute degree__.
 *
 *	axis can be one of:
 *		X_AXIS
 *		Y_AXIS
 *		Z_AXIS
 *
 *	Exclude can be any MD3_BODY_PARTS OR'ed togther that will not be applied.
 */
void rotate_all_models_absolute(int axis, float degree, unsigned int exclude) {
	struct world_link_models_t* ln = g_world->models;
	while (ln) {
		if (!ln->model) {
			ln = ln->next;
			continue;
		}
		
		if ((ln->model->body_part & exclude) == exclude) {
			/* skip this one */
			ln = ln->next;
			continue;
		}

		ln->model->rot[axis] = degree;
		
		/* modulate axis rotation to be between 0 and 359 */
		ln->model->rot[axis] = FLOAT_MOD(ln->model->rot[axis], 360);

		ln = ln->next;
	}
}


/*
 *	Backend function for:
 *		rotate_model()
 *		rotate_model_absolute()
 */
static void _rotate_model(enum MD3_BODY_PARTS type, int axis, float degree, int absolute) {
	struct md3_model_t* m = NULL;
	
	if ((axis != X_AXIS) && (axis != Y_AXIS) && (axis != Z_AXIS))
		/* not a valid axis */
		return;
	
	m = world_get_model_by_type(type);
	if (!m)
		return;
	
	if (absolute)
		m->rot[axis] = degree;
	else
		m->rot[axis] += degree;
	
	/* modulate axis rotation to be between 0 and 359 */
	m->rot[axis] = FLOAT_MOD(m->rot[axis], 360);
}


/*
 *	Set the scale factor for a given body part.
 */
void scale_model(enum MD3_BODY_PARTS type, float factor) {
	struct md3_model_t* m = NULL;
	m = world_get_model_by_type(type);
	if (!m)
		return;
	m->scale_factor = factor;	
}


/*
 *	Set the scale factor for all body parts.
 *
 *	Exclude can be any MD3_BODY_PARTS OR'ed togther that will not be applied.
 */
void scale_all_models(float factor, unsigned int exclude) {
	struct world_link_models_t* ln = g_world->models;
	while (ln) {
		if (!ln->model) {
			ln = ln->next;
			continue;
		}

		if ((ln->model->body_part & exclude) == exclude) {
			/* skip this one */
			ln = ln->next;
			continue;
		}

		ln->model->scale_factor = factor;	
		ln = ln->next;
	}
}


/*
 *	Enable the rendering options specified by enable.
 *	Disable the rendering options specified by disable.
 *
 *	Flags may be OR'ed togther.
 *	Valid flags are specified in world.h
 */
void world_set_options(struct world_t* wptr, int enable, int disable) {
	/* remove mutually exclusive bits */
	int tmp = enable;
	tmp &= ~disable;
	disable &= ~enable;
	enable = tmp;

	wptr->flags |= enable;
	wptr->flags &= ~disable;
}


/*
 *	Set the distance from the origin the camera is located.
 *
 *	By default the distance is DEFAULT_CAMERA_DISTANCE
 */
void world_set_camera_distance(struct world_t* wptr, float distance) {
	wptr->camera.r = distance;
}


/*
 *	Apply the light to GL.
 */
void apply_light(GLenum gllight, struct light_t* light) {
	glLightfv(gllight, GL_POSITION, light->position);
	glLightfv(gllight, GL_DIFFUSE, light->diffuse);
	glLightfv(gllight, GL_SPECULAR, light->specular);
	glLightfv(gllight, GL_AMBIENT, light->ambient);
	glLightfv(gllight, GL_SPOT_DIRECTION, light->spotDirection);  
	glLightf(gllight, GL_SPOT_CUTOFF, light->spotCutoff);
	glLightf(gllight, GL_SPOT_EXPONENT, light->spotExponent);
	glLightf(gllight, GL_CONSTANT_ATTENUATION, light->spotAttenuation[0]);
	glLightf(gllight, GL_LINEAR_ATTENUATION, light->spotAttenuation[1]);
	glLightf(gllight, GL_QUADRATIC_ATTENUATION, light->spotAttenuation[2]);
}


/*
 *	Apply a material to GL.
 */
void apply_material(struct material_t* material) {
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material->emission);
}


/*
 *	Bind the texture within OpenGL.
 */
void apply_texture(struct md3_shader_t* sptr) {
	/* if no texture exists, it cannot be bound */
	if (!sptr->texture)
		return;
	
	if (sptr->gl_text_bound && !*sptr->gl_text_bound) {
		/*
		 *	Bind the texture within OpenGL if it has not already been done.
		 *	This speeds up rendering.
		 */
		glGenTextures(1, sptr->gl_text_id);
		glBindTexture(GL_TEXTURE_2D, *sptr->gl_text_id);
		glTexImage2D(
					GL_TEXTURE_2D,
					0,
					sptr->texture->gl_compontents,
					sptr->texture->header.width,
					sptr->texture->header.height,
					0,
					sptr->texture->gl_format,
					GL_UNSIGNED_BYTE,
					sptr->texture->img
		);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		
		*sptr->gl_text_bound = 1;
	}
	
	/* Apply the texture */
	glBindTexture(GL_TEXTURE_2D, *sptr->gl_text_id);
}


/*
 *	Set the light to the default light settings.
 */
void init_light(struct light_t* light) {
	struct light_t dl = {
		{ 100.0, 10.0, 10.0, 1.0	}, /* positon */
		{ -1.0, -1.0, -1.0		}, /* spot direction */
		{ 0.0, 0.0, 0.0, 1.0	},         /* ambiant */
		{ 1.0, 1.0, 1.0, 1.0	},	   /* diffuse */
		{ 1.0, 1.0, 1.0, 1.0	},	   /* specular */
		180.0,				   /* spot exponant */
		0.0,				   /* spot cutoff */
		{ 1.0, 0.0, 0.0			}, /* spot attenuation */
		DEFAULT_LIGHT_TROT,
		DEFAULT_LIGHT_PROT,
		DEFAULT_LIGHT_DISTANCE,
		DEFAULT_LIGHT_DIR_TROT,
		DEFAULT_LIGHT_DIR_PROT,
		NULL							/* no default model */
	};
	memcpy(light, &dl, sizeof(struct light_t));
}


/*
 *	Set the camera to the default settings.
 */
void init_camera(struct camera_t* camera) {
	struct camera_t dc = {
		DEFAULT_CAMERA_TROT,				/* position		*/
		DEFAULT_CAMERA_PROT,
		DEFAULT_CAMERA_DISTANCE,
		{ 0, 0, 0 }							/* center xyz	*/
	};
	memcpy(camera, &dc, sizeof(struct camera_t));
}


/*
 *	Set the environment to the default settings.
 */
void init_env(struct env_t* env) {
	struct env_t de = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },		/* background color	*/
		50.0f,							/* field of view	*/
		0.1f, 1000.0f					/* near, far		*/
	};
	memcpy(env, &de, sizeof(struct env_t));
}


void world_register_mirror(struct world_t* w, double* clip_plane, float* origin, float* normal, float* rot_axis, float rot_angle) {
	struct mirror_t* m = (struct mirror_t*)malloc(sizeof(struct mirror_t));
	
	memcpy(m->clip, clip_plane, sizeof(m->clip));
	memcpy(m->origin, origin, sizeof(m->origin));
	memcpy(m->axis, rot_axis, sizeof(m->axis));
	memcpy(m->normal, normal, sizeof(m->normal));
	m->angle = rot_angle;
	
	if (!w->mirrors) {
		m->next = NULL;
		w->mirrors = m;
	} else {
		m->next = w->mirrors;
		w->mirrors = m;		
	}
}
