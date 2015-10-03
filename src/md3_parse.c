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
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <stdarg.h>

#include "definitions.h"
#include "util.h"
#include "tga.h"
#include "world.h"
#include "md3_parse.h"

/*
 *	Valid animations.
 */
static struct md3_anim_names_t anim_structs[] = {
	/*	animation id		animation name		animation flags		*/
	{	BOTH_DEATH1,		"BOTH_DEATH1",		ANIM_BODY | ANIM_LEGS	},
	{	BOTH_DEAD1,			"BOTH_DEAD1",		ANIM_BODY | ANIM_LEGS	},
	{	BOTH_DEATH2,		"BOTH_DEATH2",		ANIM_BODY | ANIM_LEGS	},
	{	BOTH_DEAD2,			"BOTH_DEAD2",		ANIM_BODY | ANIM_LEGS	},
	{	BOTH_DEATH3,		"BOTH_DEATH3",		ANIM_BODY | ANIM_LEGS	},
	{	BOTH_DEAD3,			"BOTH_DEAD3",		ANIM_BODY | ANIM_LEGS	},
	{	TORSO_GESTURE,		"TORSO_GESTURE",	ANIM_BODY				},
	{	TORSO_ATTACK,		"TORSO_ATTACK",		ANIM_BODY				},
	{	TORSO_ATTACK2,		"TORSO_ATTACK2",	ANIM_BODY				},
	{	TORSO_DROP,			"TORSO_DROP",		ANIM_BODY				},
	{	TORSO_RAISE,		"TORSO_RAISE",		ANIM_BODY				},
	{	TORSO_STAND,		"TORSO_STAND",		ANIM_BODY				},
	{	TORSO_STAND2,		"TORSO_STAND2",		ANIM_BODY				},
	{	LEGS_WALKCR,		"LEGS_WALKCR",		ANIM_LEGS				},
	{	LEGS_WALK,			"LEGS_WALK",		ANIM_LEGS				},
	{	LEGS_RUN,			"LEGS_RUN",			ANIM_LEGS				},
	{	LEGS_BACK,			"LEGS_BACK",		ANIM_LEGS				},
	{	LEGS_SWIM,			"LEGS_SWIM",		ANIM_LEGS				},
	{	LEGS_JUMP,			"LEGS_JUMP",		ANIM_LEGS				},
	{	LEGS_LAND,			"LEGS_LAND",		ANIM_LEGS				},
	{	LEGS_JUMPB,			"LEGS_JUMPB",		ANIM_LEGS				},
	{	LEGS_LANDB,			"LEGS_LANDB",		ANIM_LEGS				},
	{	LEGS_IDLE,			"LEGS_IDLE",		ANIM_LEGS				},
	{	LEGS_IDLECR,		"LEGS_IDLECR",		ANIM_LEGS				},
	{	LEGS_TURN,			"LEGS_TURN",		ANIM_LEGS				}
};


static void md3_load_surfaces(struct md3_model_t* model, char* texture_path_prefix);
static void md3_make_normal(struct md3_vertex_t* vertex);

static void load_texture_for_model(struct md3_model_t* model, char* texture, char* surface);
static int load_anim_file(char* file, struct md3_anim_t* aptr);


/*
 *	Load an MD3 model.
 *	Returns a pointer to the MD3 model structure, NULL on failure.
 *
 *	Use texture_path_prefix only if you want to use the texture specified within the MD3
 *	and not the skin stuff.  Otherwise pass NULL.
 */
struct md3_model_t* md3_load_model(char* file, char* texture_path_prefix) {
	struct md3_model_t* model = (struct md3_model_t*)malloc(sizeof(struct md3_model_t));

	#ifdef MD3_DEBUG
	int i = 0;
	#endif

	memset(model, 0, sizeof(struct md3_model_t));
	
	/*
	 *	Open model file and map it into memory.
	 */
	model->fptr = fopen(file, "rb");
	if (!model->fptr) {
		printf("ERROR: Failed to open model file \"%s\".\n", file);
		free(model);
		return NULL;
	}
	
	/* get length of file */
	fseek(model->fptr, 0, SEEK_END);
	model->file_len = ftell(model->fptr);
	fseek(model->fptr, 0, SEEK_SET);
	
	#ifdef MD3_DEBUG
	printf("File Length: %ld bytes\n", model->file_len);
	#endif
	
	fread(&model->ident, MD3_SIZEOF_HEADER, 1, model->fptr);
	
