#include <X11/keysym.h>

#include "simpleim.h"
#include "b2q.h"

extern "C" {
TIM * ClassCreator()
{
	return new TSimpleIM;
}
}

const XIMTriggerKey Switch_Enable_Key = {XK_Shift_L, 0, ControlMask | Mod1Mask};

#ifdef BDB_VERSION4
int TSimpleIM::char_db_compare(DB * db, const DBT * dbt1, const DBT * dbt2)
#else
int TSimpleIM::char_db_compare(const DBT * dbt1, const DBT * dbt2)
#endif
{
	Char_Rec *d1 = (Char_Rec *)dbt1->data;
	Char_Rec *d2 = (Char_Rec *)dbt2->data;

//cerr << "d1->ac = " << d1->access_count << " d2->ac = " << d2->access_count
//	<< " d1->c = " << d1->c << " d2->c = " << d2->c << endl;

	return wcsncmp(&d1->c, &d2->c, 1);
}

#ifdef BDB_VERSION4
int TSimpleIM::word_db_compare(DB * db, const DBT * dbt1, const DBT * dbt2)
#else
int TSimpleIM::word_db_compare(const DBT * dbt1, const DBT * dbt2)
#endif
{
	Word_Rec *d1 = (Word_Rec *)dbt1->data;
	Word_Rec *d2 = (Word_Rec *)dbt2->data;

        int len = (dbt1->size - d1->ac_size())/sizeof(wchar_t);

//cerr << "len1 = " << len << "len2 = " << len2 << " d1->ac = " << d1->access_count << " d2->ac = " << d2->access_count
//	<< " d1->w = " << d1->w[0] << " - " << d1->w[1] << " d2->w = " << d2->w[0] << " - " << d2->w[1] << endl;
	return wcsncmp(d1->w, d2->w, len);
}

/*
int TSimpleIM::db_key_compare(DB * db, const DBT * dbt1, const DBT * dbt2)
{
	char * s1 = (char *)dbt1->data;
	char * s2 = (char *)dbt2->data;
	return strcmp(s1, s2);
}
*/
enum Key_Type
{
	KT_NORMAL = 1,
	KT_SPLITOR,
	KT_EDIT,
	KT_NONEED,
};

int getkeytype(KeySym k)
{
	if ((k >= 'a') && (k <= 'z'))
		return KT_NORMAL;

	if (k == '\'')
		return KT_SPLITOR;

	if ((k == XK_BackSpace)
		|| (k == XK_Left)
		|| (k == XK_Right)
		|| (k == XK_Home)
		|| (k == XK_End)
		|| (k == XK_Delete))
		return KT_EDIT;

	return KT_NONEED;
}

TSimpleIM::TSimpleIM()
{
	char_db = NULL;
	word_db = NULL;
	phraser = NULL;
}

