#include <assert.h>
#include <string.h>

#include "wubiimc.h"
#include "wubiim.h"

time_t TWubiIMC::last_update_stamp = 0;

TWubiIMC::TWubiIMC()
{
	displayindex = displaycount = 0;
	imc_stat = 0;
	preword_cnt = 0;

	cursor = NULL;
	list_cnt = 0;
	worddb_key.set_ulen(sizeof(Word_Key));
	worddb_key.set_flags(DB_DBT_USERMEM);
	worddb_key.set_data(&item.wk);
	worddb_data.set_ulen(sizeof(Word_Rec));
	worddb_data.set_flags(DB_DBT_USERMEM);
	worddb_data.set_data(&item.wr);
	update_stamp = 0;
}

TWubiIMC::~TWubiIMC()
{
	if (cursor)
		cursor->close();
}

int TWubiIMC::is_preedit()
{
	if (update_stamp < last_update_stamp){
		clear();
		return 0;
		}
	return (input.empty() && !is_new_word()) ?0 :1;
}
// -1 : stop; 0: not fit, but continue; 1: ok
inline int key_fit(register char * key, register char * dbkey)
{
	int ret = -1;
	for(; (*key != 0); key++, dbkey++)
		if(*key != *dbkey){
			if (*key != 'z')
				return ret;
			if (*dbkey == 0)
				return 0;
			ret = 0;
			}
	if (*dbkey == 0)
		return 1;
		else
			return 0;
}

void TWubiIMC::set_cursor(Dbc * db_cursor)
{
	if(cursor)
		cursor->close();

        cursor = db_cursor;
	cursor_index = 0;
	list_cnt = 0;
	
	if (!cursor)
		return;
	
	char keybuf[MAX_KEY_LEN + 1];
	uint16_t key_len, subkey_len;
	
	input.tombs(keybuf, MAX_KEY_LEN);
	strcpy(item.wk.key, keybuf);

	for (size_t i = 0; (size_t) i < strlen(item.wk.key); i++)
		if (item.wk.key[i] == 'z'){
			item.wk.key[i] = 0;
			break;
			}

	key_len = strlen(keybuf);
	subkey_len = strlen(item.wk.key);
	
	worddb_key.set_size(item.wk.size());
/*
	if (subkey_len == key_len){
		if (cursor->get(&worddb_key, &worddb_data, DB_SET) != 0){
			cursor->close();
			cursor = NULL;
			return;
			}
		
		cursor->count(&list_cnt, 0);
		return;
		}
*/			
	if (subkey_len != 0){
		if (cursor->get(&worddb_key, &worddb_data, DB_SET_RANGE) != 0){
			cursor->close();
			cursor = NULL;
			return;
			}
	        }else if (cursor->get(&worddb_key, &worddb_data, DB_FIRST) != 0)
				assert(0);		// u gave me an empty DB, so stop

	do{
		switch (key_fit(keybuf, item.wk.key)){
			case -1:
				goto LIST_SCAN_OVER;
			case 1:
				list_cnt++;
			case 0:
				break;
			}
		}while (cursor->get(&worddb_key, &worddb_data, DB_NEXT) == 0);

LIST_SCAN_OVER:
	if (list_cnt == 0){
		cursor->close();
		cursor = NULL;
		return;
		}

	//reset cursor to start of list
	strcpy(item.wk.key, keybuf);
	item.wk.key[subkey_len] = 0;
	worddb_key.set_size(item.wk.size());

	if (subkey_len != 0){
		if (cursor->get(&worddb_key, &worddb_data, DB_SET_RANGE) != 0)
			assert(0);			// DB changed???
	        }else if (cursor->get(&worddb_key, &worddb_data, DB_FIRST) != 0)
				assert(0);		// u gave me an empty DB, so stop
				
	// find the first correct record
	do{
		if (key_fit(keybuf, item.wk.key) == 1)
			break;
		}while (cursor->get(&worddb_key, &worddb_data, DB_NEXT) == 0);
}

List_Item * TWubiIMC::list_item(uint16_t index)
{
	assert (index < list_count());

	uint32_t flag;
	int step;
        if (index > cursor_index){
		flag = DB_NEXT;
		step = 1;
		}else{
			flag = DB_PREV;
			step = -1;
			}

	char keybuf[MAX_KEY_LEN + 1];
	
	input.tombs(keybuf, MAX_KEY_LEN);

	while (cursor_index != index){
		if (cursor->get(&worddb_key, &worddb_data, flag) != 0)
			assert(0);			// tell me why
		if (key_fit(keybuf, item.wk.key) == 1)
			cursor_index += step;
		}

	cursor->get(&worddb_key, &worddb_data, DB_CURRENT);

	return &item;
}

TWstring * TWubiIMC::list_str(uint16_t index, TWstring * suffix)
{
	List_Item * li;
	li = list_item(index);
	
	ret_str.copy(li->wr.w);

	TWstring fullkey;
	fullkey.copy(li->wk.key);

	if (!suffix)
		return &ret_str;
	
	TWstring t;

	suffix->erase();

	size_t i = 0;
	if (input.find('z') >= 0){
		for(; i < input.length(); i++)
			if (input[i] == 'z')
				suffix->append(fullkey[i] & ~0x20);
				else
					suffix->append(fullkey[i]);
		}

	return &ret_str;
}

void TWubiIMC::clear()
{
	input.erase();
	if (cursor){
		cursor->close();
		cursor = NULL;
		}
	list_cnt = 0;	
	preword_cnt = 0;
	imc_stat &= ~IMC_STAT_NEW_WORD;
	update_stamp = last_update_stamp;
}

