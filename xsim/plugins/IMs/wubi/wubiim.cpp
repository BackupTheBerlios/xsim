#include <X11/keysym.h>

#include "wubiim.h"
#include "b2q.h"

extern "C" {
TIM * ClassCreator()
{
	return new TWubiIM;
}
}

const XIMTriggerKey Switch_Enable_Key = {XK_Shift_L, 0, ControlMask | Mod1Mask};

#ifdef BDB_VERSION4
int TWubiIM::word_db_compare(DB * db, const DBT * dbt1, const DBT * dbt2)
#else
int TWubiIM::word_db_compare(const DBT * dbt1, const DBT * dbt2)
#endif
{
	Word_Rec *d1 = (Word_Rec *)dbt1->data;
	Word_Rec *d2 = (Word_Rec *)dbt2->data;

	return wcscmp(d1->w, d2->w);
}

int isnormal(KeySym k)
{
	if ((k >= 'a') && (k <= 'z'))
		return 1;

	if (k == XK_BackSpace)
		return 1;

	return 0;
}

TWubiIM::TWubiIM()
{
	word_db = NULL;
}

TWstring * TWubiIM::create(TWstring * dict_path, Obj_Config * conf)
{
	char dummy[MAX_PATH_LEN + 1];
	TWstring path;
	TWstring val;
	int ret;
	TWstring * err;

	path.copy(dict_path->data());
	conf->get_val(L"WORDDB", val);
	path.append('/');
	path.append(val);

	word_db = new Db(NULL, DB_CXX_NO_EXCEPTIONS);
	word_db->set_flags(DB_DUP | DB_DUPSORT);
	word_db->set_dup_compare(word_db_compare);
	if ((ret = word_db->open(path.tombs(dummy, MAX_PATH_LEN), NULL, DB_BTREE, 0, 0644)) != 0){
		err = new TWstring;
		DbException e("Open DB", dummy, ret);
		err->copy(e.what());
		delete word_db;
		word_db = NULL;
		return err;
		}

	word_db->cursor(0, &worddb_cursor, 0);

	worddb_key.set_ulen(sizeof(Word_Key));
	worddb_key.set_flags(DB_DBT_USERMEM);
	worddb_key.set_data(&wkey);
	worddb_data.set_ulen(sizeof(Word_Rec));
	worddb_data.set_flags(DB_DBT_USERMEM);
	worddb_data.set_data(&wrec);

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

	if ((err = conf->get_val(L"B2Q_KEY", val)) != NULL)
		return err;
	if ((err = phrase_key(&val, &Switch_BQ_Key)) != NULL)
		return err;
	
	if ((err = conf->get_val(L"NEW_WORD_KEY", val)) != NULL)
		return err;
	if ((err = phrase_key(&val, &New_Word_Key)) != NULL)
		return err;

	if ((err = conf->get_val(L"WORDDELETE_KEY", val, L"ctrl N")) != NULL)
		return err;
	if ((err = phrase_key(&val, Word_Delete_Keys)) != NULL)
		return err;
	Word_Delete_Keys[0].keysym = XK_1;
	for (int i = 1; i < 9; i++)
		memcpy(&Word_Delete_Keys[i], &Word_Delete_Keys[0], sizeof(XIMTriggerKey));
	Word_Delete_Keys[1].keysym = XK_2;
	Word_Delete_Keys[2].keysym = XK_3;
	Word_Delete_Keys[3].keysym = XK_4;
	Word_Delete_Keys[4].keysym = XK_5;
	Word_Delete_Keys[5].keysym = XK_6;
	Word_Delete_Keys[6].keysym = XK_7;
	Word_Delete_Keys[7].keysym = XK_8;
	Word_Delete_Keys[8].keysym = XK_9;

	return NULL;
}

TWubiIM::~TWubiIM()
{
	if (word_db){
		word_db->close(0);
		delete word_db;
		}
}

void TWubiIM::update_rec(List_Item * li)
{
	if (!li)
		return;
		
	wkey.copy(&li->wk);
	worddb_key.set_size(wkey.size());
	wrec.copy(&li->wr);
	worddb_data.set_size(wrec.size());

	// maybe some user will build a word that exist in db
	if (worddb_cursor->get(&worddb_key, &worddb_data, DB_GET_BOTH) == 0)
		return;

	worddb_data.set_size(wrec.size());
	if (worddb_cursor->put(&worddb_key, &worddb_data, DB_KEYFIRST))
		assert(0);

	word_db->sync(0);
}

void TWubiIM::delete_word(List_Item * item)
{
	wkey.copy(&item->wk);
	worddb_key.set_size(wkey.size());
	wrec.copy(&item->wr);
	worddb_data.set_size(wrec.size());

	if (worddb_cursor->get(&worddb_key, &worddb_data, DB_GET_BOTH) == 0)
		worddb_cursor->del(0);
	word_db->sync(0);
}

void TWubiIM::build_list(TWubiIMC * imc, TWstring * key)
{
	char keybuf[MAX_KEY_LEN + 1];
	size_t i;
	key->tombs(keybuf, MAX_KEY_LEN);
	for (i = 0; i < key->length(); i++)
		if (key->data()[i] != 'z')
			break;
	// all 'z' is no meaning, do nothing
	if (i == strlen(keybuf)){
		imc->set_cursor(NULL);
		return;
		}

	Dbc * cursor;
	word_db->cursor(0, &cursor, 0);

	imc->set_cursor(cursor);
}

