#ifndef _ICMAN_H
#define _ICMAN_H

#include <X11/Xlib.h>

#include "IMdkit/IMdkit.h"
#include "IMdkit/Xi18n.h"

#include "ic.h"

class TIM;

class TICManager
{
private:
	TIC * ics;
	TIC * freeics;

	CARD16 icid_;
private:
	int saveic(TIC * ic, IMChangeICStruct *call_data);
	int loadic(TIC * ic, IMChangeICStruct *call_data);
	
public:
	TICManager();
	~TICManager();

	TIC * newic(IMChangeICStruct *call_data, TIM * val);
	TIC * deleteic(IMDestroyICStruct *call_data);
	int seticvalue(IMChangeICStruct *call_data);//return 1 if input panel need repaint
	int geticvalue(IMChangeICStruct *call_data);

	TIC * getic(CARD16 icid);
	void destory_all();
};

#endif
