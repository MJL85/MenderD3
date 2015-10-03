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
 
#ifndef _GL_WIDGET_H
#define _GL_WIDGET_H

#include <qgl.h>
#include <qevent.h>
#include <qtimer.h>
#include "definitions.h"
#include "world.h"

/*
 *	The maximum number of frames to be rendered.
 *	This is just an estimation and it may go slightly over.
 */
#define MAX_FRAMERATE		100

class gl_widget : public QGLWidget {
	Q_OBJECT
	
	public:
		gl_widget(int argc, char** argv, const QGLFormat& format, QWidget* parent = 0, const char* name = 0, const QGLWidget* shareWidget = 0, WFlags f = 0);
		~gl_widget();
		struct md3_model_t* selected_object;					/* currently selected object	*/
		
	public slots:
		void idle_cycle();		
	
	private:
		/* functions */
		void gl_widget::select_object(int x, int y);
		
		/* data */
		QTimer* timer;	
	
		int argc;
		char** argv;
	
		struct mouse_t mouse;
		int width;
		int height;
		
		/* frame rate information */
		double next_frame_msec;
		double last_msec;
		int frames;
		int frame_skip;
		
		int max_frame_rate;							/* maximum possible FPS */
		
	protected:
		void initializeGL();
		void resizeGL(int w, int h);
		void paintGL();
	
		void mousePressEvent(QMouseEvent* e);
		void mouseReleaseEvent(QMouseEvent* e);
		void mouseMoveEvent(QMouseEvent* e);
	
	
};

#endif /* _GL_WIDGET_H */
