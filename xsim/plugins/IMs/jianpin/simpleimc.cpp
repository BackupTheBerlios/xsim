#include <assert.h>
#include <string.h>

#include "simpleimc.h"

TSimpleIMC::TSimpleIMC()
{
	displayindex = displaycount = 0;
	imc_stat = 0;
	caret = 0;
}

TSimpleIMC::~TSimpleIMC()
{
}

void TSimpleIMC::add_word(Word_Key * key, Word_Rec * data)
{
	Word_Pack wp;

	wp.wk.copy(key);
	wp.wr.copy(data);
	if (data->access_count != MAX_ACCESS_COUNT)
		for (size_t i = 0; i < wpack.size(); i++)
			if ((data->access_count < wpack[i].wr.access_count) &&
				(key->len >= wpack[i].wk.len)){
				wpack.insert(wpack.begin() + i, wp);
				return;
				}
	wpack.push_back(wp);
}

void TSimpleIMC::add_char(Char_Key * key, Char_Rec * data)
{
	Char_Pack cp;

	cp.ck.copy(key);
	cp.cr.copy(data);
	int found = 0;
	size_t pos=0;

	for (size_t i = 0; i < cpack.size(); i++){
		if (data->c == cpack[i].cr.c)
			return;
		if (found)
			continue;
		if (data->access_count < cpack[i].cr.access_count){
			pos = i;
			found = 1;
			}
		}
	if (found)
		cpack.insert(cpack.begin() + pos, cp);
		else
			cpack.push_back(cp);
}

size_t findsplitpos(wchar_t * s, size_t cnt)
{
	size_t len = wcslen(s);
	size_t counter = 0;

	for (size_t i = 0; i < len; i++){
		if ((s[i] == ' ') || (s[i] == '\''))
			counter ++;
		if (counter == cnt)
			return i;
		}
	assert(0);
}

void TSimpleIMC::add_preword(List_Item * li)
{
	int cut_cnt;

	if (preword.empty())
		preword_accesscount = li->cr.access_count;
		else
			preword_py.append('\'');

	if (li->type == LI_CHAR){
		preword.append(li->cr.c);
		preword_py.append(li->ck.py);
		char_left --;
		cut_cnt = 1;
		}else{
			preword.append(li->wr.w);
			preword_py.append(li->wk.py);
			cut_cnt = wcslen(li->wr.w);
			char_left -= cut_cnt;
			}

	if (char_left != 0){
		size_t pos = 0;
		TWstring tmp;
		tmp.copy(input);
		pos = findsplitpos(tmp.data(), cut_cnt);
		pos ++;
		input.copy(tmp, tmp.length() - pos, pos);
		tmp.erase(pos);
		preword_input.append(tmp);
		}else
			input.erase();
}

void TSimpleIMC::pop_preword()
{
	if (preword.empty())
		return;
	preword_input.append(input.data());
	input.copy(preword_input.data());
	preword.erase();
	preword_py.erase();
	preword_input.erase();
}

int TSimpleIMC::word_complete()
{
	if (char_left)
		return 0;
	return 1;
}

List_Item * TSimpleIMC::get_finalword()
{
	assert(char_left == 0);

	if (preword.length() == 1){
		ret_item.type = LI_CHAR;
		preword_py.tombs(ret_item.ck.py, sizeof(ret_item.ck.py));
		ret_item.cr.c = preword[0];
		ret_item.cr.access_count = preword_accesscount;
		}else{
			ret_item.type = LI_WORD;
			ret_item.wk.len = preword.length();
			preword_py.tombs(ret_item.wk.py, sizeof(ret_item.wk.py));
//			preword.copyto(ret_item.wr.w, sizeof(ret_item.wr.w));
			preword.copyto(ret_item.wr.w, preword.length() + 1);
			ret_item.cr.access_count = MAX_ACCESS_COUNT;
			}
	return &ret_item;
}

int TSimpleIMC::need_rebuild_charlist(TWstring & ch_py)
{
	return first_ch_py.compare(ch_py.data());
}

void TSimpleIMC::set_first_char_py(TWstring & ch_py)
{
	first_ch_py.copy(ch_py);
}

void TSimpleIMC::set_char_left(size_t left_cnt)
{
	char_left = left_cnt;
}

List_Item * TSimpleIMC::list_item(uint16_t index)
{
	assert (index < (word_count() + char_count()));

	if (index < word_count()){
		ret_item.type = LI_WORD;
		ret_item.wk.copy(&(wpack[index].wk));
		ret_item.wr.copy(&(wpack[index].wr));
		}else{
			ret_item.type = LI_CHAR;
			ret_item.ck.copy(&(cpack[index - word_count()].ck));
			ret_item.cr.copy(&(cpack[index - word_count()].cr));
			}

	return &ret_item;
}

TWstring * TSimpleIMC::list_str(uint16_t index, TWstring * suffix)
{
	List_Item * li = list_item(index);
	
	if (li->type == LI_CHAR){
		ret_str.copy(&li->cr.c, 1);
		}else
			ret_str.copy(li->wr.w);

	if (suffix)
		suffix->erase();			
	return &ret_str;
}

void TSimpleIMC::clearlist(IMC_LIST_CLEAR_TYPE clear_type)
{
	displayindex = displaycount = 0;
	if (clear_type == IMC_CLEARALL){
		cpack.clear();
		first_ch_py.erase();
		}
	wpack.clear();
}

void TSimpleIMC::clear()
{
	input.erase();

	preword.erase();
	preword_py.erase();
	preword_input.erase();

	char_left = 0;
	caret = 0;
	//imc_stat = 0;
	//clearlist(IMC_CLEARALL);
}

void TSimpleIMC::setdisplay(uint16_t index, uint16_t count)
{
	assert (index + count <= list_count());

	displayindex = index;
	displaycount = count;
}

TWstring * TSimpleIMC::display_input()
{
	ret_str.copy(preword.data());
	ret_str.append(input.data());
	return &ret_str;
}