TWstring * TSimpleIM::create(TWstring * dict_path, Obj_Config * conf)
{
	char dummy[MAX_PATH_LEN + 1];
	TWstring path;
	TWstring val;
	int ret;
	TWstring * err;

	path.copy(dict_path->data());
	conf->get_val(L"CHARDB", val);
	path.append('/');
	path.append(val);

	char_db = new Db(NULL, DB_CXX_NO_EXCEPTIONS);
	char_db->set_flags(DB_DUP | DB_DUPSORT);
	char_db->set_dup_compare(char_db_compare);
	if ((ret = char_db->open(path.tombs(dummy, MAX_PATH_LEN), NULL, DB_BTREE, 0, 0644)) != 0){
		err = new TWstring;
		DbException e("Open DB", dummy, ret);
		err->copy(e.what());
		delete char_db;
		char_db=NULL;
		return err;
		}
	char_db->cursor(0, &chardb_cursor, 0);

	chardb_key.set_ulen(sizeof(ckey));
	chardb_key.set_flags(DB_DBT_USERMEM);
	chardb_key.set_data(&ckey);
	chardb_data.set_ulen(sizeof(Char_Rec));
	chardb_data.set_flags(DB_DBT_USERMEM);
	chardb_data.set_data(&crec);

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
		word_db=NULL;
		return err;
		}
	word_db->cursor(0, &worddb_cursor, 0);

	worddb_key.set_ulen(sizeof(Word_Key));
	worddb_key.set_flags(DB_DBT_USERMEM);
	worddb_key.set_data(&wkey);
	worddb_data.set_ulen(sizeof(Word_Rec));
	worddb_data.set_flags(DB_DBT_USERMEM);
	worddb_data.set_data(&wrec);

	int care_h, care_ng;

	if ((err = conf->get_val(L"IM_DESC", im_desc)) != NULL)
		return err;

	if ((err = conf->get_val(L"CARE_H", val)) != NULL)
		return err;
	if (val.casecompare(L"Yes") == 0)
		care_h = 1;
			else
				care_h = 0;

	if ((err = conf->get_val(L"CARE_NG", val)) != NULL)
		return err;
	if (val.casecompare(L"Yes") == 0)
		care_ng = 1;
			else
				care_ng = 0;

	phraser = new TPYPhraser(care_h, care_ng);

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

TSimpleIM::~TSimpleIM()
{
	if (char_db){
		char_db->close(0);
		delete char_db;
		}

	if (word_db){
		word_db->close(0);
		delete word_db;
		}

	if (phraser)
		delete phraser;
}

void TSimpleIM::update_rec(List_Item * item)
{
	switch(item->type){
		case LI_CHAR:
			if (item->cr.access_count == 0)
				return;

			ckey.copy(&item->ck);
			crec.copy(&item->cr);

			chardb_key.set_size(ckey.size());

			if (chardb_cursor->get(&chardb_key, &chardb_data, DB_GET_BOTH) == DB_NOTFOUND){
				assert (0);
				}

			crec.access_count--;
			if (chardb_cursor->put(&chardb_key, &chardb_data, DB_CURRENT)){
				assert(0);
				}

			char_db->sync(0);
			break;

		case LI_WORD:
			if (item->wr.access_count == 0)
				return;

			wkey.copy(&item->wk);
			worddb_key.set_size(wkey.size());
			wrec.copy(&item->wr);
			worddb_data.set_size(wrec.size());

			// maybe some user will build a word that exist in db
			if (worddb_cursor->get(&worddb_key, &worddb_data, DB_GET_BOTH) == 0){
				wrec.access_count--;
				if (worddb_cursor->put(&worddb_key, &worddb_data, DB_CURRENT)){
					assert(0);
					}
				}else{
					wrec.access_count = MAX_ACCESS_COUNT - 1;
					worddb_data.set_size(wrec.size());
					if (worddb_cursor->put(&worddb_key, &worddb_data, DB_KEYFIRST)){
						assert(0);
						}
					}

			word_db->sync(0);
			break;
		}
}

void TSimpleIM::delete_word(List_Item * item)
{
	assert (item->type == LI_WORD);

	wkey.copy(&item->wk);
	worddb_key.set_size(wkey.size());
	wrec.copy(&item->wr);
	worddb_data.set_size(wrec.size());

	if (worddb_cursor->get(&worddb_key, &worddb_data, DB_GET_BOTH) == 0)
		worddb_cursor->del(0);

	word_db->sync(0);
}

