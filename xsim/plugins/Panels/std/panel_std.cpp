#include <assert.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "win.h"
#include "panel_std.h"
#include "ic.h"
#include "im.h"
#include "imc.h"

const uint8_t MAX_DISPLAY_WORD_CNT = 9;	// coz' no "10" key,and i dont like use "0" :P

extern "C" {
TPanel_STD * ClassCreator()
{
	return new TPanel_STD;
}
}

TPanel_STD::TPanel_STD()
{
	suffix_gc = NULL;
	gc = NULL;
	caret_gc = NULL;
	fontset = NULL;
}

TWstring * TPanel_STD::create(Obj_Config * conf)
{
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

	if ((err = conf->get_val(L"SUFFIX_COLOR", val1)) != NULL)
		return err;

	suffix_gc = new TGC(id(), &val1, &val2);

	if ((err = conf->get_val(L"CARET_COLOR", val1, L"blue")) != NULL)
		return err;

	caret_gc = new TGC(id(), &val1, &val2);

	space_width = fontset->width();
	space_height = fontset->width() / 2;
	max_display_width = fontset->width() * MAX_DISPLAY_WORD_CNT * (2 + 1 + 1) + space_width * 2;
	win_h = fontset->height() * 2 + space_height * 4;
	win_w = 100;

	XSetWindowAttributes wa;
	wa.override_redirect = True;
	set_winattr(CWOverrideRedirect, &wa);

	set_event_mask(ExposureMask);
	return NULL;
}

TPanel_STD::~TPanel_STD()
{
	if (fontset)
		delete fontset;
	if (gc)
		delete gc;
	if (suffix_gc)
		delete suffix_gc;
	if (caret_gc)
		delete caret_gc;
}

int TPanel_STD::display_count_calc(int mode, TIC * ic)
{
	int i;
        TIMC * imc = ic->get_imc();
	uint16_t index = imc->display_index();
	uint16_t count = imc->display_count();

	TWstring list_string;
	TWstring list_suffix;
	XRectangle rect;
	switch (mode){
		case CALC_INIT:
			int max_w;

			index = 0;

			list_string.copy(L' ');
			list_string.append(ic->get_im()->get_desc()->data());
			list_string.append(L"  ");
			list_string.append(imc->display_input()->data());
			list_string.append(L' ');

#ifdef MDK_PATCH
			{
			char dummy[256];
			list_string.tombs(dummy, 255);
			XmbTextExtents(fontset->id(),
				dummy,
				strlen(dummy),
				NULL, &rect);
			}
#else
			XwcTextExtents(fontset->id(),
				list_string.data(),
				list_string.length(),
				NULL, &rect);
#endif
			max_w = (max_display_width < (rect.width + space_width)) ?rect.width + space_width :max_display_width;

			list_string.erase();

			for (i = 0; i < (int)(imc->list_count()); i++){
				list_string.append('X');
				list_string.append(imc->list_str(i, &list_suffix)->data());
				list_string.append(list_suffix);
				list_string.append(' ');
#ifdef MDK_PATCH
				{
				char dummy[256];
				list_string.tombs(dummy, 255);
				XmbTextExtents(fontset->id(),
					dummy,
					strlen(dummy),
					NULL, &rect);
				}
#else
				XwcTextExtents(fontset->id(),
					list_string.data(),
					list_string.length(),
					NULL, &rect);
#endif
				if (rect.width > max_w)
					break;
				win_w = rect.width + space_width;
				}
			if (win_w < max_w)
				win_w = max_w;
			imc->set_display_size(win_w);
			count = i - index;
			if (count > MAX_DISPLAY_WORD_CNT)
				count = MAX_DISPLAY_WORD_CNT;
			break;

		case CALC_FORWARD:
			if ((index + count) >= imc->list_count())
				return 0;
			
			index += count;
		
			for (i = index; i < (int)(imc->list_count()); i++){
				list_string.append('X');
				list_string.append(imc->list_str(i, &list_suffix)->data());
				list_string.append(list_suffix);
				list_string.append(' ');
#ifdef MDK_PATCH
				{
				char dummy[256];
				list_string.tombs(dummy, 255);
				XmbTextExtents(fontset->id(),
					dummy,
					strlen(dummy),
					NULL, &rect);
				}
#else
				XwcTextExtents(fontset->id(),
					list_string.data(),
					list_string.length(),
					NULL, &rect);
#endif
				if (rect.width > win_w)
					break;
				}
			count = i - index;
			if (count > MAX_DISPLAY_WORD_CNT)
				count = MAX_DISPLAY_WORD_CNT;

			break;

		case CALC_BACKWARD:
			if (index == 0)
				return 0;

			for (i = index - 1; i >= 0; i--){
				list_string.append('X');
				list_string.append(imc->list_str(i, &list_suffix)->data());
				list_string.append(list_suffix);
				list_string.append(' ');
#ifdef MDK_PATCH
				{
				char dummy[256];
				list_string.tombs(dummy, 255);
				XmbTextExtents(fontset->id(),
					dummy,
					strlen(dummy),
					NULL, &rect);
				}
#else
				XwcTextExtents(fontset->id(),
					list_string.data(),
					list_string.length(),
					NULL, &rect);
#endif
				if (rect.width > win_w)
					break;
				}
			count = index - (i + 1);
			if (count > MAX_DISPLAY_WORD_CNT)
				count = MAX_DISPLAY_WORD_CNT;
			index -= count;
			break;
		}

	imc->setdisplay(index, count);
	return 1;
}

