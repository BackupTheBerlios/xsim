#ifndef _STATUSPANEL_H
#define _STATUSPANEL_H

#include "conf.h"

class TIMC;
class TWstring;

class TIMC;

typedef struct Status_Pack_tag
{
	TWstring im_ident;
	TWstring im_desc;
	TIMC * imc;
}Status_Pack;

class TStatusPanel
{
private:
	void * dlhandle;
protected:
	virtual TWstring * create(Obj_Config * conf, Status_Pack * sp) = 0;
public:
	virtual ~TStatusPanel(){};

	virtual void update(Status_Pack * sp) = 0;

	static TStatusPanel * create(TWstring * plugins_path, Obj_Config * conf, Status_Pack * status_pack);
	static void destory(TStatusPanel * spanel);
};

#endif
