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
 
#include <qgl.h>
#include <qevent.h>
#include <math.h>
#include "quaternion.h"
#include "render.h"
#include "world.h"
#include "util.h" 
#include "gui.h"
#include "gl_widget.h"
#include "md3_parse.h"


gl_widget::gl_widget(int argc, char** argv, const QGLFormat& format, QWidget* parent, const char* name, const QGLWidget* shareWidget, WFlags f)
	: QGLWidget(format, parent, name, shareWidget, f) {
		
	this->argc = argc;
	this->argv = argv;
	
	/* setup the mouse */
	this->mouse.depressed = 0;
	this->mouse.old_pos[0] = 0;
	this->mouse.old_pos[1] = 0;
	this->mouse.sensitivity = 2.0f;
	
	/* initialize frame rate stuff */
	this->next_frame_msec = 0;
	this->last_msec = 0;
	this->frames = 0;
	this->frame_skip = 0;
	this->max_frame_rate = MAX_FRAMERATE;
	
	/* enable double buffering */
	this->setAutoBufferSwap(1);
		
	/* set selected object to nothing */
	this->selected_object = NULL;

	/*
	 *	Setup the event timer - this will give us render calls on idle cycles
	 *
	 *	Optimization.
	 *
	 *	A one millisecond delay always occurs between rendering frames.
	 *	This is to minimize CPU usage by constantly executing the process.
	 *	This also means that a maximum of 1000 frames can be rendered per second,
	 *	which is fine since the monitor probably can not handle over 100.
	 */
	this->timer = new QTimer(this);
	this->timer->start(1);
	QObject::connect(this->timer, SIGNAL(timeout()), this, SLOT(idle_cycle()));
}


gl_widget::~gl_widget() {
	/*
	 *	Delete the bounding box list from GL.
	 */
	glDeleteLists(g_world->gl_box_id, 1);
	glDeleteLists(g_world->gl_plane_id, 1);
}

/*
 *	Called by the timer constantly.
 */
void gl_widget::idle_cycle() {
	char buf[64] = {0};
	double now = get_time_in_ms();
	
	if (now >= this->next_frame_msec) {
		/* one second has elapsed */

		/* update GUI widget with frame rate */
		sprintf(buf, "%i Frames Per Second     %i Skip", this->frames, this->frame_skip);		
		g_gui->fps->setText(buf);
		
		this->next_frame_msec = (now + 1000.0);
		this->frames = 1;
		this->frame_skip = 0;
	} else {
		if ((now - this->last_msec) <= ((1000.0 / this->max_frame_rate) - 1)) {
			/*
			 *	Optimization.
			 *
			 *	If the last frame was rendered less than
			 *		1 millisecond / maximum FPS
			 *	milliseconds ago then skip the rendering of
			 *	this frame.
			 *	This allows the FPS to be capped at
			 *	max_frame_rate so extra CPU is not wasted
			 *	on rerendering virtually the same scene.
			 *
			 *	On my AMD64 machine this reduces the
			 *	FPS from 2800 to max_frame_rate.
			 *
			 *	Note the -1 in the equation is for the timer delay of 1ms.
			 */
			this->frame_skip++;
			return;
		}
		
		this->last_msec = now;
		this->frames++;
	}
	
	/* rerender */
	this->updateGL();
}


/*
 *	Initialize OpenGL and any Glut functionality.
 */
void gl_widget::initializeGL() {
	/* enable gl options */
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	
	/* clear the stencil buffer */
	glClearStencil(0);

	if (WORLD_IS_SET(ENGINE_LIGHTING))
		glEnable(GL_LIGHTING);
	
	/*
	 *	Optimization.
	 *	We want every optimization possible,
	 *	so we don't draw the back faces.
	 */
	glCullFace(GL_FRONT);

	glShadeModel(GL_SMOOTH);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* set texture options */
	#if 0
	this is done when a texture is bound
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	#endif
	
	/* set background color */
	glClearColor(g_world->env.bg_rgba[0], g_world->env.bg_rgba[1], g_world->env.bg_rgba[2], g_world->env.bg_rgba[3]);
	
	/* make the bounding box */
	g_world->gl_box_id = make_bounding_box();
	g_world->gl_plane_id = make_tes_plane();
	
	/* register the mirror walls */
	{
		/* floor */
		double clip[] = { 0.0, -1.0, 0.0, 0.0 };
		float origin[] = { -50.0f, -25.0f, -50.0f };
		float axis[] = { 0.0f, 0.0f, 0.0f };
		float normal[] = { 0.0f, -1.0f, 0.0f };
		float angle = 0.0f;
		world_register_mirror(g_world, clip, origin, normal, axis, angle);
	}
	{
		/* right wall */
		double clip[] = { 0.0, -1.0, 0.0, 0.0 };
		float origin[] = { -50.0f, -50.0f, -75.0f };
		float axis[] = { 1.0f, 0.0f, 0.0f };
		float normal[] = { 0.0f, 0.0f, -1.0f };
		float angle = 90.0f;
		world_register_mirror(g_world, clip, origin, normal, axis, angle);
	}
	{
		/* back wall */
		double clip[] = { 0.0, -1.0, 0.0, 0.0 };
		float origin[] = { -75.0f, -50.0f, -50.0f };
		float axis[] = { 0.0f, 0.0f, 1.0f };
		float normal[] = { -1.0f, 0.0f, 0.0f };
		float angle = -90.0f;
		world_register_mirror(g_world, clip, origin, normal, axis, angle);
	}
}

