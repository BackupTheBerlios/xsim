#ifndef _PANEL_H
#define _PANEL_H

#include "conf.h"

enum DISPLAY_CALC_MODE
{
	CALC_INIT = 1,
	CALC_FORWARD,
	CALC_BACKWARD,
};

class TIC;
class TWstring;

class TPanel
{
private:
	void * dlhandle;
protected:
	virtual TWstring * create(Obj_Config * conf) = 0;
public:
	virtual ~TPanel() {}

	// imc display member would be modified by call set_display
	virtual int display_count_calc(int mode, TIC * ic)= 0;
	virtual void update(TIC * ic) = 0;

	static TPanel * create(TWstring * plugins_path, Obj_Config * conf);
	static void destory(TPanel * panel);
	
};

#endif