	#ifdef MD3_DEBUG
	printf("magic number: %i (%s)\n", model->ident, (model->ident == 0x33504449) ? "little endian" : "big endian");
	printf("md3 version: %i\n", model->version);
	printf("name: \"%s\"\n", model->name);
	printf("flags: %i\n", model->flags);
	printf("frames: %i\n", model->num_frames);
	printf("tags: %i\n", model->num_tags);
	printf("surfaces: %i\n", model->num_surfaces);
	printf("tags: %i\n", model->num_tags);
	printf("surfaces: %i\n", model->num_surfaces);
	printf("skins: %i\n", model->num_skins);
	printf("frames offset: %i\n", model->ofs_frames);
	printf("tags offsets: %i\n", model->ofs_tags);
	printf("surfaces offsets: %i\n", model->ofs_surfaces);
	printf("EOF offset: %i\n", model->ofs_eof);
	printf("\n\n");
	#endif
	
	/* FRAMES */
	LOAD_ARRAY(model->frames, struct md3_frame_t, model->num_frames, 0, model->ofs_frames, model->fptr);

	#ifdef MD3_DEBUG
	printf("Frames loaded: %i\n", i);
	printf("Frame 1:\n");
	printf("\tname: [%s]\n", model->frames[0].name);
	printf("\n");
	#endif

	/* TAGS */
	LOAD_ARRAY(model->tags, struct md3_tag_t, (model->num_tags * model->num_frames), 0, model->ofs_tags, model->fptr);
	
	/* links - depend on number of tags (actual links are made later) */
	model->links = (struct md3_model_t**)malloc(sizeof(struct md3_model_t*) * model->num_tags);
	memset(model->links, 0, (sizeof(struct md3_model_t*) * model->num_tags));

	#ifdef MD3_DEBUG
	printf("Tags loaded: %i\n", i);
	printf("Tag 1:\n");
	printf("\tname: [%s]\n", model->tags[0].name);
	printf("\n");
	#endif

	/* SURFACES */
	md3_load_surfaces(model, texture_path_prefix);

	#ifdef MD3_DEBUG
	printf("Surfaces loaded: %i\n", model->num_surfaces);
	{
		struct md3_surface_t* sptr = model->surface_ptr;
		int sn = 0;
		while (sptr) {
			printf("Surface %i:\n", sn);
			printf("\tname: [%s]\n", sptr->name);
			printf("\tflags: %i\n", sptr->flags);
			printf("\tnum_frames: %i\n", sptr->num_frames);
			printf("\tnum_shaders: %i\n", sptr->num_shaders);
			printf("\tnum_verts: %i\n", sptr->num_verts);
			printf("\tnum_triangles: %i\n", sptr->num_triangles);
			printf("\tofs_triangles: %i\n", sptr->ofs_triangles);
			printf("\tofs_shaders: %i\n", sptr->ofs_shaders);
			printf("\tofs_st: %i\n", sptr->ofs_st);
			printf("\tofs_xyznormal: %i\n", sptr->ofs_xyznormal);
			printf("\tofs_end: %i\n", sptr->ofs_end);
			sptr = sptr->next;
			++sn;
		}
	}
	#endif
	
	/* close the file */
	fclose(model->fptr);
	
	/* initialize the animation state */
	model->anim_state.anim_info = NULL;
	model->anim_state.id = 0;
	model->anim_state.frame = 0;
	model->anim_state.next_frame = 0;
	model->anim_state.t = 0;
	model->anim_state.last_time = 0.0;
	model->anim_state.animated = 0;
	
	/* initialize custom rotation */
	model->rot[0] = 0.0f;
	model->rot[1] = 0.0f;
	model->rot[2] = 0.0f;
	model->scale_factor = 1.0f;
	
	return model;
}