#include "accum.h"
/*
 *	Called when the widget is resized.
 */
void gl_widget::resizeGL(int w, int h) {
	/* setup viewport */
	glViewport(0, 0, w, h);

	/* setup the projection matrix */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(g_world->env.fov, ((float)w / (float)h), g_world->env.vnear, g_world->env.vfar);
	
	this->width = w;
	this->height = h;

	/* switch back and initialize the model view matrix */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


/*
 *	Called to renderer the scene.
 */
void gl_widget::paintGL() {
	/* clear color and depth buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	/* set matrix mode to model view */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* initialize names buffer */
	glInitNames();
	glPushName(0);
	glLoadName(ETHER);

	/* setup camera */
	gluLookAt(g_world->camera.r * cos(g_world->camera.prot * deg) * cos(g_world->camera.trot * deg),
				g_world->camera.r * sin(g_world->camera.prot * deg),
				g_world->camera.r * cos(g_world->camera.prot * deg) * sin(g_world->camera.trot * deg),
				g_world->camera.center_xyz[0],
				g_world->camera.center_xyz[1],
				g_world->camera.center_xyz[2],
				0,1,0);
	
	/* apply the light sources */
	if (WORLD_IS_SET(ENGINE_LIGHTING)) {
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		apply_light(GL_LIGHT0, &g_world->light[0]);
	} else
		glDisable(GL_LIGHTING);
	
	/* apply textures */
	if (WORLD_IS_SET(RENDER_TEXTURES))
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);

	render();	
}


/*
 *	Called when a mouse button is depressed.
 */
void gl_widget::mousePressEvent(QMouseEvent* e) {
	int x = e->x();
	int y = e->y();
	
	if (e->button() == LeftButton) {
		this->mouse.old_pos[0] = x;
		this->mouse.old_pos[1] = y;
		this->mouse.depressed = 1;
	} else {
		select_object(x, y);
	}
}


/*
 *	Called when a mouse button is released.
 */
void gl_widget::mouseReleaseEvent(QMouseEvent* e) {
	this->mouse.depressed = 0;
	return;
	e = NULL;	/* get rid of unused variable warning */
}


/*
 *	Called when the mouse moves over the widget.
 */
