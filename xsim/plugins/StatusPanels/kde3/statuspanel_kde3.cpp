#include <assert.h>

#include <kapplication.h>

#include "statuspanel_kde3.h"
#include "conf.h"
#include "imc.h"
#include "win.h"

KApplication * app = NULL;

extern "C" {
TStatusPanel_KDE3 * ClassCreator()
{
	char faketitle[]="xsim";
	char * fakeargv[1];
	fakeargv[0] = faketitle;
	int fakecnt = 1;

	app = new KApplication(TWinMan::get_display(),
			fakecnt, fakeargv,"xsim", false, false);

	return new TStatusPanel_KDE3;
}
}

TWstring * TStatusPanel_KDE3::create(Obj_Config * conf, Status_Pack * sp)
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

	// now we start draw Big X and init some data
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

TStatusPanel_KDE3::~TStatusPanel_KDE3()
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
	tray->hide();
	//if (tray)
	//	delete tray;
}

void TStatusPanel_KDE3::update(Status_Pack * sp)
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

Window TStatusPanel_KDE3::id()
{
	return tray->winId();
}

void TStatusPanel_KDE3::event_handler(XEvent & e)
{
	switch (e.type){
		case Expose:
			app->x11ProcessEvent(&e);
			break;

		}
}