static void md3_load_surfaces(struct md3_model_t* model, char* texture_path_prefix) {
	struct md3_surface_t* sptr = NULL;
	int surface_base = 0;
	int surface_start = 0;
	int shader_base = 0;
	int vert_base = 0;
	int surface = 0;
	int i = 0;
	char text_file[1024];
	
	/* assume there is at least 1 surface */
	model->surface_ptr = (struct md3_surface_t*)malloc(sizeof(struct md3_surface_t));
	memset(model->surface_ptr, 0, sizeof(struct md3_surface_t));
	sptr = model->surface_ptr;
	
	/* calculate where surfaces start */
	surface_base = model->ofs_surfaces;
	surface_start = surface_base;
	
	/* iterate through each surface */
	for (; surface < model->num_surfaces; ++surface) {
		/* load in surface data */
		fseek(model->fptr, surface_start, SEEK_SET);
		fread(&sptr->ident, ((sizeof(int) * 11) + (sizeof(char) * MAX_QPATH)), 1, model->fptr);
		
		/* load shaders */
		sptr->shader = (struct md3_shader_t*)malloc(sizeof(struct md3_shader_t) * sptr->num_shaders);
		shader_base = (surface_start + sptr->ofs_shaders);
		for (i = 0; i < sptr->num_shaders; ++i) {
			fseek(model->fptr, (shader_base + (i * MD3_SIZEOF_SHADER)), SEEK_SET);
			fread(sptr->shader + i, MD3_SIZEOF_SHADER, 1, model->fptr);
			
			/*
			 *	For some reason or another shader names may start with a '\0'.
			 *	If they do, replace it with 'm'.
			 */
			if (sptr->shader[i].name[0] == '\0')
				sptr->shader[i].name[0] = 'm';
			
			sptr->shader[i].gl_text_id = NULL;
			sptr->shader[i].gl_text_bound = NULL;
			
			/* load the texture */
			if (!texture_path_prefix) {
				/* using the skin config stuff */
				sptr->shader[i].texture = NULL;
			} else {
				/* use the texture within the file */			
				str_to_lower(sptr->shader[i].name);
				sprintf(text_file, "%s%s", (texture_path_prefix ? texture_path_prefix : ""), sptr->shader[i].name);
				format_path_for_os(text_file);
				sptr->shader[i].texture = world_texture_cached(g_world, text_file, &sptr->shader[i]);
				if (!sptr->shader[i].texture) {
					/* if texture not already cached, load it */
					sptr->shader[i].texture = load_tga(text_file);
					
					if (sptr->shader[i].texture) {
						/* register it with the world */
						world_add_texture(g_world, sptr->shader[i].texture, text_file, &sptr->shader[i]);
	
						#ifdef MD3_DEBUG
						printf("Texture \"%s\" loaded.\n", text_file);
						#endif
					}
				} else {
					/* tell the world we need to use this texture */
					world_using_texture(g_world, sptr->shader[i].texture);
					
					/* if the texture id is not -1 then it has already been bound in GL */
	
					#ifdef MD3_DEBUG
					printf("Texture \"%s\" loaded (cached).\n", text_file);
					#endif
				}
						
				if (!sptr->shader[i].texture)
					printf("Error: Unable to load texture \"%s\".\n", text_file);
			}
		}
		
		/* load triangles */
		LOAD_ARRAY(sptr->triangle, struct md3_triangle_t, sptr->num_triangles, surface_start, sptr->ofs_triangles, model->fptr);
		model->total_triangles += sptr->num_triangles;
			
		/* load texture coordinates */
		LOAD_ARRAY(sptr->st, struct md3_texcoord_t, sptr->num_verts, surface_start, sptr->ofs_st, model->fptr);
			
		/* load verticies */
		sptr->vertex = (struct md3_vertex_t*)malloc(sizeof(struct md3_vertex_t) * (sptr->num_verts * sptr->num_frames));
		vert_base = (surface_start + sptr->ofs_xyznormal);
		for (i = 0; i < (sptr->num_frames * sptr->num_verts); ++i) {
			fseek(model->fptr, (vert_base + (i * MD3_SIZEOF_VERTEX)), SEEK_SET);
			fread(sptr->vertex + i, MD3_SIZEOF_VERTEX, 1, model->fptr);

			/* Calculate xyz normal */
			md3_make_normal(sptr->vertex + i);
		}
		
		/* go to start of next surface */
		if ((surface + 1) < model->num_surfaces) {
			surface_start += sptr->ofs_end;
			sptr->next = (struct md3_surface_t*)malloc(sizeof(struct md3_surface_t));
			memset(sptr->next, 0, sizeof(struct md3_surface_t));
			sptr = sptr->next;
		}
	}
}


/*
 *	Unload a model and deallocate memory used by the structures.
 */