void gl_widget::mouseMoveEvent(QMouseEvent* e) {
	int x = e->x();
	int y = e->y();
	char buf[64] = {0};
	
	if (!this->mouse.depressed)
		/* left mouse button not pressed */
		return;

	if (this->selected_object && (this->selected_object->body_part == MD3_LIGHT)) {
		/*
		 *	Light
		 *
		 *	Update rotation angles based on moved mouse
		 *	position since button first depressed.
		 */
		g_world->light[0].trot -= ((x - this->mouse.old_pos[0]) / (5 / this->mouse.sensitivity));
		g_world->light[0].prot -= ((y - this->mouse.old_pos[1]) / (5 / this->mouse.sensitivity));
		
		if (g_world->light[0].trot > 360.0f)	g_world->light[0].trot -= 360.0f;
		if (g_world->light[0].trot < 0.0f)		g_world->light[0].trot += 360.0f;
		if (g_world->light[0].prot > 90.0f)		g_world->light[0].prot = 90.0f;
		if (g_world->light[0].prot < -90.0f)	g_world->light[0].prot = -90.0f;
	
		/* calculate the position of the light */
		g_world->light[0].position[0] = g_world->light[0].r * cos(g_world->light[0].prot * deg) * cos(g_world->light[0].trot * deg);
		g_world->light[0].position[1] = g_world->light[0].r * sin(g_world->light[0].prot * deg);
		g_world->light[0].position[2] = g_world->light[0].r * cos(g_world->light[0].prot * deg) * sin(g_world->light[0].trot * deg);
		g_world->light[0].position[3] = 1;

		/*
		 *	The next few lines are crude hacks for the rotation of the flashlight.
		 *	I don't think this can possibly be the correct way to do it but it'll
		 *	have to do for now.
		 *
		 *	Don't read it.
		 */
		
		/* calculate the angle the flashlight needs to be rotated to face the origin */
		g_world->light[0].dir_prot = (-1 * (acos((double)g_world->light[0].position[1] / (double)g_world->light[0].r) * (180.0/PI) - 90.0));

		/* if x is negative we need to multiply z by -1 */
		double z = (g_world->light[0].position[2] * SIGN(g_world->light[0].position[0]));
		g_world->light[0].dir_trot = ((acos(z / (double)g_world->light[0].r) * (180.0/PI)));
		
		/* depending on the sign of x a final rotation must be made so lens faces origin not the end */
		if (SIGN(g_world->light[0].position[0]) == 1) {
			g_world->light[0].dir_trot -= 90.0;
		} else {
			g_world->light[0].dir_trot += 90.0;
			g_world->light[0].dir_prot *= -1;
		}
	} else {
		/*
		 *	Camera
		 *
		 *	Update rotation angles based on moved mouse
		 *	position since button first depressed.
		 */
		g_world->camera.trot += ((x - this->mouse.old_pos[0]) / (5 / this->mouse.sensitivity));
		g_world->camera.prot += ((y - this->mouse.old_pos[1]) / (5 / this->mouse.sensitivity));
	
		if (g_world->camera.trot > 360.0f)		g_world->camera.trot -= 360.0f;
		if (g_world->camera.trot < 0.0f)		g_world->camera.trot += 360.0f;
		if (g_world->camera.prot > 90.0f)		g_world->camera.prot = 90.0f;
		if (g_world->camera.prot < -90.0f)		g_world->camera.prot = -90.0f;
	
		/* update GUI widget with camera position */
		sprintf(buf, "Camera Position:   x = %3.2f   y = %3.2f   z = %3.2f",
					g_world->camera.r * cos(g_world->camera.prot * deg) * cos(g_world->camera.trot * deg),
					g_world->camera.r * sin(g_world->camera.prot * deg),
					g_world->camera.r * cos(g_world->camera.prot * deg) * sin(g_world->camera.trot * deg)
		);
		g_gui->cam_pos->setText(buf);
	}

	/* save the current mouse position */
	this->mouse.old_pos[0] = x;
	this->mouse.old_pos[1] = y;
	
	this->updateGL();
}


void gl_widget::select_object(int x, int y) {
	unsigned int buf[128];
	unsigned int hits;
	unsigned int target = ETHER;
	int viewport[4];
	
	int reenable_flags = 0;
	
	/*
	 *	We do not need multipass rendering when doing selection.
	 *	Turn off AA and Depth of Field.
	 */
	if (WORLD_IS_SET(ENGINE_AA))
		reenable_flags |= ENGINE_AA;
	if (WORLD_IS_SET(ENGINE_DEPTH_OF_FIELD))
		reenable_flags |= ENGINE_DEPTH_OF_FIELD;
	world_set_options(g_world, 0, reenable_flags);	

	glSelectBuffer(128, buf);					/* set buffer to local */
	glGetIntegerv(GL_VIEWPORT, viewport);		/* get current viewport configuration */
	glMatrixMode(GL_PROJECTION);				/* change to projection matrix */

	glPushMatrix();
	glRenderMode(GL_SELECT);
	glLoadIdentity();

	gluPickMatrix(x, viewport[3]-y, 2, 2, viewport);	/* define selection area */

	gluPerspective(g_world->env.fov, ((float)this->width / (float)this->height), g_world->env.vnear, g_world->env.vfar);

	this->updateGL();

	hits = glRenderMode(GL_RENDER);
  
	glMatrixMode(GL_PROJECTION);	/* go back to projection since gl_display set to modelview */
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	if (hits > 0) {
		/* minimum selection of minimum distances */
		unsigned int i = 0;
		unsigned int min = buf[1];
		target = buf[3];

		for (; i < hits; ++i) {
			if (buf[(i * 4) + 1] < min) {
				min = buf[(i * 4) + 1];
				target = buf[(i * 4) + 3];
			}
		}
	}

	/* if we had a previously selected object turn off rendering its bounding box */
	if (this->selected_object)
		this->selected_object->draw_bounding_box = 0;

	/* save the selected object */
	if (target == ETHER)
		this->selected_object = NULL;
	else {
		this->selected_object = world_get_model_by_type((enum MD3_BODY_PARTS)target);
		
		/* turn on rendering this objects bounding box */
		if (this->selected_object)
			this->selected_object->draw_bounding_box = 1;
	}
		
	/* tell the global GUI widget about the selection */
	g_gui->srot->object_selected(this->selected_object);
	
	/* Reenable the flags we turned off */
	world_set_options(g_world, reenable_flags, 0);
}
