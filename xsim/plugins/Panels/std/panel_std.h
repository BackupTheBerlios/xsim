#ifndef _PANEL_STD_H
#define _PANEL_STD_H

#include "panel.h"
#include "win.h"
#include "conf.h"

class TIC;

class TPanel_STD:public TPanel,public TWin
{
private:
	TFontSet * fontset;
	TGC * gc, *suffix_gc, *caret_gc;

	int space_width;
	int space_height;
	int max_display_width;

	int win_w, win_h;
	int win_x, win_y;
private:
	void set_win_pos(TIC * ic);
	void repaint(TIC * ic);

	void paint_win(TIC * ic);

	void event_handler(XEvent & e);

	TWstring * create(Obj_Config * conf);
public:
	TPanel_STD();
	~TPanel_STD();

	// imc display member would be modified by call set_display
	int display_count_calc(int mode, TIC * ic);

	void update(TIC * ic);
};

#endif