void md3_unload_model(struct md3_model_t* model) {
	struct md3_surface_t* next_surface = NULL;
	int i = 0;

	if (!model)
		return;
	
	/* tell the world */
	world_del_model(g_world, model);

	/* free frames */
	free(model->frames);
		
	/* free tags */
	free(model->tags);
		
	/* free surfaces */
	while (model->surface_ptr) {
		next_surface = model->surface_ptr->next;
		
		/* unload textures - tell the world we no longer need them */
		for (i = 0; i < model->surface_ptr->num_shaders; ++i)
			world_not_using_texture(g_world, model->surface_ptr->shader[i].texture);
		
		/* free shaders */
		free(model->surface_ptr->shader);
			
		/* free triangles */
		free(model->surface_ptr->triangle);
			
		/* free vertexes */
		free(model->surface_ptr->vertex);

		/* free surface */
		free(model->surface_ptr);
		
		model->surface_ptr = next_surface;
	}
	
	/* free the model name */
	free(model->model_name);
	
	/* free array of links */
	free(model->links);
	
	/* free model */
	free(model);
	
}


/*
 *	This code is modified from the Quake3 source code base.
 *	File: code/q3map/misc_model.c:InsertMD3Model()
 *
 *	It will decode the xyz normal short in the form:
 *		8 significant bits = lat
 *		8 least sig bits = lng
 *	To x, y, z coordinates.
 */
static void md3_make_normal(struct md3_vertex_t* vertex) {
	float lat, lng;
	
	/* decode */
	lat = ((vertex->normal >> 8) & 0xFF);
	lng = (vertex->normal & 0xFF);
	lat *= (PI / 128);
	lng *= (PI / 128);

	vertex->normalxyz[0] = cos(lat) * sin(lng);
	vertex->normalxyz[1] = sin(lat) * sin(lng);
	vertex->normalxyz[2] = cos(lng);
}


/*
 *	Load a full model.
 *
 *	The file should consist of 1 line per body part in the form:
 *		name file
 *		name file
 *		...
 *
 *	Where "name" is the body part name and "file" is a relative
 *	path from this file to the md3 file.
 *
 *	Returns root model loaded.
 */
struct md3_model_t* load_model(char* file) {
	FILE* fptr = NULL;
	char* path = NULL;
	char buf[1024];
	char name[64];
	char mfile[1024];
	char line_type;
	struct md3_model_t* model = NULL;
	char* text_path = NULL;
	int root_model = 1;
	
	struct md3_model_t* models[10] = {0};
	int loaded = 0;
	int i = 0;
	
	fptr = fopen(file, "r");
	if (!fptr)
		return NULL;
	
	path = get_path(file, 1);
	text_path = get_path(path, 1);
	
	/*
	 *	each line in the file is the
	 *	path to a md3 model file
	 *	relative to this file
	 *
	 *	In the form:
	 *		name file
	 */
	while (!feof(fptr)) {
		line_type = fgetc(fptr);
		fgetc(fptr);	/* next char always a space */

		if (!fgets(buf, 1024, fptr))
			break;
		strip_lf(buf);

		if (line_type == 'm') {
			/* model line */		
			sscanf(buf, "%s %s", name, mfile);
		
			/* make the relative path from this binary */
			sprintf(buf, "%s%s", (path ? path : ""), mfile);
		
			/* load the model */
			model = md3_load_model(buf, NULL);
		
			if (!model)
				continue;
		
			/* assign our custom name to this model */
			model->model_name = strdup(name);
			if (!strcmp(name, "upper"))
				model->body_part = MD3_TORSO;
			else if (!strcmp(name, "lower"))
				model->body_part = MD3_LEGS;
			else if (!strcmp(name, "head"))
				model->body_part = MD3_HEAD;
		
			/* add the model to the world */
			world_add_model(g_world, model, root_model);
					
			/* link this model to the others */
			for (i = 0; i < loaded; ++i) {
				if (models[i])
					md3_link_models(models[i], model);
			}
					
			/* keep track of this model */
			models[loaded] = model;
			++loaded;
				
			/* only the first model in the file is considered the root model */
			root_model = 0;
		} else if (line_type == 'a') {
			/* Load the animation data for this model */

			/* make the relative path from this binary */
			strcpy(name, buf);
			sprintf(buf, "%s%s", (path ? path : ""), name);

			load_anim_file(buf, g_world->anims);
		} else if (line_type == 't') {		
			/* Load textures for this model */
			char model[64];
			char surface[64];
			int m = 0;
			enum MD3_BODY_PARTS model_type = 0;
				
			sscanf(buf, "%s %s %s", model, surface, name);
			sprintf(mfile, "%s%s", (text_path ? text_path : ""), name);
			
			if (!strcmp(model, "upper"))
				model_type = MD3_TORSO;
			else if (!strcmp(model, "lower"))
				model_type = MD3_LEGS;
			else if (!strcmp(model, "head"))
				model_type = MD3_HEAD;
			
			/* Get the model this texture belongs to */
			for (; m < loaded; ++m) {
				if (models[m]->body_part == model_type) {
					/* this is the model - find the surface */
					load_texture_for_model(models[m], mfile, surface);
					break;
				}
			}
			
		}
	}

