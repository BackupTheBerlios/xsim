#include "wstring.h"
#include "pyphraser.h"
#include "simpleimc.h"

struct PY_Phrase_tag
{
	char * tail;
	char * nomore;		// if prefix is in here, no more char should take
	char * more;		// if prefix is in here, more char could be add

	int splitpos;		// == 1 when the prefix and the tail can build one char pingyin
				// < 0  when the prefix and tail can not build a char pingyin,
				//		but the string after splitpos can build a char pingyin
				//		the prefix and the str before splitpos can build a char pingyin too
				// == 0 when the prefix and tail can never build one char pingyin nor any word pingyin
};

// hope not too much fault here :P, it take me hours...
const struct PY_Phrase_tag PY_PHRASE_LIST[] = {
	{"a", "bcdfgklmnpstwyz", "hiu", 1},
		{"ha", "csz", "", 1},
		{"ia", "djlqx", "", -1},
		{"ua", "gkr", "h", -1},
			{"hua", "csz", "", 1},
//	{"b", "", "", 1},
//	{"c", "", "", 1},
//	{"d", "", "", 1},
	{"e", "bcdgklmnrstyz", "hiu", 1},
		{"he", "csz", "", 1},
		{"ie", "bdjlmnpqtx", "", -1},
		{"ue", "jlnqxy", "", -1},
//	{"f", "", "", 1},
	{"g", "", "n", 1},
		{"ng", "", "aeio", 1},
			{"ang", "bcdfgklmnprstwyz", "hiu", 1},
				{"hang", "csz", "", 1},
				{"iang", "jlnqx", "", -1},
				{"uang", "gk", "h", -1},
					{"huang", "csz", "", 1},
			{"eng", "bcdfgklmnprstwz", "h", 1},
				{"heng", "csz", "", 1},
			{"ing", "bdjlmnpqtxy", "", -2},
			{"ong", "cdgklnrstyz", "hi", -2},
				{"hong", "cz", "", 1},
				{"iong", "jqx", "", -3},
	{"h", "", "csz", 1},
	{"i", "bcdjlmnpqrstxyz", "aehu", 0},
		{"hi", "", "csz", 0},
		{"ai", "bcdgklmnpstwz", "hu", 1},
			{"hai", "csz", "", 1},
			{"uai", "gk", "h", -1},
				{"huai", "csz", "", 1},
		{"ei", "bdfgklmnptwz", "h", 1},
			{"hei", "csz", "", 1},
		{"ui", "cdgkrstz", "h", 1},
			{"hui", "csz", "", 1},
//	{"j", "", "", 1},
//	{"k", "", "", 1},
//	{"l", "", "", 1},
//	{"m", "", "", 1},
	{"n", "", "aeiu", 1},
		{"an", "bcdfgklmnprstwyz", "hiu", 1},
			{"han", "csz", "", 1},
			{"ian", "bdjlmnpqtx", "", -1},
			{"uan", "cdgjklnqrstxyz", "h", -1},
				{"huan", "csz", "", 1},
		{"en", "bcdfgkmnprswz", "h", 1},
			{"hen", "csz", "", 1},
		{"in", "bjlmnpqxy", "", -1},
		{"un", "cdgjklnqrstxyz", "h", -1},
			{"hun", "csz", "", 1},
	{"o", "bfklmpwy", "ua", 1},
		{"ao", "bcdgklmnprstyz", "hi", 1},
			{"hao", "csz", "", 1},
			{"iao", "bdjlmnpqtx", "", -1},
		{"uo", "cdgklnrstz", "h", -1},
			{"huo", "csz", "", 1},
//	{"p", "", "", 1},
//	{"q", "", "", 1},
	{"r", "e", "", 1},
//	{"s", "", "", 1},
//	{"t", "", "", 1},
	{"u", "bcdfgjklmnpqrstwxyz", "hio", 0},
		{"hu", "", "csz", 1},
		{"iu", "djlmnqx", "", 0},
		{"ou", "cdfgklmnprstyz", "h", 1},
			{"hou", "csz", "", 1},
	{"v", "jlnqx", "", 0},
//	{"w", "", "", 1},
//	{"x", "", "", 1},
//	{"y", "", "", 1},
//	{"z", "", "", 1},

	{0, 0, 0, 0},
	};

int TPYPhraser::prefix_chk(char prefix, char * str)
{
	int i;
	for (i = 0; PY_PHRASE_LIST[i].tail != NULL; i++)
		if (strcmp(PY_PHRASE_LIST[i].tail, str) == 0)
			break;;

	if (PY_PHRASE_LIST[i].tail == NULL)
		return 1;

	if (strchr(PY_PHRASE_LIST[i].nomore, prefix))
		return 2;

	if (strchr(PY_PHRASE_LIST[i].more, prefix))
		return 3;

	size_t str_len = strlen(str);
	if (careNG || (str[1] == 0) || (str[str_len - 1] != 'n'))
		return PY_PHRASE_LIST[i].splitpos;

	// some spacial check needed when not care NG
	if ((str[str_len - 2] != 'a') && (str[str_len - 2] != 'e') && (str[str_len - 2] != 'i'))
		return PY_PHRASE_LIST[i].splitpos;

	char nextbuf[str_len + 2];
	strcpy(nextbuf, str);
	nextbuf[str_len] = 'g';
	nextbuf[str_len + 1] = 0;
	int ret;
	
	ret = prefix_chk(prefix, nextbuf);
	if ((ret + str_len) <= 0)
		ret ++;

	return ret;
}

