#ifndef _PYPHRASER_H
#define _PYPHRASER_H

#include "simpleimc.h"

class TPYPhraser
{
private:
	Word_Key key;
	TWstring charlist[MAX_WORD_LEN + 1];
	int careH;
	int careNG;

private:
	int prefix_chk(char prefix, char * str);

public:
	TPYPhraser(int careHv, int careNGv);
	// return 0 when py is impossible,or return the char count can convert from py
	int build_key(TWstring & inputpy);
	int charfit(TWstring & fullpy, size_t index = 0);
	int wordfit(TWstring & fullpy, size_t c_cnt);
	TWstring * list_item(size_t index) { return &charlist[key.len - index - 1];}

	size_t char_count() { return key.len; }
	Word_Key * get_key() { return &key; }
};

#endif
