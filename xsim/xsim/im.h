#ifndef _IM_H
#define _IM_H

#include <vector>
#include <X11/Xlib.h>
#include <IMdkit.h>

#include "conf.h"
#include "wstring.h"

using std::vector;

// key_type is the return value when processinput() is called
enum KEY_TYPE
{
	SWITCH_IM_KEY = 1,	// with the input, switch to next IM
	COMMIT_KEY,		// with the input, XIM will commit the string to client
	LISTFORWARD_KEY,	// with the input, move forward the displayed string
	LISTBACKWARD_KEY,	// with the input, move backward the displayed string
	FORWARD_KEY,		// with the input, nothing to do, and pass it to client
	IGNORE_KEY,		// with the input, nothing to do, and not pass to client
	NORMAL_KEY,		// with the input, inputing
	HIDE_KEY,		// with the input, input stoped
	STAT_KEY,		// with the input, input status changed
};

class TIMC;

class TIM
{
private:
	void * dlhandle;
protected:
	virtual TWstring * create(TWstring * dict_path, Obj_Config * conf) = 0;
public:
	virtual ~TIM(){}

	virtual const wchar_t * get_ident() = 0;
	virtual TWstring * get_desc() = 0;
	virtual TWstring * get_commit() = 0;

	virtual int processinput(TIMC * imc, XKeyEvent * e) = 0;
	
	virtual TIMC * create_imc() = 0;

	// if you need KeyRelease event, return 1
	virtual long needed_release_event() { return 0; }
	
	static TWstring * phrase_key(TWstring * str, XIMTriggerKey * keyret);
	static int iskey(XKeyEvent * e, const XIMTriggerKey * t, int keycnt = 1);

	static TIM * create(TWstring * plugins_path, TWstring * dict_path, Obj_Config * im_conf);
	static void destory(TIM * im);
};

#endif
