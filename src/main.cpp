/*
 *	MenderD3 - An MD3 Viewer
 *
 *	Written by:
 *		Michael Laforest
 *		Berek Bryan
 *
 *	Copyright 2006
 *
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
#include "definitions.h"
#include "util.h"
#include "md3_parse.h"
#include "world.h"
#include "gui.h"

void a(struct md3_model_t* m) {
	int i = 0;
	for (; i < m->num_tags; ++i) {
		printf("[%s] -> [%s]\n",
				m->surface_ptr->name,
				m->links[i] ? m->links[i]->surface_ptr->name : "none"
		);
		if (m->links[i])
			a(m->links[i]);
	}
}

int main(int argc, char** argv) {
	/* initialize the world */
	g_world = world_init();
	
	/* load the full model */
	//load_model("../models/sarge.mod");
	//load_weapon("../models/weapons2/rocketl/rocketl.md3", "../");
	/*struct md3_model_t* m = world_get_model_by_type(MD3_WEAPON);
	unload_weapon(m);*/

	//world_set_options(g_world, 0, RENDER_TEXTURES);
	
	set_model_animation(LEGS_IDLE);
	set_model_animation(TORSO_STAND);
	
	/* start the GUI */
	gui_start(argc, argv);
	
	/* free the world */
	world_free(g_world);
	
	return 0;	
}