TPYPhraser::TPYPhraser(int careHv, int careNGv)
{
	key.len = 0;
	
	careH = careHv;
	careNG = careNGv;
}

int TPYPhraser::build_key(TWstring & inputpy)
{
	TWstring clean_py;
	char mark[MAX_WORD_LEN + 1];
	int ret;
	int index;
	char pybuf[inputpy.length() + 1];

	for (index = 0; index < (int)inputpy.length(); index++)	// clean the space add by me
		if (inputpy[index] != ' ')
			clean_py.append(inputpy[index]);

	for (index = 0; index < (int)MAX_WORD_LEN + 1; index++)
		mark[index] = ' ';
		
	clean_py.tombs(pybuf, clean_py.length());
        key.len = 0;
	for(index = clean_py.length() - 1; index >= 0; index--){
		if (pybuf[index] == '\''){
			mark[key.len] = '\'';
			pybuf[index] = 0;
			index --;
			}
		assert(index >= 0);
		if (index > 0)
			ret = prefix_chk(pybuf[index - 1], &pybuf[index]);
			else
				ret = prefix_chk(' ', &pybuf[index]);

		switch(ret){
			case 0:
				return 0;

			case 1:
				strcpy(key.py, &pybuf[index]);
				pybuf[index] = 0;
				charlist[key.len].copy(key.py);
				key.len ++;
				break;

			case 2:
				index--;
				strcpy(key.py, &pybuf[index]);
				pybuf[index] = 0;
				charlist[key.len].copy(key.py);
				key.len ++;
				break;

			case 3:
				break;
				
			default:
				// add index by splitpos, but need minus :P
				index -= ret;
				strcpy(key.py, &pybuf[index]);
				pybuf[index] = 0;
				charlist[key.len].copy(key.py);
				key.len ++;
				break;
			}
		}

	inputpy.erase();

	for (index = key.len - 1; index > 0; index --){
		inputpy.append(charlist[index]);
		inputpy.append(mark[index]);
		}
	inputpy.append(charlist[0]);
	if (mark[0] == '\'')
		inputpy.append('\'');

	if ((!careH) && (key.py[1] != 0))	// we need modify key if not care h and len at least 2
		if  ((key.py[0] == 'c') || (key.py[0] == 's') || (key.py[0] == 'z'))
			if (key.py[1] != 'h')
				key.py[1] = 0;

	return 1;
}

int TPYPhraser::charfit(TWstring & fullpy, size_t index)
{
	size_t len = list_item(index)->length();
	size_t full_len = fullpy.length();
	TWstring py;
	py.copy(list_item(index)->data());
	int ret = 0;
	switch (len){
		case 1:
			if (fullpy[0] != py[0])
				break;
			if ((careH) && (full_len > 1) && (fullpy[1] == 'h'))
				break;
			ret = 1;
			break;
		case 2:
			if ((py[1] == 'h') && (fullpy.compare(py, len) == 0)){
				ret = 1;
				break;
				}
		default:
			if (!careH){
				if (py[1] == 'h'){
					py.erase(1, 1);
					len--;
					}
				if ((full_len > 1) && (fullpy[1] == 'h')){
					fullpy.erase(1, 1);
					full_len--;
					}
				}

			if (!careNG){
				if (py[len - 1] == 'g')
					py.erase(len - 1);
				if (fullpy[full_len - 1] == 'g')
					fullpy.erase(full_len - 1);
				}
			if (fullpy.compare(py) == 0){
				ret = 1;
				break;
				}
		}

	return ret;
}

// should be remove when the word file stable
#if 1
#include <iostream>
using std::cerr;
using std::endl;
#endif

int TPYPhraser::wordfit(TWstring & fullpy, size_t c_cnt)
{
        TWstring tmp;
	size_t i;	
	int pos1 = 0, pos2;
	
	for (i = 0; i < c_cnt - 1; i ++){
		pos2 = fullpy.find('\'', pos1);
#if 1		// the word file is come from turbolinux, and has some bug in it,
		// i have fixed some, but maybe more over there
		if (pos2 <= 0){
			cerr << "we got internal error, the word's char count = " << c_cnt
				<< " and py is \"";
			for (size_t kk = 0; kk < fullpy.length(); kk++)
				cerr << (char)fullpy[kk];
			cerr << "\"" << endl;
			cerr << "maybe the word file is bad. check the word file with the py" << endl;
			return 0;
			}
#endif
		assert(pos2 > 0);
		tmp.copy(fullpy, pos2 - pos1, pos1);

		if (!charfit(tmp, i))
			return 0;

		pos1 = pos2 + 1;
		}
	tmp.copy(fullpy, fullpy.length() - pos1, pos1);
	if (!charfit(tmp, i))
		return 0;
	return 1;
}
