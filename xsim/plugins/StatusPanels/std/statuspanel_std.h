#ifndef _STATUSPANEL_STD_H
#define _STATUSPANEL_STD_H

#include <X11/Xlib.h>

#include "win.h"
#include "statuspanel.h"

class TIC;

class TStatusPanel_STD:public TStatusPanel, public TWin
{
private:
	TGC * gc, * stat_gc;
	TFontSet * fontset;

	int win_x, win_y;
	TWstring im_desc;
private:
	void event_handler(XEvent & e);

	void move_status_panel(XEvent & e);
protected:
	TWstring * create(Obj_Config * conf, Status_Pack * sp);
public:
	~TStatusPanel_STD();
	TStatusPanel_STD();
	void update(Status_Pack * sp);
};

#endif