void TPanel_STD::set_win_pos(TIC * ic)
{
	Window cw, child;
	IC * ic_ = ic->get_IC();

	switch (ic_->input_style & (XIMPreeditCallbacks | XIMPreeditPosition | XIMPreeditNothing)){
		case XIMPreeditNothing:
			win_x = (TWinMan::get_display_w() - win_w) / 2;
			win_y = TWinMan::get_display_h();
			break;

		case XIMPreeditPosition:
			cw = (ic_->focus_win) ?ic_->focus_win :ic_->client_win;
			XTranslateCoordinates(TWinMan::get_display(), cw, TWinMan::get_rootwin(),
				ic_->pre_attr.spot_location.x,
				ic_->pre_attr.spot_location.y + space_height,
				&win_x, &win_y, &child);
			break;
/*
		case XIMPreeditCallbacks:
			cw = (ic_->focus_win) ?ic_->focus_win :ic_->client_win;

			XTranslateCoordinates(display, cw, root_win,
				ic_->pre_attr.spot_location.x,
				ic_->pre_attr.spot_location.y + space_heigth,
				&win_x, &win_y, &child);

			break;
*/
		default:
			win_x = 0;
			win_y = 0;
			break;
		}

	if ((win_x + win_w) > TWinMan::get_display_w() - 2)
		win_x = TWinMan::get_display_w() - win_w - 2;
	if ((win_y + win_h) > TWinMan::get_display_h() - 2)
		win_y = TWinMan::get_display_h() - win_h - 2;

	move(win_x, win_y);
	resize(win_w, win_h);
}
/*

styles

mozila:	514	XIMPreeditCallbacks | XIMStatusCallbacks
gtk:	1028	XIMPreeditPosition  | XIMStatusNothing
kde:	1032	XIMPreeditNothing   | XIMStatusNothing
	XWindowAttributes wa;


	XGetWindowAttributes
*/

