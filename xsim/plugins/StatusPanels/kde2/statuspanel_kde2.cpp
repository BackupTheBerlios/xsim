/*
	Copyright (C) 2002 chukuang

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
//kde2.x.x has a old name kapp.h, do include it to make the g++ happy
#include <kapp.h>
#include "conf.h"
#include "imc.h"
#include "win.h"

#include "statuspanel_kde2.h"

KApplication * app = NULL;

extern "C" TStatusPanel_KDE2 * ClassCreator()
{
	char faketitle[]="xsim";
	char * fakeargv;
	fakeargv = faketitle;
	int fakecnt = 1;

	app = new KApplication(TWinMan::get_display(), fakecnt, &fakeargv, "xsim");

	return new TStatusPanel_KDE2;
}

TWstring * TStatusPanel_KDE2::create(Obj_Config * conf, Status_Pack * sp)
{
	TWstring * err;
	TWstring val;
	char dummy[256];

	tray = new KSystemTray;
	pixmap = new QPixmap(10, 10);	// painter need a no null pixmap
	painter = new QPainter;
	brush = new QBrush;
	font = new QFont;
	pen = new QPen;

	app->setMainWidget(tray);	// realy need?
	// set codec to current locale
	codec = QTextCodec::codecForLocale();

	// set status color
	if ((err = conf->get_val(L"STATUS_COLOR", val)) != NULL)
		return err;

	QColor c(val.tombs(dummy, 255));
	brush->setStyle(Qt::SolidPattern);
	brush->setColor(c);

	// set foreground color
	if ((err = conf->get_val(L"FG_COLOR", val)) != NULL)
		return err;

	c.setNamedColor(val.tombs(dummy, 255));
	pen->setColor(c);

	// set background color
	if ((err = conf->get_val(L"BG_COLOR", val)) != NULL)
		return err;

	bg_color = new QColor(val.tombs(dummy, 255));
	// load font ++++++++++++++++++++++++++++++++++
	if ((err = conf->get_val(L"FONT_NAME", val)) != NULL)
		return err;

	font->setRawName(val.tombs(dummy, 255));
	font->setPointSize(14);
	// --------------------------------------------
	TWstring s;
	s.copy(sp->im_desc[0]);
	s.tombs(dummy, 10);

	QString qstr;
	QString tmp;
	qstr = codec->toUnicode(dummy);

	painter->begin(pixmap);
	painter->setFont(*font);

	QFontMetrics fm = painter->fontMetrics();

	w = fm.width(qstr) + 4;
	h = fm.height() + 4;
	ft_ascent = fm.ascent();
	painter->end();

	pointarray.putPoints(0, 3, 2, 2, w - 2, 2, 2, h - 2);
	// need end to resize pixmap
	pixmap->resize(w, h);

	painter->begin(pixmap);

	painter->setFont(*font);
	painter->setPen(*pen);
	painter->setBackgroundColor(*bg_color);

	pixmap->fill(*bg_color);			// clear pixmap
	painter->drawText(2, ft_ascent + 2, qstr);

	painter->end();

	tray->setPixmap(*pixmap);
	TWinMan::register_win(this);
	tray->show();
	return NULL;
}

TStatusPanel_KDE2::TStatusPanel_KDE2()
{
	bg_color = NULL;
	pen = NULL;
	brush = NULL;
	pixmap = NULL;
	font = NULL;
	painter = NULL;
	tray = NULL;
}

TStatusPanel_KDE2::~TStatusPanel_KDE2()
{
	TWinMan::unregister_win(this);
	
	if (bg_color)
		delete bg_color;
	if (pen)
		delete pen;
	if (brush)
		delete brush;
	if (pixmap)
		delete pixmap;
        if (font)
		delete font;
	if (painter)
		delete painter;
	if (tray){
		tray->hide();
		//delete tray;
		}
	if (app){
		app->exit();
		//delete app;
		}

}

void TStatusPanel_KDE2::update(Status_Pack * sp)
{
	TIMC * imc = sp->imc;
	// clear pixmap content
	pixmap->fill(*bg_color);

	QString qstr;
	if (imc == NULL){
		char dummy[10];
                TWstring s;
		s.copy(sp->im_desc[0]);
		s.tombs(dummy, 10);
		qstr = codec->toUnicode(dummy);

		painter->begin(pixmap);
		painter->setFont(*font);
		painter->setPen(*pen);
		painter->setBackgroundColor(*bg_color);
		painter->drawText(2, ft_ascent + 2, qstr);
		painter->end();

		tray->setPixmap(*pixmap);
		tray->repaint();
		return;
		}

	if (!imc->enabled()){
		qstr = "En";

		painter->begin(pixmap);
		painter->setFont(*font);
		painter->setPen(*pen);
		painter->setBackgroundColor(*bg_color);
		painter->drawText(2, ft_ascent + 2, qstr);
		painter->end();

		tray->setPixmap(*pixmap);

		tray->repaint();
		return;
		}

	painter->begin(pixmap);
	painter->setFont(*font);
	painter->setBackgroundColor(*bg_color);
	painter->setPen(Qt::NoPen);
	painter->setBrush(*brush);

	if (imc->is_mb_input()){
		painter->fillRect(2,  2, w - 4, w - 4, *brush);
		}else{
			painter->drawPolygon(pointarray);
			}

	painter->setPen(*pen);

	char dummy[20];
        TWstring s;
	s.copy(sp->im_desc[0]);
	s.tombs(dummy, 19);

	qstr = codec->toUnicode(dummy);

	painter->drawText(2, ft_ascent + 2, qstr);
	painter->end();
	tray->setPixmap(*pixmap);

	tray->repaint();
}

Window TStatusPanel_KDE2::id()
{
	return tray->winId();
}

void TStatusPanel_KDE2::event_handler(XEvent & e)
{
	switch (e.type){
		case Expose:
			app->x11ProcessEvent(&e);
			break;

		}
}
