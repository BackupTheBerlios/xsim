#ifndef _WUBIIM_H
#define _WUBIIM_H

#include <vector>

#include <X11/Xlib.h>
#include <db_cxx.h>

#include "IMdkit/IMdkit.h"
#include "IMdkit/Xi18n.h"

#include "im.h"
#include "conf.h"

#include "wubiimc.h"
#include "b2q.h"

const wchar_t WUBI_IM_IDENT[] = L"wubi";

class TWubiIMC;
class TWubiIM:public TIM
{
private:
	XIMTriggerKey Switch_BQ_Key;
	XIMTriggerKey Listforward_Key;
	XIMTriggerKey Listbackward_Key;
	XIMTriggerKey New_Word_Key;
	XIMTriggerKey Word_Delete_Keys[9];

private:
	TWstring im_desc;

	Db * word_db;
	Dbc * worddb_cursor;

	Dbt worddb_key, worddb_data;

	Word_Key wkey;
	Word_Rec wrec;

	TWstring output;
private:
	int processnormal(TWubiIMC * imc, XKeyEvent * e);
	int convertB2Q(XKeyEvent * e, const b2q_tab * b2q);	// Ban to Quan :P
	int switchBQmode(TWubiIMC * imc);

#ifdef BDB_VERSION4
	static int word_db_compare(DB * db, const DBT * dbt1, const DBT * dbt2);
#else
	static int word_db_compare(const DBT * dbt1, const DBT * dbt2);
#endif
	void update_rec(List_Item * li);
	void build_list(TWubiIMC * imc, TWstring * key);
	void delete_word(List_Item * item);
protected:
	TWstring * create(TWstring * dict_path, Obj_Config * conf);
public:
	TWubiIM();
	~TWubiIM();

	TIMC * create_imc();;

	const wchar_t * get_ident() { return WUBI_IM_IDENT; }
	TWstring * get_desc() { return &im_desc; }
	TWstring * get_commit() { return & output; }

	int processinput(TIMC * std_imc, XKeyEvent * e);
	long needed_release_event() { return 1; }

};

#endif