int TWubiIM::processnormal(TWubiIMC * imc, XKeyEvent * e)
{
	if (!imc->is_preedit())
		if (convertB2Q(e, sb2q))		// if no input before, try to convert to Qian jiao
			return COMMIT_KEY;

	if ((e->state & ShiftMask) == ShiftMask)	// we dont need shift masked key anymore
		return FORWARD_KEY;

	KeySym k;
	k = XLookupKeysym(e, 0);

	uint16_t i;

	if (k == XK_Return)				// if key is return , we commit the input to client
		if (imc->is_preedit()){
			if (imc->is_new_word()){
				Dbc * cursor;
				word_db->cursor(0, &cursor, 0);
				update_rec(imc->get_finalitem(cursor));
				cursor->close();
				output.copy(imc->get_finalword()->data());
				}else
					output.copy(imc->get_input()->data());
			imc->clear();
			return COMMIT_KEY;
			}else
				return FORWARD_KEY;	// if no input, just pass return to client

	// now do with wubi
	TWstring key;
	key.copy(imc->get_input()->data());

	if (imc->is_preedit()){					// if has input now, try to commit
		if (k == XK_space)
			k = '1';
		if ((k >= '1') && (k <= '9')){
			i = k - '1';
			if (i >= imc->display_count())
				return IGNORE_KEY;

			List_Item * li;
			li = imc->display_list_item(i);
			if (imc->is_new_word()){
				imc->add_preword(li);
				imc->clear_list();
				return NORMAL_KEY;
				}
			output.copy(li->wr.w);
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

        i = key.length();

	if (k == XK_BackSpace){
		if (!imc->is_preedit())
			return FORWARD_KEY;
		if (i == 0){				// building word
			if (imc->del_last_preword())
				return NORMAL_KEY;
				else{
					imc->clear();
					return HIDE_KEY;
					}
			}
		key.erase(i - 1);
		if (!imc->is_preedit()){
			imc->clear();
			return HIDE_KEY;
			}
		if (i == 1){				// building word
			imc->clear_list();
			return NORMAL_KEY;
			}
		}else{
			if (i == MAX_KEY_LEN)	// if exceed max input , ignore
				return IGNORE_KEY;
			key.append(k);
			}

	imc->set_input(&key);
	build_list(imc, &key);

	return NORMAL_KEY;
}

int TWubiIM::processinput(TIMC * std_imc, XKeyEvent * e)
{
	TWubiIMC * imc = (TWubiIMC *) std_imc;
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

	if (iskey(e, &Switch_BQ_Key)){
		switchBQmode(imc);
		return STAT_KEY;
		}

	if (imc->is_preedit()){
		if (iskey(e, &Listforward_Key))
			return LISTFORWARD_KEY;

		if (iskey(e, &Listbackward_Key))
			return LISTBACKWARD_KEY;
		int i;
		if ((i = iskey(e, Word_Delete_Keys, 9)) > 0){
			if (i > imc->display_count())
				return IGNORE_KEY;
			List_Item * li = imc->display_list_item(i-1);
			//is it a word?
			if (li->wr.w[1] != 0){
				delete_word(li);
				imc->set_update_stamp(time(NULL));
				imc->clear();
				return HIDE_KEY;
				}
			return IGNORE_KEY;
			}
		}

	if (iskey(e, &New_Word_Key)){
		imc->start_new_word();
		return NORMAL_KEY;
		}
	
	if (e->state & (ControlMask | Mod1Mask))	// the key with shift only is needed
		return FORWARD_KEY;

	if (imc->is_mb_input())
		if (convertB2Q(e, cb2q))
			return COMMIT_KEY;
			else
				return FORWARD_KEY;

	return processnormal((TWubiIMC *)imc, e);
}

int TWubiIM::switchBQmode(TWubiIMC * imc)
{
	imc->clear();

	imc->switch_stat(IMC_STAT_MB_INPUT);
	return 1;
}


int TWubiIM::convertB2Q(XKeyEvent * e, const b2q_tab * b2q)
{
	KeySym k;
	static wchar_t B2Q_charmode1 = char_pair1_1;		// char \'
	static wchar_t B2Q_charmode2 = char_pair2_1;		// char \"

	k = XLookupKeysym(e, 0);

	for (int i = 0; b2q[i].e != 0; i++)
		if (b2q[i].e == (char) k){
			if (e->state & (ShiftMask | LockMask)){
				output.copy(b2q[i].u);
				}else{
					output.copy(b2q[i].l);
					}
			switch (output[0]){
				case char_pair1_1:			//switch char pair ¡®  ¡¯
					output.copy(B2Q_charmode1);
					B2Q_charmode1 ^= (char_pair1_1 ^ char_pair1_2);
					break;
				case char_pair2_1:			//switch char pair ¡°  ¡±
					output.copy(B2Q_charmode2);
					B2Q_charmode2 ^= (char_pair2_1 ^ char_pair2_2);
					break;
				}
			return 1;
			}
	return 0;
}

TIMC * TWubiIM::create_imc()
{
	TWubiIMC * imc;
	
	imc = new TWubiIMC;

	return imc;
}