int TSimpleIM::processedit(TSimpleIMC * imc, TWstring & py, int & caret, KeySym k)
{
	wchar_t dc;
	switch (k){
		case XK_BackSpace:
			if (caret == 0)
				return IGNORE_KEY;
			dc = py[--caret];
			py.erase(caret, 1);
			if (caret == 0)                            // if backspace clear the input, hide im
				if (imc->preword_len()){
					caret = imc->preword_input_len();
					imc->set_input(&py);
					imc->pop_preword();
					py.copy(imc->get_input()->data());
					}else if (py.empty()){
						imc->clear();
						return HIDE_KEY;
						}
			if (dc == L' '){
				imc->set_caret_pos(caret + imc->preword_len());
				imc->set_input(&py);
				return NORMAL_KEY;
				}
			break;
		case XK_Left:
			if (caret == 0)
				return IGNORE_KEY;
			caret--;
			if (caret > 0){
				imc->set_caret_pos(caret + imc->preword_len());
				return NORMAL_KEY;
				}

			caret = imc->preword_input_len();
			imc->pop_preword();
			py.copy(imc->get_input()->data());
			imc->set_caret_pos(caret + imc->preword_len());
			break;
			
		case XK_Right:
			if ((size_t)caret == imc->get_input()->length())
				return IGNORE_KEY;
			imc->set_caret_pos(caret + imc->preword_len() + 1);
			return NORMAL_KEY;

		case XK_Home:
			imc->pop_preword();
			py.copy(imc->get_input()->data());
			imc->set_caret_pos(0);
			caret = 0;
			break;
		case XK_End:
			imc->set_caret_pos(py.length() + imc->preword_len());
			return NORMAL_KEY;
		case XK_Delete:
			if ((size_t)caret == py.length())
				return IGNORE_KEY;
			dc = py[caret];
			py.erase(caret, 1);
			if (py.empty()){
					imc->clear();
					return HIDE_KEY;
					}
			if (dc == L' '){
				imc->set_input(&py);
				return NORMAL_KEY;
				}
			break;
		}
	return 0;
}

int TSimpleIM::processnormal(TSimpleIMC * imc, XKeyEvent * e)
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
			output.erase();
			for (i = 0; i < imc->display_input()->length(); i++)
				if ((*imc->display_input())[i] != ' ')
					output.append((*imc->display_input())[i]);
			imc->clear();
			return COMMIT_KEY;
			}else
				return FORWARD_KEY;	// if no input, just pass return to client

	TWstring py;
        int caret;
	py.copy(imc->get_input()->data());

	if (imc->is_preedit()){					// if has input now, try to commit
		if (k == XK_space)
			k = '1';
		if ((k >= '1') && (k <= '9')){
			i = k - '1';
			if (i >= imc->display_count())
				return IGNORE_KEY;
			imc->add_preword(imc->display_list_item(i));
			if (!imc->word_complete()){
				py.copy(imc->get_input()->data());
				caret = py.length();
				goto BUILD_WORD_LIST;
				}
			List_Item * li;
			li = imc->get_finalword();
			if (li->type == LI_CHAR)
				output.copy(li->cr.c);
				else
					output.copy(li->wr.w);
			update_rec(li);
			imc->clear();
			return COMMIT_KEY;
			}
		if (k == XK_Escape){
			imc->clear();
			return HIDE_KEY;
			}
		}

        i = py.length();
	caret = imc->caret_pos() - imc->preword_len();
	switch (getkeytype(k)){
		case KT_NORMAL:
		case KT_SPLITOR:
			if (i == MAX_PY_LEN)	// if exceed max input , ignore
				return IGNORE_KEY;

			py.insert(k, caret);
			caret++;
			break;
		case KT_NONEED:
			// if not a-z or '\'' or any of edit key ,pass to client
			return FORWARD_KEY;
		case KT_EDIT:
			if (!imc->is_preedit())
				return FORWARD_KEY;
			// now we process pinyin edit
			int ret = processedit(imc, py, caret, k);
			if (ret)
				return ret;
		}

