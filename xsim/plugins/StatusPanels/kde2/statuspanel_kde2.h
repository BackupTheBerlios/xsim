#ifndef _STATUSPANEL_KDE2_H
#define _STATUSPANEL_KDE2_H

#include "statuspanel.h"
#include "win.h"

#include <qpainter.h>
#include <qpointarray.h>
#include <ksystemtray.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qrect.h>
#include <qtextcodec.h>

class TIMC;

class TStatusPanel_KDE2:public TStatusPanel, public TWinObj
{
private:
	KSystemTray * tray;
	QPainter * painter;
	QBrush * brush;
	QPen * pen;
	QPixmap * pixmap;
	QFont * font;
	QTextCodec * codec;
	QColor * bg_color;
	QPointArray pointarray;
	QRect rect;

	int ft_ascent;
	int w,h;

protected:
	TWstring * create(Obj_Config * conf, Status_Pack * sp);
	void event_handler(XEvent & e);
public:
	TStatusPanel_KDE2();
	~TStatusPanel_KDE2();
	void update(Status_Pack * sp);
	Window id();
};

#endif
