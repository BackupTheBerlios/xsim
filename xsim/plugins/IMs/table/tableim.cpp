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

#include <X11/keysym.h>

#include "tableim.h"

extern "C" TIM * ClassCreator()
{
	return new TTableIM;
}

const XIMTriggerKey Switch_Enable_Key = {XK_Shift_L, 0, ControlMask | Mod1Mask};

/*
#ifdef BDB_VERSION4
int TTableIM::word_db_compare(DB * db, const DBT * dbt1, const DBT * dbt2)
#else
int TTableIM::word_db_compare(const DBT * dbt1, const DBT * dbt2)
#endif
{
	Word_Rec * d1 = (Word_Rec *)dbt1->data;
	Word_Rec * d2 = (Word_Rec *)dbt2->data;
	
	uint8_t ret = d1->sort_index - d2->sort_index;
	if (ret != 0)
		return ret;
		
	return wcscmp(d1->w, d2->w);
}
*/
int isnormal(KeySym k)
{
	if ((k >= 'a') && (k <= 'z'))
		return 1;

	if (k == XK_BackSpace)
		return 1;

	return 0;
}

TTableIM::TTableIM()
{
	word_db = NULL;
}

TWstring * TTableIM::create(TWstring * dict_path, Obj_Config * conf)
{
	char dummy[MAX_PATH_LEN + 1];
	TWstring path;
	TWstring val;
	int ret;
	TWstring * err;

	err = conf->get_val(L"WORDDB", val);
	if (err != NULL)
		return err;

	path.copy(dict_path->data());
	path.append('/');
	path.append(val);

	word_db = new Db(NULL, DB_CXX_NO_EXCEPTIONS);
	word_db->set_flags(DB_DUP);
//	word_db->set_flags(DB_DUP | DB_DUPSORT);
//	word_db->set_dup_compare(word_db_compare);
	if ((ret = word_db->open(path.tombs(dummy, MAX_PATH_LEN), NULL, DB_BTREE, 0, 0644)) != 0){
		err = new TWstring;
		DbException e("Open DB", dummy, ret);
		err->copy(e.what());
		delete word_db;
		word_db = NULL;
		return err;
		}

	if ((err = conf->get_val(L"IM_DESC", im_desc)) != NULL)
		return err;

	if ((err = conf->get_val(L"PREV_KEY", val)) != NULL)
		return err;
	if ((err = phrase_key(&val, &Listbackward_Key)) != NULL)
		return err;

	if ((err = conf->get_val(L"NEXT_KEY", val)) != NULL)
		return err;
	if ((err = phrase_key(&val, &Listforward_Key)) != NULL)
		return err;

	return NULL;
}

TTableIM::~TTableIM()
{
	if (word_db){
		word_db->close(0);
		delete word_db;
		}
}

int TTableIM::processnormal(TTableIMC * imc, XKeyEvent * e)
{
	if ((e->state & ShiftMask) == ShiftMask)	// we dont need shift masked key anymore
		return FORWARD_KEY;

	KeySym k;
	k = XLookupKeysym(e, 0);

	uint16_t i;

	if (k == XK_Return)				// if key is return , we commit the input to client
		if (imc->is_preedit()){
			output.copy(imc->get_input()->data());
			imc->clear();
			return COMMIT_KEY;
			}else
				return FORWARD_KEY;	// if no input, just pass return to client

	if (imc->is_preedit()){					// if has input now, try to commit
		if (k == XK_space)
			k = '1';
		if ((k >= '1') && (k <= '9')){
			i = k - '1';
			if (i >= imc->display_count())
				return IGNORE_KEY;

			output.copy(imc->display_str(i, NULL)->data());
			imc->clear();
			return COMMIT_KEY;
			}
		if (k == XK_Escape){
			imc->clear();
			return HIDE_KEY;
			}
		}

	if (!isnormal(k))				// if not a-z or backspace ,pass to client
		return FORWARD_KEY;

	// now do query
	TWstring key;
	key.copy(imc->get_input()->data());

        i = key.length();

	if (k == XK_BackSpace){
		if (!imc->is_preedit())
			return FORWARD_KEY;
		if (i == 1){				// building word
			imc->clear();
			return NORMAL_KEY;
			}
		key.erase(i - 1);
		}else{
			if (i == MAX_KEY_LEN)	// if exceed max input , ignore
				return IGNORE_KEY;
			key.append(k);
			}

	imc->set_input(&key);

	Dbc * cursor;
	word_db->cursor(0, &cursor, 0);
	// imc will build the list with cursor and key
	imc->set_cursor(NULL);
	imc->set_cursor(cursor);

	return NORMAL_KEY;
}

int TTableIM::processinput(TIMC * std_imc, XKeyEvent * e)
{
	TTableIMC * imc = (TTableIMC *) std_imc;
	static int do_switch_enable = 0;

	if (iskey(e, &Switch_Enable_Key)){
		if (e->type == KeyPress){
			do_switch_enable = 1;
			return FORWARD_KEY;
			}else if (do_switch_enable){
				do_switch_enable = 0;
				imc->switch_stat(IMC_STAT_DISABLE);
				return STAT_KEY;
				}
		}else
			do_switch_enable = 0;

	if (e->type == KeyRelease)
		return IGNORE_KEY;

	if (!imc->enabled())
		return FORWARD_KEY;

	if (imc->is_preedit()){
		if (iskey(e, &Listforward_Key))
			return LISTFORWARD_KEY;

		if (iskey(e, &Listbackward_Key))
			return LISTBACKWARD_KEY;
		}

	if (e->state & (ControlMask | Mod1Mask))	// the key with shift only is needed
		return FORWARD_KEY;

	return processnormal((TTableIMC *)imc, e);
}

TIMC * TTableIM::create_imc()
{
	TTableIMC * imc;
	
	imc = new TTableIMC;

	return imc;
}