BUILD_WORD_LIST:
	int j;
	for (j = caret, i = 0; j > 0; j--, i++)
		if (py[i] == L' ')
			caret --;
			
	for (j = py.length() - 1; j >= 0; j--)
		if (py[j] == L' ')
			py.erase(j, 1);

	if ((j = py.find(L"''")) != -1){
		if (k == L'\'')
			return IGNORE_KEY;
		py.erase(j, 1);
		}
	if (py[0] == L'\'')
		py.erase(0, 1);
	if (py.length() == 0){
		imc->clear();
		return HIDE_KEY;
		}

	int ret = phraser->build_key(py);	// py will be splited
	if (phraser->char_count() + imc->preword_len() > MAX_WORD_LEN)
		return IGNORE_KEY;

	for (i = 0, j = 0; j < caret; i++)
		if (py[i] != L' ')
			j ++;
	imc->set_caret_pos(i + imc->preword_len());
	if (!ret){
		imc->clearlist(IMC_CLEARALL);
		imc->set_input(&py);
		return NORMAL_KEY;
		}

	imc->set_input(&py);

	build_list(imc, phraser);

	return NORMAL_KEY;
}

void TSimpleIM::build_list(TSimpleIMC * imc, TPYPhraser * phraser)
{

	// ************************* build char list
	char charpybuf[MAX_CHAR_PY_LEN + 1];
	strcpy(charpybuf, phraser->get_key()->py);

	size_t first_py_len = strlen(charpybuf);
	size_t i = phraser->char_count();

	TWstring fullpy;

	fullpy.copy(phraser->list_item(0)->data());
	imc->set_char_left(i);
	if (imc->need_rebuild_charlist(fullpy)){
		imc->clearlist(IMC_CLEARALL);
		imc->set_first_char_py(fullpy);

		strcpy(ckey.py, charpybuf);
		chardb_key.set_size(ckey.size());

		if (chardb_cursor->get(&chardb_key, &chardb_data, DB_SET_RANGE) == 0)
			for (;;) {
				if (strncmp(ckey.py, charpybuf, first_py_len) != 0)
					break;
				fullpy.copy(ckey.py);
				if (phraser->charfit(fullpy))
					imc->add_char(&ckey, &crec);
				if (chardb_cursor->get(&chardb_key, &chardb_data, DB_NEXT))
					break;
				}
		}else
			imc->clearlist(IMC_CLEARWORD);

	if (i == 1)
		return;
	// ************************* build word list
	for(;i > 1; i--){
		wkey.copy(phraser->get_key());
		wkey.len = i;
		worddb_key.set_size(wkey.size());
		memset(&wrec, 0, sizeof(wrec));
		if (worddb_cursor->get(&worddb_key, &worddb_data, DB_SET_RANGE) == 0)
			for (;;) {
				if ((wkey.len != i) || (strncmp(wkey.py, charpybuf, first_py_len) != 0))
					break;
				fullpy.copy(wkey.py);
				if (phraser->wordfit(fullpy, i))
					imc->add_word(&wkey, &wrec);

				memset(&wrec, 0, sizeof(wrec));
				if (worddb_cursor->get(&worddb_key, &worddb_data, DB_NEXT))
					break;
				}
		}
}

int TSimpleIM::processinput(TIMC * std_imc, XKeyEvent * e)
{
	static int do_switch_enable = 0;
	TSimpleIMC * imc = (TSimpleIMC *)std_imc;

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
			if (li->type == LI_WORD){
				delete_word(li);
				imc->clear();
				return HIDE_KEY;
				}
			return IGNORE_KEY;
			}
		}


	if (e->state & (ControlMask | Mod1Mask))	// the key with shift only is needed
		return FORWARD_KEY;

	if (imc->is_mb_input())
		if (convertB2Q(e, cb2q))
			return COMMIT_KEY;
			else
				return FORWARD_KEY;

	return processnormal(imc, e);;
}

int TSimpleIM::switchBQmode(TSimpleIMC * imc)
{
	imc->clear();

	imc->switch_stat(IMC_STAT_MB_INPUT);
	return 1;
}


int TSimpleIM::convertB2Q(XKeyEvent * e, const b2q_tab * b2q)
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

TIMC * TSimpleIM::create_imc()
{
	TSimpleIMC * imc;
	
	imc = new TSimpleIMC;

	return imc;
}
