#include <assert.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "statuspanel_std.h"
#include "conf.h"
#include "win.h"
#include "imc.h"
#include "ic.h"
#include "im.h"

const wchar_t CONFIG_FILE_NAME[] = L"statuspanel_std.config";

extern "C" {
TStatusPanel_STD * ClassCreator()
{
	return new TStatusPanel_STD;
}
}

TStatusPanel_STD::TStatusPanel_STD()
{
	gc = stat_gc = NULL;
	fontset = NULL;
}

TWstring * TStatusPanel_STD::create(Obj_Config * conf, Status_Pack * sp)
{
	vector<TWstring> name;
	vector<TWstring> value;
	int w, h;

	TWstring val1, val2;
	TWstring * err;

	if ((err = conf->get_val(L"FONT_NAME", val1)) != NULL)
		return err;
	fontset = new TFontSet;
	if ((err = fontset->create(&val1)) != NULL)
		return err;

	if ((err = conf->get_val(L"FG_COLOR", val1)) != NULL)
		return err;
	if ((err = conf->get_val(L"BG_COLOR", val2)) != NULL)
		return err;
		
	TWin::create(0, 0, 1, 1, &val1, &val2, NULL);

	gc = new TGC(id(), &val1, &val2);

	if ((err = conf->get_val(L"STATUS_COLOR", val1)) != NULL)
		return err;
	stat_gc = new TGC(id(), &val1, &val1);

	w = fontset->width() * 2 + 4;
	h = fontset->height() +  4;
	if (TConf::load_config(CONFIG_FILE_NAME, name, value)){
		win_x = value[0].toint();
		win_y = value[1].toint();
		}else{
			win_x = TWinMan::get_display_w() - 3 * w - 1;
			win_y = TWinMan::get_display_h() - h - 1;
			}

	XSetWindowAttributes wa;
	wa.override_redirect = True;
	set_winattr(CWOverrideRedirect, &wa);

	set_event_mask(ExposureMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|VisibilityChangeMask);

	im_desc.copy(sp->im_desc.data());
	resize(w, h);
	move(win_x, win_y);
	show();

	return NULL;
}

TStatusPanel_STD::~TStatusPanel_STD()
{
	vector<TWstring> name;
	vector<TWstring> value;
	TWstring n, v;

	n.copy(L"stat_panel_x");
	v.fromint(win_x);
	name.push_back(n);
	value.push_back(v);

	n.copy(L"stat_panel_y");
	v.fromint(win_y);
	name.push_back(n);
	value.push_back(v);

	TConf::save_config(CONFIG_FILE_NAME, name, value);
	
	if (gc)
		delete gc;
	if (stat_gc)
		delete stat_gc;
	if (fontset)
		delete fontset;
}

void TStatusPanel_STD::update(Status_Pack * sp)
{
	TIMC * imc = sp->imc;

	XClearWindow(TWinMan::get_display(), id());

	uint16_t x = 2;
	uint16_t y = fontset->ascent() + 2;

	if (imc == NULL){
		im_desc.copy(sp->im_desc[0]);
#ifdef MDK_PATCH
		char dummy[10];
		im_desc.tombs(dummy, 4);
		XmbDrawString(TWinMan::get_display(), id(), fontset->id(), gc->id(), x, y, dummy, strlen(dummy));
#else
		XwcDrawString(TWinMan::get_display(), id(), fontset->id(), gc->id(), x, y, &im_desc[0], 1);
#endif
		return;
		}

	if (!imc->enabled()){
#ifdef MDK_PATCH
		XmbDrawImageString(TWinMan::get_display(), id(), fontset->id(), gc->id(), x, y, "En", 2);
#else
		XwcDrawImageString(TWinMan::get_display(), id(), fontset->id(), gc->id(), x, y, L"En", 2);
#endif
		return;
		}

	if (imc->is_mb_input()){
		//XFillArc(display, stat_win, stat_gc, space_width, space_heigth, font_width * 2, font_height, 0, (360 * 64));
		XFillRectangle(TWinMan::get_display(), id(), stat_gc->id(), 2, 2, fontset->width() * 2 + 1, fontset->height() + 1);
		}else{
			XPoint p[3];
			p[0].x = 2;
			p[0].y = 2;
			p[1].x = fontset->width() * 2 + 3;
			p[1].y = 2;
			p[2].x = 2;
			p[2].y = fontset->height() + 3;
			//XFillArc(display, stat_win, stat_gc, space_width, space_heigth, font_width * 2, font_height, (45 * 64), (225 * 64));
			XFillPolygon(TWinMan::get_display(), id(), stat_gc->id(), p, 3, Convex,CoordModeOrigin);
			}

	im_desc.copy(sp->im_desc[0]);
#ifdef MDK_PATCH
	char dummy[10];
	im_desc.tombs(dummy, 4);
	XmbDrawString(TWinMan::get_display(), id(), fontset->id(), gc->id(), x, y, dummy, strlen(dummy));
#else
	XwcDrawString(TWinMan::get_display(), id(), fontset->id(), gc->id(), x, y, &im_desc[0], 1);
#endif
}

void TStatusPanel_STD::event_handler(XEvent & e)
{
	switch (e.type){
		case Expose:{
			TIMC * imc;
			TIC * ic = TIC::get_focus_ic();
			if (ic && ic->on() && ic->has_imc())
				imc = ic->get_imc();
				else
					imc = NULL;

			Status_Pack sp;
			sp.im_desc.copy(im_desc);
			sp.imc = imc;
			update(&sp);
			break;
			}
		case ButtonPress:
			if (e.xbutton.button != Button1)
				break;
			move_status_panel(e);
			break;
		case VisibilityNotify:{
/*
			int timeout, interval;
			int prefer_blanking;
			int allow_exposures;

                        XGetScreenSaver(TWinMan::get_display(),
				&timeout, &interval, &prefer_blanking, &allow_exposures);

//			if (timeout)
*/
			XRaiseWindow(TWinMan::get_display(), id());
			break;
			}
		}
}

void TStatusPanel_STD::move_status_panel(XEvent & e)
{
	XEvent me;		//mouse event
	Time pt = CurrentTime;	//prev time

	for (;;){
		XMaskEvent (TWinMan::get_display(), PointerMotionMask | ButtonReleaseMask, &me);

		if (me.xany.type == ButtonRelease)
			break;

		if (me.xany.type == MotionNotify){
                        if (me.xmotion.time - pt < 50)
                                continue;
			win_x = me.xmotion.x_root - e.xbutton.x;
			win_y = me.xmotion.y_root - e.xbutton.y;
			XMoveWindow (TWinMan::get_display(), id(), win_x, win_y);

			pt = me.xmotion.time;
			}
                }
}
