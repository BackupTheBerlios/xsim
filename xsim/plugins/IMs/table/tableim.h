#ifndef _TABLEIM_H
#define _TABLEIM_H

#include <vector>

#include <X11/Xlib.h>

#include "IMdkit/IMdkit.h"
#include "IMdkit/Xi18n.h"

#include "im.h"
#include "conf.h"

#include "tableimc.h"

const wchar_t TABLE_IM_IDENT[] = L"table";

class TTableIMC;
class TTableIM:public TIM
{
private:
	XIMTriggerKey Listforward_Key;
	XIMTriggerKey Listbackward_Key;

private:
	TWstring im_desc;
	TWstring output;

	Db * word_db;

private:
	int processnormal(TTableIMC * imc, XKeyEvent * e);
/*
#ifdef BDB_VERSION4
	static int word_db_compare(DB * db, const DBT * dbt1, const DBT * dbt2);
#else
	static int word_db_compare(const DBT * dbt1, const DBT * dbt2);
#endif
*/
protected:
	TWstring * create(TWstring * dict_path, Obj_Config * conf);
public:
	TTableIM();
	~TTableIM();

	TIMC * create_imc();;

	const wchar_t * get_ident() { return TABLE_IM_IDENT; }
	TWstring * get_desc() { return &im_desc; }
	TWstring * get_commit() { return & output; }

	int processinput(TIMC * std_imc, XKeyEvent * e);
	long needed_release_event() { return 1; }
};

#endif
