#ifndef _IC_H
#define _IC_H

#include <X11/Xlib.h>
#include <X11/Xmd.h>

#include "wstring.h"

typedef struct {
	XRectangle	area;		/* area */
	XRectangle	area_needed;	/* area needed */
	XPoint		spot_location;	/* spot location */
	Colormap	cmap;		/* colormap */
	CARD32		foreground;	/* foreground */
	CARD32		background;	/* background */
	Pixmap		bg_pixmap;	/* background pixmap */
	//char		*base_font;	/* base font of fontset */
	TWstring	base_font;	/* base font of fontset */
	CARD32		line_space;	/* line spacing */
	Cursor		cursor;		/* cursor */
} PreeditAttributes;

typedef struct {
	XRectangle	area;		/* area */
	XRectangle	area_needed;	/* area needed */
	Colormap	cmap;		/* colormap */
	CARD32		foreground;	/* foreground */
	CARD32		background;	/* background */
	Pixmap		bg_pixmap;	/* background pixmap */
	//char		*base_font;	/* base font of fontset */
	TWstring	base_font;	/* base font of fontset */
	CARD32		line_space;	/* line spacing */
	Cursor		cursor;		/* cursor */
} StatusAttributes;

typedef struct {
	CARD16			id;		/* ic id */
	CARD16			connect_id;	/* connect id */
	INT32			input_style;	/* input style */
	Window			client_win;	/* client window */
	Window			focus_win;	/* focus window */
	//char			*resource_name;	/* resource name */
	//char			*resource_class;/* resource class */
	PreeditAttributes 	pre_attr; 	/* preedit attributes */
	StatusAttributes 	sts_attr; 	/* status attributes */
} IC;

enum IC_INIT_MODE
{
	IC_CLEAR = 1,
	IC_RESET,
	IC_ERASE,
};

class TIM;
class TIMC;

class TIC
{
friend class TICManager;
private:
	IC	ic_;
	TIC	* next;
private:
	TIM * im;
	TIMC * imc;
	static TIC * focused_ic;
	uint8_t on_stat;

private:
	TIC(TIM * val);
	~TIC();

public:
	TIM * get_im(){ return im; }
	void set_im(TIM * val);

	TIMC * get_imc();
	int has_imc() { return (int)imc; }

	void init(int init_mode, TIM * val);

	static TIC * get_focus_ic() { return focused_ic; }
	IC * get_IC() { return &ic_; };
	
	int alive();
	Window get_client_win() { return ic_.client_win; }

	int has_focus() { return (focused_ic == this); }
	void set_focus() { focused_ic = this; }
	void unset_focus() { if (has_focus()) focused_ic = NULL; }

	int on() { return on_stat; }
	void turn_on();
	void turn_off();
};


#endif
