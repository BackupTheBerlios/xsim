#ifndef _SIMPLEIM_H
#define _SIMPLEIM_H

#include <vector>

#include <X11/Xlib.h>
#include <db_cxx.h>

#include "IMdkit/IMdkit.h"
#include "IMdkit/Xi18n.h"

#include "im.h"
#include "conf.h"

#include "simpleimc.h"
#include "pyphraser.h"
#include "b2q.h"

const wchar_t SIMPLE_IM_IDENT[] = L"jianpin";

class TSimpleIMC;
class TSimpleIM:public TIM
{
private:
	XIMTriggerKey Switch_BQ_Key;
	XIMTriggerKey Listforward_Key;
	XIMTriggerKey Listbackward_Key;
	XIMTriggerKey Word_Delete_Keys[9];

private:
	TWstring im_desc;

	Db * char_db;
	Dbc * chardb_cursor;

	Db * word_db;
	Dbc * worddb_cursor;

	Dbt chardb_key, chardb_data;
	Dbt worddb_key, worddb_data;

	Char_Key ckey;
	Char_Rec crec;
	Word_Key wkey;
	Word_Rec wrec;


	TWstring output;

	TPYPhraser * phraser;
	
private:
	int processnormal(TSimpleIMC * imc, XKeyEvent * e);
	int processedit(TSimpleIMC * imc, TWstring & py, int & caret, KeySym k);
	int convertB2Q(XKeyEvent * e, const b2q_tab * b2q);	// Ban to Quan :P
	int switchBQmode(TSimpleIMC * imc);

#ifdef BDB_VERSION4
	static int char_db_compare(DB * db, const DBT * dbt1, const DBT * dbt2);
	static int word_db_compare(DB * db, const DBT * dbt1, const DBT * dbt2);
#else
	static int char_db_compare(const DBT * dbt1, const DBT * dbt2);
	static int word_db_compare(const DBT * dbt1, const DBT * dbt2);
#endif
	void build_list(TSimpleIMC * imc, TPYPhraser * phraser);
	void update_rec(List_Item * item);
	void delete_word(List_Item * item);
protected:
	TWstring * create(TWstring * dict_path, Obj_Config * conf);
public:
	TSimpleIM();
	~TSimpleIM();

	TIMC * create_imc();;

	const wchar_t * get_ident() { return SIMPLE_IM_IDENT;};
	TWstring * get_desc() { return &im_desc; }
	TWstring * get_commit() { return & output; }

	int processinput(TIMC * std_imc, XKeyEvent * e);
	long needed_release_event() { return 1; }

};

#endif