void TPanel_STD::repaint(TIC * ic)
{
	TIMC * imc;

	imc = ic->get_imc();

	wchar_t index;
	TWstring s, suffix;
	uint16_t x, x_save, x_save2 = 0, i;
	XRectangle rect;
	uint16_t y = fontset->ascent() + space_height;

	s.copy(' ');
	s.append(ic->get_im()->get_desc()->data());
	s.append(' ');
#ifdef MDK_PATCH
	char dummy[256];
	s.tombs(dummy, 255);
	XmbTextExtents(fontset->id(),
		dummy,
		strlen(dummy),
		NULL, &rect);
#else
	XwcTextExtents(fontset->id(),
		s.data(),
		s.length(),
		NULL, &rect);
#endif
	x_save = rect.width;

	if (imc->caret_pos() != -1){
		i = s.length();
		s.append(' ');
		s.append(imc->display_input()->data());
		s.erase(i + imc->caret_pos() + 1 /* space */);
#ifdef MDK_PATCH
		s.tombs(dummy, 255);
		XmbTextExtents(fontset->id(),
			dummy,
			strlen(dummy),
			NULL, &rect);
#else
		XwcTextExtents(fontset->id(),
			s.data(),
			s.length(),
			NULL, &rect);
#endif
		x_save2 = rect.width;
		s.erase(i);
		}

	s.append(' ');
	s.append(imc->display_input()->data());

#ifdef MDK_PATCH
	s.tombs(dummy, 255);
	XmbDrawImageString(TWinMan::get_display(), id(), fontset->id(), gc->id(), 0, y, dummy, strlen(dummy));
#else
	XwcDrawImageString(TWinMan::get_display(), id(), fontset->id(), gc->id(), 0, y, s.data(), s.length());
#endif

	y = fontset->height() + space_height * 3 + fontset->ascent();
        x = 0;
        
	for (i = 0; i < imc->display_count(); i++){
		s.copy(' ');
		index = i + 1 + '0';
		s.append(index);
		s.append(imc->display_str(i, &suffix)->data());
#ifdef MDK_PATCH
		s.tombs(dummy, 255);
		XmbDrawImageString(TWinMan::get_display(), id(), fontset->id(), gc->id(), x, y, dummy, strlen(dummy));
		XmbTextExtents(fontset->id(),
			dummy,
			strlen(dummy),
			NULL, &rect);
#else
		XwcDrawImageString(TWinMan::get_display(), id(), fontset->id(), gc->id(), x, y, s.data(), s.length());
		XwcTextExtents(fontset->id(),
			s.data(),
			s.length(),
			NULL, &rect);
#endif
		x += rect.width;

		if (suffix.empty())
			continue;
#ifdef MDK_PATCH
		suffix.tombs(dummy, 255);
		XmbDrawImageString(TWinMan::get_display(), id(), fontset->id(), suffix_gc->id(), x, y, dummy, strlen(dummy));
		XmbTextExtents(fontset->id(),
			dummy,
			strlen(dummy),
			NULL, &rect);
#else
		XwcDrawImageString(TWinMan::get_display(), id(), fontset->id(), suffix_gc->id(), x, y, suffix.data(), suffix.length());
		XwcTextExtents(fontset->id(),
			suffix.data(),
			suffix.length(),
			NULL, &rect);
#endif
		x += rect.width;
		}

	y = fontset->height() + space_height * 2;

	XDrawLine(TWinMan::get_display(), id(), gc->id(), x_save, 0, x_save, y);
	XDrawLine(TWinMan::get_display(), id(), gc->id(), 0, y, win_w, y);
	if (imc->caret_pos() != -1)
		XDrawLine(TWinMan::get_display(), id(), caret_gc->id(), x_save2, 4, x_save2, y - 4);


//	XFlush(display);
}

void TPanel_STD::paint_win(TIC * ic)
{
	win_w = ic->get_imc()->get_display_size();
	set_win_pos(ic);

	show();
	XClearWindow(TWinMan::get_display(), id());
	repaint(ic);
}

void TPanel_STD::update(TIC * ic)
{
	TIMC * imc = ic->get_imc();

	if (imc && ic->has_focus() && imc->is_preedit() && imc->enabled())
		paint_win(ic);
		else
			hide();
}

void TPanel_STD::event_handler(XEvent & e)
{
	switch (e.type){
		case Expose:
			repaint(TIC::get_focus_ic());
			break;
		}
}
