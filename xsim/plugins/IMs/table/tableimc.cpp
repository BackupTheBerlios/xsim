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

#include <cassert>

#include "tableimc.h"
#include "tableim.h"

TTableIMC::TTableIMC()
{
	displayindex = displaycount = 0;
	imc_stat = 0;

	cursor = NULL;
	list_cnt = 0;
	worddb_key.set_ulen(sizeof(Word_Key));
	worddb_key.set_flags(DB_DBT_USERMEM);
	worddb_key.set_data(&wkey);
	worddb_data.set_ulen(sizeof(Word_Rec));
	worddb_data.set_flags(DB_DBT_USERMEM);
	worddb_data.set_data(&wrec);
}

TTableIMC::~TTableIMC()
{
	if (cursor)
		cursor->close();
}

void TTableIMC::set_cursor(Dbc * db_cursor)
{
	if(cursor)
		cursor->close();

        cursor = db_cursor;
	cursor_index = 0;
	list_cnt = 0;
	
	if (!cursor)
		return;
	
	input.tombs(wkey.key, MAX_KEY_LEN);
	worddb_key.set_size(wkey.size());

	if (cursor->get(&worddb_key, &worddb_data, DB_SET) != 0){
		cursor->close();
		cursor = NULL;
		return;
		}

	cursor->count(&list_cnt, 0);
}

TWstring * TTableIMC::list_str(uint16_t index, TWstring * suffix)
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

	for (; cursor_index != index; cursor_index += step)
		if (cursor->get(&worddb_key, &worddb_data, flag) != 0)
			assert(0);			// tell me why

	cursor->get(&worddb_key, &worddb_data, DB_CURRENT);

	ret_str.copy(wrec.w);
	return &ret_str;
}

void TTableIMC::clear()
{
	input.erase();
	if (cursor){
		cursor->close();
		cursor = NULL;
		}
	list_cnt = 0;	
}

void TTableIMC::setdisplay(uint16_t index, uint16_t count)
{
	assert (index + count <= list_count());

	displayindex = index;
	displaycount = count;
}

TWstring * TTableIMC::display_input()
{
	ret_str.copy(input);
	return &ret_str;
}