	if (path)
		free(path);
	if (text_path)
		free(text_path);
	
	fclose(fptr);
	
	return *models;
}


/*
 *	Unload a full model starting at the given root.
 *
 *	If unload_weapon_link is 0 the weapon link will not be unloaded.
 */
void unload_model(struct md3_model_t* model, int unload_weapon_link) {
	int link = 0;
	
	if (!model)
		return;
	
	if ((!unload_weapon_link) && (model->body_part == MD3_WEAPON))
		/* we do not want to unload the weapon, just skip it */
		return;
	
	/* First unload all the links to this model */
	for (; link < model->num_tags; ++link)
		unload_model(model->links[link], unload_weapon_link);
	
	/* Unload the model - md3_unload_model() will tell the world for us */
	md3_unload_model(model);
}


/*
 *	Load a weapon and add to the world.
 */
struct md3_model_t* load_weapon(char* path, char* texture_path_prefix) {
	struct md3_model_t* w = md3_load_model(path, texture_path_prefix);
	if (!w)
		return NULL;
	w->model_name = strdup("weapon");
	w->body_part = MD3_WEAPON;
	world_link_model(g_world, w);
	world_add_model(g_world, w, 0);
	return w;
}


/*
 *	Unload a weapon and delete from the world.
 */
void unload_weapon(struct md3_model_t* w) {
	world_delink_model(g_world, w);
	world_del_model(g_world, w);
	md3_unload_model(w);
}
	
	
/*
 *	Link the child to the parent.
 *
 *	Return number of links made.
 */
int md3_link_models(struct md3_model_t* parent, struct md3_model_t* child) {
	int ctag;
	int ptag;
	int links = 0;
	
	if (parent == child)
		/* a node can not be linked to itself */
		return 0;
	
	/* iterate through each tag in the child */
	for (ctag = 0; ctag < child->num_tags; ++ctag) {
		/* find this tag in parent */
		for (ptag = 0; ptag < parent->num_tags; ++ptag) {
			if (!strcmp(parent->tags[ptag].name, child->tags[ctag].name)) {
				parent->links[ptag] = child;
				++links;
				break;
			}
		}
	}
	
	return links;
}


/*
 *	Delink the child from the parent.
 *
 *	Return 1 if the link was broken.
 */
int md3_unlink_models(struct md3_model_t* parent, struct md3_model_t* child) {
	int link = 0;

	if (parent == child)
		/* a node can not be linked to itself */
		return 0;

	for (; link < parent->num_tags; ++link) {
		if (parent->links[link] == child) {
			/* delink this one */
			parent->links[link] = NULL;
			return 1;
		}
	}
	return 0;
}


/*
 *	Load the animation data into the array pointed to by anims.
 *	Anims should have exactly MD3_MAX_ANIMS elements.
 *
 *	Returns number of animations loaded.
 */
