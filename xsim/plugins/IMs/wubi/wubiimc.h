#ifndef _WUBIIMC_H
#define _WUBIIMC_H

#include <wchar.h>
#include <vector>
#include <db_cxx.h>
#include <time.h>

#include "imc.h"
#include "wstring.h"

using std::vector;

const uint8_t MAX_WORD_LEN = 12;
const uint8_t MAX_KEY_LEN = 4;
const uint16_t MAX_ACCESS_COUNT = (uint16_t) -1;

typedef struct Word_Key_tag
{
	char key[MAX_KEY_LEN + 1];

	size_t size(){ return strlen(key) + 1; }
	void copy(struct Word_Key_tag * val) { memcpy(this, val, sizeof(Word_Key_tag));}
}Word_Key;

typedef struct Word_Rec_tag
{
	wchar_t w[MAX_WORD_LEN + 1];

	size_t size(){ return (wcslen(w) + 1) * sizeof(wchar_t); }
	void copy(struct Word_Rec_tag * val) { memcpy(this, val, sizeof(Word_Rec_tag));}
}Word_Rec;

typedef struct List_Item_tag
{
	Word_Key wk;
	Word_Rec wr;
}List_Item;

const uint16_t IMC_STAT_DISABLE		= 0x0001;
const uint16_t IMC_STAT_MB_INPUT	= 0x0002;
const uint16_t IMC_STAT_NEW_WORD	= 0x0004;

class TWubiIM;
class TWubiIMC:public TIMC
{
private:
	static time_t last_update_stamp;
private:
	time_t update_stamp;
	TWstring input;

	TWstring ret_str;
	List_Item item;

	uint16_t imc_stat;

	uint16_t displayindex, displaycount;
	
	Dbt worddb_key, worddb_data;

	Dbc * cursor;
	uint16_t cursor_index;

	db_recno_t list_cnt;
	
	List_Item preword[MAX_WORD_LEN];
	
	size_t preword_cnt;

public:
	TWubiIMC();
	~TWubiIMC();

	void clear();

	TWstring * get_input() { return &input; }
	void set_input(TWstring * val) { input.copy(val->data()); }

	void set_cursor(Dbc * db_cursor);

	uint16_t list_count() { return list_cnt; }
	TWstring * list_str(uint16_t index, TWstring * suffix);
	List_Item * list_item(uint16_t index);

	void setdisplay(uint16_t index, uint16_t count);
	uint16_t display_count(){ return displaycount;}
	uint16_t display_index(){ return displayindex;}

	TWstring *display_input();
	List_Item * display_list_item(uint16_t index) { return list_item(displayindex + index); }

	void start_new_word();
	void add_preword(List_Item * li);
	TWstring * get_preword();
	List_Item * get_finalitem(Dbc * dbcursor);
	TWstring * get_finalword();
	int del_last_preword();
	
	void clear_list();
	void find_word(List_Item * li, Dbc * dbcursor);
	
	void switch_stat(uint16_t stat_id) { imc_stat ^= stat_id; }
	int enabled() { return (imc_stat & IMC_STAT_DISABLE) ^ IMC_STAT_DISABLE;}
	int is_mb_input() { return imc_stat & IMC_STAT_MB_INPUT; }
	int is_new_word() { return imc_stat & IMC_STAT_NEW_WORD; }
	int is_preedit();

	static void set_update_stamp(time_t val) { last_update_stamp = val; }
};

#endif