void TWubiIMC::setdisplay(uint16_t index, uint16_t count)
{
	assert (index + count <= list_count());

	displayindex = index;
	displaycount = count;
}

TWstring * TWubiIMC::display_input()
{
	ret_str.erase();
	if (is_new_word()){
		ret_str.copy(L'(');
		for (size_t i = 0; i < preword_cnt; i++)
			ret_str.append(preword[i].wr.w);
		ret_str.append(L") ");
		}
		
	ret_str.append(input);
	return &ret_str;
}

void TWubiIMC::start_new_word()
{
	clear();
	imc_stat |= IMC_STAT_NEW_WORD;
}

TWstring * TWubiIMC::get_finalword()
{
	ret_str.erase();
	for (size_t i = 0; i < preword_cnt; i++)
		ret_str.append(preword[i].wr.w);
	return &ret_str;
}

void TWubiIMC::find_word(List_Item * li, Dbc * dbcursor)
{
	// we have a char, with a key length is 1
	// so we need find full key of the char
	memcpy(&item, li, sizeof(List_Item));
	if (dbcursor->get(&worddb_key, &worddb_data, DB_SET_RANGE) != 0)
		assert(0);		//realy??

	do{
		if ((item.wr.w[0] == li->wr.w[0]) && (item.wr.w[1] == 0) && (item.wk.key[1] != 0))
			break;
		}while(dbcursor->get(&worddb_key, &worddb_data, DB_NEXT) == 0);
	memcpy(li, &item, sizeof(List_Item));
}

List_Item * TWubiIMC::get_finalitem(Dbc * dbcursor)
{
	if (preword_cnt < 2)
		return NULL;
	TWstring * t;
	t = get_finalword();
	
	Word_Key wk;
	switch(t->length()){
		case 2:
			if (strlen(preword[0].wk.key) == 1)
				find_word(&preword[0], dbcursor);
			if (strlen(preword[1].wk.key) == 1)
				find_word(&preword[1], dbcursor);
			wk.key[0] = preword[0].wk.key[0];
			wk.key[1] = preword[0].wk.key[1];
			wk.key[2] = preword[1].wk.key[0];
			wk.key[3] = preword[1].wk.key[1];
			wk.key[4] = 0;
			break;
		case 3:
			// 2 + 1
			if (wcslen(preword[0].wr.w) == 2){
				wk.key[0] = preword[0].wk.key[0];
				wk.key[1] = preword[0].wk.key[2];
				if (strlen(preword[1].wk.key) == 1)
					find_word(&preword[1], dbcursor);
				wk.key[2] = preword[1].wk.key[0];
				wk.key[3] = preword[1].wk.key[1];
				wk.key[4] = 0;
				break;
				}
			// 1 + 2
			if (wcslen(preword[1].wr.w) == 2){
				wk.key[0] = preword[0].wk.key[0];
				wk.key[1] = preword[1].wk.key[0];
				wk.key[2] = preword[1].wk.key[2];
				wk.key[3] = preword[1].wk.key[3];
				wk.key[4] = 0;
				break;
				}
			// 1 + 1 + 1
			wk.key[0] = preword[0].wk.key[0];
			wk.key[1] = preword[1].wk.key[0];
			if (strlen(preword[2].wk.key) == 1)
				find_word(&preword[2], dbcursor);
			wk.key[2] = preword[2].wk.key[0];
			wk.key[3] = preword[2].wk.key[1];
			wk.key[4] = 0;
			break;
		default:	// 4 or more chars word
			int pos = 0;
			
			for (size_t i = 0; pos < 3; i++)
				switch(wcslen(preword[i].wr.w)){
					case 1:
						wk.key[pos++] = preword[i].wk.key[0];
						break;
					case 2:
						wk.key[pos++] = preword[i].wk.key[0];
						wk.key[pos++] = preword[i].wk.key[2];
						break;
					default:
						wk.key[pos++] = preword[i].wk.key[0];
						wk.key[pos++] = preword[i].wk.key[1];
						wk.key[pos++] = preword[i].wk.key[2];
						break;
					}
			
			switch(wcslen(preword[preword_cnt - 1].wr.w)){
				case 1:
					wk.key[3] = preword[preword_cnt - 1].wk.key[0];
					break;
				case 2:
				case 3:
					wk.key[3] = preword[preword_cnt - 1].wk.key[2];
					break;
				default:
					wk.key[3] = preword[preword_cnt - 1].wk.key[3];
					break;
				}
			wk.key[4] = 0;
			break;
		}
	item.wk.copy(&wk);
	t->copyto(item.wr.w, MAX_WORD_LEN);
	return &item;
}

void TWubiIMC::add_preword(List_Item * li)
{
	if (preword_cnt == MAX_WORD_LEN)
		return;
	size_t len = 0;
	size_t i = 0;
	for (; i < preword_cnt; i++)
		len += wcslen(preword[i].wr.w);
	
	wcscpy(preword[i].wr.w, li->wr.w);
	len += wcslen(preword[i].wr.w);
	
	if (len > MAX_WORD_LEN)
		return;

	strcpy(preword[i].wk.key, li->wk.key);
	preword_cnt ++;
}

int TWubiIMC::del_last_preword()
{
	if (preword_cnt == 0)
		return 0;
	preword_cnt --;
	return 1;
}

void TWubiIMC::clear_list()
{
	input.erase();
	if (cursor){
		cursor->close();
		cursor = NULL;
		}
	list_cnt = 0;	
}