static int load_anim_file(char* file, struct md3_anim_t* aptr) {
	FILE* fptr = NULL;
	char buf[1024] = {0};
	struct md3_anim_names_t* anim_struct;
	struct md3_anim_t tmp_anim;
	int loaded = 0;
	int id = 0;
	int legs_offset = 0;
		
	fptr = fopen(file, "r");
	if (!fptr)
		return 0;
	
	memset(aptr, 0, (sizeof(struct md3_anim_t) * MD3_MAX_ANIMS));
	
	while (!feof(fptr)) {
		fgets(buf, 1023, fptr);
		
		if (feof(fptr))
			break;
		
		/* all animations start with a number, so skip non numeric lines */
		if (!IS_NUMERIC(*buf))
			continue;
		
		memset(&tmp_anim, 0, sizeof(struct md3_anim_t));

		/*
		 *	Try to read this line in.
		 *
		 *	Format:
		 *		<first frame> <num frames> <looping frames> <frames per second> // <animation name>
		 */
		sscanf(buf, "%i %i %i %i // %s ",
			&tmp_anim.first_frame,
			&tmp_anim.frames,
			&tmp_anim.loop,
			&tmp_anim.fps,
			tmp_anim.name
		);
		
		anim_struct = get_animation_by_name(tmp_anim.name);
		if (!anim_struct) {
			/* ERROR */
			printf("*** ERROR: Animation sequence \"%s\" is not valid.\n", tmp_anim.name);
			continue;
		}
		
		id = anim_struct->id;
		
		/* Copy it into the correct place in the array */
		memcpy(&aptr[id], &tmp_anim, sizeof(struct md3_anim_t));
		
		/*
		 *	loop is the last X frames to be looped,
		 *	so set it to where the animation should
		 *	begin again.
		 */
		aptr[id].last_frame = ((aptr[id].first_frame + aptr[id].frames) - 1);
		aptr[id].loop = (aptr[id].last_frame - aptr[id].loop);
		aptr[id].anim_id = id;
		
		/*
		 *	For some reason or another the frame numbers
		 *	for the legs continue to grow from the frame
		 *	numbers from the body.
		 *	Since these are different models, the legs
		 *	animations actually start where the legs+body
		 *	animations stop.
		 *	So we have to shift all the leg animation frames
		 *	down by the difference offset.
		 */
		if (id >= LEGS_WALKCR) {
			if (!legs_offset)
				legs_offset = (aptr[LEGS_WALKCR].first_frame - aptr[TORSO_GESTURE].first_frame);
			
			aptr[id].first_frame -= legs_offset;
			aptr[id].last_frame -= legs_offset;
			aptr[id].loop -= legs_offset - 1;
		}
		
		++loaded;
	}
	
	fclose(fptr);
	
	return loaded;
}
	

/*
 *	Load a texture for a specific surface for the given model.
 */
static void load_texture_for_model(struct md3_model_t* model, char* texture, char* surface) {
	struct md3_surface_t* sptr = model->surface_ptr;
	while (sptr) {
		if (!strcmp(sptr->name, surface)) {
			/* this is the surface - load the texture here */
			sptr->shader[0].texture = world_texture_cached(g_world, texture, &sptr->shader[0]);
			
			if (!sptr->shader[0].texture) {
				/* if texture not already cached, load it */
				format_path_for_os(texture);
				sptr->shader[0].texture = load_tga(texture);
								
				if (sptr->shader[0].texture) {
					/* register it with the world */
					world_add_texture(g_world, sptr->shader[0].texture, texture, &sptr->shader[0]);
				
					#ifdef MD3_DEBUG
					printf("Texture \"%s\" loaded.\n", texture);
					#endif
				} else
					printf("Error: Unable to load texture \"%s\" - not found.\n", texture);
			} else {
				/* tell the world we need to use this texture */
				world_using_texture(g_world, sptr->shader[0].texture);
								
				/* if the texture id is not -1 then it has already been bound in GL */
				
				#ifdef MD3_DEBUG
				printf("Texture \"%s\" loaded (cached).\n", texture);
				#endif
			}

			if (!sptr->shader[0].texture)
				printf("Error: Unable to load texture \"%s\".\n", texture);

			return;
		}
		sptr = sptr->next;
	}
	printf("Error: Failed to load texture \"%s\" to model %s surface %s.\n", texture, model->model_name, surface);
}


/*
 *	Return the assoicated animation structure by the id.
 */
struct md3_anim_names_t* get_animation_by_id(enum MD3_ANIMATIONS id) {
	int i = 0;
	for (; i < MD3_MAX_ANIMS; ++i)
		if (anim_structs[i].id == id)
			return &(anim_structs[i]);
	return NULL;
}


/*
 *	Return the assoicated animation structure by the name.
 */
struct md3_anim_names_t* get_animation_by_name(char* name) {
	int i = 0;
	for (; i < MD3_MAX_ANIMS; ++i)
		if (!strcmp(anim_structs[i].name, name))
			return &(anim_structs[i]);
	return NULL;
}


/*
 *	Load a light model.
 */
void load_light_model(char* file, int light_num) {
	struct md3_model_t* m = NULL;
	char buf[64] = {0};
	
	m = md3_load_model(file, "../");
	
	if (!m)
		return;
	
	m->body_part = MD3_LIGHT;
	
	sprintf(buf, "Light %i", light_num);
	m->model_name = strdup(buf);
	
	/* manually kill tags so no links are possible */
	m->num_tags = 0;
	
	world_add_model(g_world, m, 0);
	
	g_world->light[light_num].model = m;
}
