#ifndef _TABLEIMC_H
#define _TABLEIMC_H

#include <cwchar>
#include <db_cxx.h>

#include "imc.h"
#include "wstring.h"

const uint8_t MAX_WORD_LEN = 20;
const uint8_t MAX_KEY_LEN = 20;

typedef struct Word_Key_tag
{
	char key[MAX_KEY_LEN + 1];

	size_t size(){ return strlen(key) + 1; }
	void copy(struct Word_Key_tag * val) { memcpy(this, val, sizeof(Word_Key_tag));}
}Word_Key;

typedef struct Word_Rec_tag
{
	uint8_t sort_index;
	wchar_t w[MAX_WORD_LEN + 1];

	size_t si_size() { return (uint32_t) w - (uint32_t)&sort_index; }
	size_t size(){ return si_size() + (wcslen(w) + 1) * sizeof(wchar_t); }
	void copy(struct Word_Rec_tag * val) { memcpy(this, val, sizeof(Word_Rec_tag));}
}Word_Rec;

const uint16_t IMC_STAT_DISABLE		= 0x0001;

class TTableIM;
class TTableIMC:public TIMC
{
private:
	TWstring input;

	TWstring ret_str;

	uint16_t imc_stat;

	uint16_t displayindex, displaycount;
	
	Dbt worddb_key, worddb_data;
	
	Dbc * cursor;
	uint16_t cursor_index;

        Word_Key wkey;
        Word_Rec wrec;

	db_recno_t list_cnt;
public:
	TTableIMC();
	~TTableIMC();

	void clear();

	TWstring * get_input() { return &input; }
	void set_input(TWstring * val) { input.copy(val->data()); }

	void set_cursor(Dbc * db_cursor);

	uint16_t list_count() { return list_cnt; }
	TWstring * list_str(uint16_t index, TWstring * suffix);

	void setdisplay(uint16_t index, uint16_t count);
	uint16_t display_count(){ return displaycount;}
	uint16_t display_index(){ return displayindex;}

	TWstring *display_input();

	void switch_stat(uint16_t stat_id) { imc_stat ^= stat_id; }
	int enabled() { return (imc_stat & IMC_STAT_DISABLE) ^ IMC_STAT_DISABLE;}
	int is_preedit() { return (input.empty()) ?0 :1; }

};

#endif
