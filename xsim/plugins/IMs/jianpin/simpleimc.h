#ifndef _SIMPLEIMC_H
#define _SIMPLEIMC_H

#include <wchar.h>
#include <vector>

#include "imc.h"
#include "wstring.h"

using std::vector;

const uint8_t MAX_WORD_LEN = 9;
const uint8_t MAX_CHAR_PY_LEN = 6;
const uint8_t MAX_PY_LEN = MAX_WORD_LEN * MAX_CHAR_PY_LEN + MAX_WORD_LEN - 1; //  char + '
const uint16_t MAX_ACCESS_COUNT = (uint16_t) -1;

typedef struct Char_Key_tag
{
	char py[MAX_PY_LEN + 1];

	size_t size(){ return strlen(py) + 1; }
	void copy(struct Char_Key_tag * val) { memcpy(this, val, sizeof(Char_Key_tag));}
}Char_Key;

typedef struct Char_Rec_tag
{
	uint16_t access_count;
	wchar_t c;

	size_t size(){ return sizeof(Char_Rec_tag); }
	void copy(struct Char_Rec_tag * val) { memcpy(this, val, sizeof(Char_Rec_tag));}
}Char_Rec;

typedef struct Word_Key_tag
{
	uint8_t len;
	char py[MAX_PY_LEN + 1];

	size_t size(){ return 1 + strlen(py) + 1; }
	void copy(struct Word_Key_tag * val) { memcpy(this, val, sizeof(Word_Key_tag));}
}Word_Key;

typedef struct Word_Rec_tag
{
	uint16_t access_count;
	wchar_t w[MAX_WORD_LEN + 1];

	size_t ac_size() { return (uint32_t) w - (uint32_t)&access_count; }
	size_t size(){ return ac_size() + wcslen(w) * sizeof(wchar_t); }
	void copy(struct Word_Rec_tag * val) { memcpy(this, val, sizeof(Word_Rec_tag));}
}Word_Rec;

enum LIST_ITEM_TYPE_enu
{
	LI_WORD = 1,
	LI_CHAR,
};

typedef struct List_Item_tag
{
	LIST_ITEM_TYPE_enu type;

	Char_Key ck;
	Word_Key wk;
	Char_Rec cr;
	Word_Rec wr;
}List_Item;

typedef enum IMC_LIST_CLEAR_TYPE_emu
{
	IMC_CLEARALL = 1,
	IMC_CLEARWORD,
}IMC_LIST_CLEAR_TYPE;

const uint16_t IMC_STAT_DISABLE		= 0x0001;
const uint16_t IMC_STAT_MB_INPUT	= 0x0002;

class TSimpleIM;
class TSimpleIMC:public TIMC
{
private:
	typedef struct Char_Pack_tag
	{
		Char_Key ck;
		Char_Rec cr;
	}Char_Pack;

	typedef struct Word_Pack_tag
	{
		Word_Key wk;
		Word_Rec wr;
	}Word_Pack;

private:
	TWstring input;

	vector<Char_Pack> cpack;
	vector<Word_Pack> wpack;

	uint16_t displayindex, displaycount;

	List_Item ret_item;
	TWstring ret_str;

	TWstring preword;
	TWstring preword_input;
	TWstring preword_py;
	size_t preword_accesscount;

	size_t char_left;
	int caret;
	TWstring first_ch_py;
	uint16_t imc_stat;
	
public:
	TSimpleIMC();
	~TSimpleIMC();

	int need_rebuild_charlist(TWstring & ch_py);
	void set_first_char_py(TWstring & ch_py);

	void add_char(Char_Key * key, Char_Rec * data);
	void add_word(Word_Key * key, Word_Rec * data);

	uint16_t word_count() { return wpack.size(); }
	uint16_t char_count() { return cpack.size(); }
	void clear();
	void clearlist(IMC_LIST_CLEAR_TYPE clear_type);

	void set_input(TWstring * val) { input.copy(*val); }
	TWstring * get_input() { return &input; }

	void add_preword(List_Item * li);
	int word_complete();
	List_Item * get_finalword();
	void set_char_left(size_t left_cnt);
	size_t preword_len() { return preword.length(); }
	size_t preword_input_len() { return preword_input.length(); }
	void pop_preword();

	uint16_t list_count() { return char_count() + word_count(); }
	TWstring * list_str(uint16_t index, TWstring * suffix);
	List_Item * list_item(uint16_t index);

	void setdisplay(uint16_t index, uint16_t count);
	uint16_t display_count(){ return displaycount;}
	uint16_t display_index(){ return displayindex;}

	TWstring *display_input();
	List_Item * display_list_item(uint16_t index) { return list_item(displayindex + index); }

	void switch_stat(uint16_t stat_id) { imc_stat ^= stat_id; }
	
	int enabled() { return (imc_stat & IMC_STAT_DISABLE) ^ IMC_STAT_DISABLE;}

	int is_mb_input() { return imc_stat & IMC_STAT_MB_INPUT; }

	int is_preedit() { return preword.length() + input.length();}

	int caret_pos() { return caret; }
	void set_caret_pos(int pos) { caret = pos; }
};

#endif
