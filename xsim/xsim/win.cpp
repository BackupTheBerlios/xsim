#include <assert.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "win.h"
#include "wstring.h"

// TFontSet mothed start >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TFontSet::TFontSet()
{
	fontset = NULL;
}

TWstring * TFontSet::create(TWstring * fontname)
{
	assert(fontname);

	int charset_count=0;
	char **charset_list, *def_string;
	XFontStruct **font_structs;
	char dummy[256];

        font_width  = 0;
        font_height = 0;
        font_ascent = 0;

	Display * display = TWinMan::get_display();

    	fontset = XCreateFontSet(display, fontname->tombs(dummy, 255), &charset_list, &charset_count, &def_string);

    	if (!fontset || charset_count){
		TWstring * err_msg = new TWstring;
		err_msg->copy(L"failed create font :");
		err_msg->append(fontname->data());
		err_msg->append(L'\n');
		for (int i = 0; i<charset_count; i++){
			err_msg->append(L"invalid font ");
			err_msg->append(charset_list[i]);
			err_msg->append(L'\n');
			}
		if (charset_list)
			XFreeStringList(charset_list);
		if (fontset){
			XFreeFontSet(display, fontset);
			fontset	= NULL;
			}
		return err_msg;
		}

	charset_count = XFontsOfFontSet(fontset, &font_structs, &charset_list);

	int charwidth;
	for (int i = 0; i < charset_count; i++){
		charwidth = font_structs[i]->max_bounds.width / 2;
		if (charwidth > font_width)
	    		font_width = charwidth;
		charwidth = font_structs[i]->ascent + font_structs[i]->descent;
		if (charwidth > font_height){
	    		font_height = charwidth;
	    		font_ascent = font_structs[i]->ascent;
			}
		}
	return NULL;
}

TFontSet::~TFontSet()
{
	if (fontset)
		XFreeFontSet(TWinMan::get_display(), fontset);
}
// TFontSet method end <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// TGC method start >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

TGC::TGC(Window win, TWstring * fg_c, TWstring * bg_c)
{
	Display * display = TWinMan::get_display();

	unsigned long fg_color, bg_color;
	
	fg_color = TWinMan::getcolor(fg_c);
	bg_color = TWinMan::getcolor(bg_c);

	gc = XCreateGC(display, win, 0, NULL);

	XSetForeground(display, gc, fg_color);
	XSetBackground(display, gc, bg_color);
}

TGC::~TGC()
{
	XFreeGC(TWinMan::get_display(), gc);
}
// TGC method end <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// TWinMan method start >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
vector<TWinObj *> TWinMan::win_list;
Display * TWinMan::display = NULL;
int TWinMan::display_w = 1024;
int TWinMan::display_h = 768;

TWinMan::TWinMan()
{
	//should not create more then one window manager
	assert(display == NULL);

	if (!(display = XOpenDisplay(NULL))){
		TWstring err_msg;
		err_msg.copy(L"Can not open display");
		throw err_msg;
		}

	display_w = DisplayWidth(display, DefaultScreen(display));
	display_h = DisplayHeight(display, DefaultScreen(display));
}

TWinMan::~TWinMan()
{
	for (size_t i = 0; i < win_list.size(); i++)
		delete win_list[i];
	XCloseDisplay(display);
}

int window_error_handler(Display * display, XErrorEvent * e)
{
	return 0;
}

int TWinMan::win_alive(Window w)
{
	int (*old_handler)(Display *, XErrorEvent *);

	old_handler = XSetErrorHandler(window_error_handler);
	XWindowAttributes wa;
	Status ret = XGetWindowAttributes(TWinMan::get_display(), w, &wa);
	XSetErrorHandler(old_handler);
	return ret;
}

unsigned long TWinMan::getcolor(TWstring * colorname)
{
	Colormap colormap;
	XColor c;
	char dummy[256];

	colormap = DefaultColormap(TWinMan::get_display(), DefaultScreen(display));
	XParseColor(TWinMan::get_display(), colormap, colorname->tombs(dummy, 255), &c);
	XAllocColor(TWinMan::get_display(), colormap, &c);

	return c.pixel;
}

void TWinMan::register_win(TWinObj * win)
{
	win_list.push_back(win);
}

void TWinMan::unregister_win(TWinObj * win)
{
	for (size_t i = 0; i < win_list.size(); i++)
		if (win_list[i] == win){
			win_list.erase((win_list.begin() + i));
			return;
			}
//	assert(0);
}

void TWinMan::event_loop()
{
	XEvent e;

	for(;;){
	        XNextEvent(get_display(), &e);
		if (XFilterEvent(&e, None) == True)
			continue;

		for (size_t i = 0; i < win_list.size(); i++)
			if (win_list[i]->id() == e.xany.window)
				win_list[i]->event_handler(e);
		}
}

// TWinMan method end <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// TWin define ****************************************************************
TWin::TWin()
{
	win = 0;
}

TWstring * TWin::create(int x, int y, int w, int h,TWstring * fg_color, TWstring * bg_color, TWin * parent)
{
	Window root_win;

	Display * display = TWinMan::get_display();		//maybe display not connected

	if (parent)
		root_win = parent->id();
		else
			root_win = RootWindow(display, DefaultScreen(display));

	win = XCreateSimpleWindow(display, root_win,
		x, y, w, h, 1,
		TWinMan::getcolor(fg_color), TWinMan::getcolor(bg_color));

	if (!win){
		TWstring * err_msg = new TWstring;
		err_msg->copy(L"Can not create window");
		return err_msg;
		}
	TWinMan::register_win(this);
	
	return NULL;
}

TWin::~TWin()
{
	if (win){
		XDestroyWindow(TWinMan::get_display(), win);
		TWinMan::unregister_win(this);
		}
}

int TWin::set_winattr(unsigned long attr_mask, XSetWindowAttributes * wa)
{
	return XChangeWindowAttributes(TWinMan::get_display(), win, attr_mask, wa);
}

int TWin::set_event_mask(long mask)
{
	return XSelectInput(TWinMan::get_display(), win, mask);
}

void TWin::show()
{
	XRaiseWindow(TWinMan::get_display(), win);
	XMapWindow(TWinMan::get_display(), win);
	XFlush(TWinMan::get_display());
}

void TWin::hide()
{
	XUnmapWindow(TWinMan::get_display(), win);
}
