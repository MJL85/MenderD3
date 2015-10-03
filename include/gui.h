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

#ifndef _GUI_H
#define _GUI_H

#include <qapplication.h>
#include <qframe.h>
#include <qlayout.h>
#include <qgl.h>
#include <qgroupbox.h> 
#include <qlabel.h> 
#include <qpushbutton.h> 
#include <qlistbox.h> 
#include <qbuttongroup.h>
#include <qradiobutton.h> 
#include <qslider.h> 
#include <qmenubar.h> 
#include <qpopupmenu.h> 
#include <qobject.h> 
#include <qcombobox.h> 
#include <qcheckbox.h> 
#include <qstring.h>
#include <qfiledialog.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include "md3_parse.h"
#include "gl_widget.h"
#include "definitions.h"

#define DEFAULT_LOAD_MODEL					"../models/sarge.mod"
#define DEFAULT_LOAD_WEAPON					"../models/weapons2/rocketl/rocketl.md3"


#define CREDITS_STR			"MenderD3 " MENDERD3_VERSION "\n"		\
							"Written by: Michael Laforest\n"		\
							"        and Willard Bryan\n"			\
							"Copyright 2006, GPL"
							
#define MODELS_PATH			"../models/"
#define WEAPONS_PATH		"../models/weapons2/"
#define DEFAULT_LOAD_LIGHT0	"../models/flashlight/flashlight.md3"

#define MAX_MENU_WIDTH		300


/* global to GUI widget */
extern class gui_widget* g_gui;

/*
 *	The kinds of loadable models to distinguish
 *	between model_widget instances.
 */
enum loadable_types {
	MODEL_TYPE,
	WEAPON_TYPE
};

int gui_start(int argc, char** argv);

class model_widget : public QGroupBox {
	Q_OBJECT
	
	/* gui_widget::reset() will access our model */
	friend class gui_widget;
	
	public:
		model_widget(enum loadable_types mtype, int strips, Orientation orientation, const QString& title, QWidget* parent = 0, const char * name = 0);
	
	public slots:
		void load();
	
	private:
		enum loadable_types type;
		QPushButton* open;
		struct md3_model_t* model;
};


class animate_widget : public QGroupBox {
	Q_OBJECT
	
	public:
		animate_widget(int strips, Orientation orientation, const QString& title, QWidget* parent = 0, const char * name = 0);
	
		void reset_animation();
	
	public slots:
		void tl_clicked();
		void t_clicked();
		void l_clicked();
		void reset_clicked();
		void tls_clicked();
		void ts_clicked();
		void ls_clicked();
		void stopall_clicked();
		void loop_checked();
	
	private:
		QGridLayout* ani_grid;		
		
		QComboBox* tl_CB;
		QComboBox* t_CB;
		QComboBox* l_CB;
		
		QPushButton* tl_start;
		QPushButton* t_start;
		QPushButton* l_start;		  
		QPushButton* tl_stop;
		QPushButton* t_stop;
		QPushButton* l_stop;
		QPushButton* stopall;
		QPushButton* reset;
		
		QCheckBox* loop_check;
		
		QWidget* base;
};

class opt_widget : public QGroupBox {
	Q_OBJECT
	
	public:
		opt_widget(int strips, Orientation orientation, const QString& title, QWidget* parent = 0, const char * name = 0);
		
	public slots:
		void wire_checked();
		void noText_checked();
		void mirror_checked();
		void light_checked();
		void nointerp_checked();
		void zoom_changed(int zfactor);
		void vlights_checked();
		void resetLights_pushed();

	private:
		
		QGridLayout* opt_grid;
		
		QLabel* zLabel;
		
		QCheckBox* wireCB;
		QCheckBox* noTextCB;
		QCheckBox* mirrorCB;
		QCheckBox* lightCB;
		QCheckBox* view_lightsCB;
		QCheckBox* no_interpCB;
		
		QPushButton* reset_lights;
		
		QSlider* zoom;
	
		QWidget* base;
};

class srot_widget : public QGroupBox {
	Q_OBJECT
	
	public:
		srot_widget(int strips, Orientation orientation, const QString& title, QWidget* parent = 0, const char * name = 0);
		
		void object_selected(struct md3_model_t* model);
		
	public slots:
		void scale_changed(int s_factor);
		void xrot_changed(int x_factor);
		void yrot_changed(int y_factor);
		void zrot_changed(int z_factor);
		void resets_clicked();
		
	private:
		struct md3_model_t* selected_model;	
	
		QGridLayout* move_grid;
		
		QLabel* scale_Label;
		QLabel* xrot_Label;
		QLabel* yrot_Label;
		QLabel* zrot_Label;
		
		QSlider* scale_S;
		QSlider* xrot_S;
		QSlider* yrot_S;
		QSlider* zrot_S;
		
		QPushButton* reset_sliders;
		
		QWidget* base;
	
};

class aa_widget : public QButtonGroup {
	Q_OBJECT
	
	public:
		aa_widget(int strips, Orientation orientation, const QString& title, QWidget* parent = 0, const char* name = 0);
	
	public slots:
		void aa_none_clicked();
		void aa_2x_clicked();
		void aa_4x_clicked();
		void aa_8x_clicked();
	
	private:
		QRadioButton* aa_none_rb;
		QRadioButton* aa_2x_rb;
		QRadioButton* aa_4x_rb;
		QRadioButton* aa_8x_rb;
};

class dof_widget : public QGroupBox {
	Q_OBJECT
	
	public:
		dof_widget(int strips, Orientation orientation, const QString& title, QWidget* parent = 0, const char * name = 0);
	
	public slots:
		void focus_changed(int factor);
		void dof_toggled();
	
	private:
		QWidget* base;	
		QGridLayout* grid;
	
		QLabel* focus_label;
		QSlider* focus_S;
	
		QCheckBox* dof_cb;
};

class gui_widget : public QFrame {
	Q_OBJECT

	friend class gl_widget;
	friend class model_widget;
	friend class opt_widget;
	
	public:
		gui_widget(int argc, char** argv);
		~gui_widget();
	
	private:
		QGridLayout* base_grid;
		gl_widget* gl;
		
		QVBoxLayout* side_layout;
		
		model_widget* model;
		model_widget* weapon;
		animate_widget* animate; 
		opt_widget* opt;
		srot_widget* srot;
		aa_widget* aa;
		dof_widget* dof;
	
		QLabel* credits;
		
		/* bottom widgets */
		QHBoxLayout* bottom_layout;
		QLabel* model_inf;	
		QLabel* cam_pos;
		QLabel* fps;
		
		QWidget* base;
	
};

#endif /* _GUI_H */
