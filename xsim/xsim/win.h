// define a very simple X Window System OO interface
#ifndef _WIN_H
#define _WIN_H

#include <vector>

#include <X11/Xlib.h>

using std::vector;

class TWstring;

// inherits from this class will make u handle X Window System event automatic
class TWinObj
{
friend class TWinMan;
protected:
	virtual void event_handler(XEvent & e) = 0;
public:
	virtual Window id() = 0;
};

class TWinMan
{
private:
	static vector<TWinObj *> win_list;
	static Display * display;
	static int display_w, display_h;

public:
	TWinMan();
	~TWinMan();
	static Display * get_display() { return display; }
	static int get_display_w() { return display_w; }
	static int get_display_h() { return display_h; }

	static Window get_rootwin() { return RootWindow(display, DefaultScreen(display)); }

	static unsigned long getcolor(TWstring * colorname);
	static int win_alive(Window w);		//if win is alive

	static void register_win(TWinObj * win);
	static void unregister_win(TWinObj * win);
	static void event_loop();
};

class TGC
{
private:
	GC gc;
public:
	TGC(Window win, TWstring * fg_c, TWstring * bg_c);
	~TGC();

	GC id() { return gc; }
};

class TFontSet
{
private:
	XFontSet fontset;

	int font_width;
	int font_height;
	int font_ascent;
public:
	TFontSet();
	TWstring * create(TWstring * font_name);
	~TFontSet();

	int width() { return font_width; }
	int height() { return font_height; }
	int ascent() { return font_ascent; }

	XFontSet id() { return fontset; }
};

class TWin:TWinObj
{
private:
	Window win;
protected:
	virtual void event_handler(XEvent & e) {}

public:
	TWin();
	virtual ~TWin();

	virtual TWstring * create(int x, int y, int w, int h, TWstring * fg_color, TWstring * bg_color, TWin * parent = NULL);

	virtual void show();
	virtual void hide();

	int set_winattr(unsigned long attr_mask, XSetWindowAttributes * wa);
	int set_event_mask(long mask);

	Window id() { return win; }

	void move(int x, int y) { XMoveWindow(TWinMan::get_display(), win, x, y); }
	void resize(int w, int h) { XResizeWindow(TWinMan::get_display(), win, w, h); }
public:
	static void event_loop();
};

#endif
