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

#include <iostream>
#include <string>
#include <db_cxx.h>
#include <locale.h>
#include <wchar.h>

#include "../tableimc.h"

using std::cin;
using std::cerr;
using std::cout;
using std::endl;

int main(int argc, char * argv[])
{

	Db db(NULL, 0);
	db.set_flags(DB_DUP | DB_DUPSORT);
	switch(argc){
		case 2:
			db.open(argv[1], NULL, DB_BTREE, DB_RDONLY, 0644);
			setlocale(LC_ALL, "zh_CN.GBK");
			break;
		case 3:
			db.open(argv[1], NULL, DB_BTREE, DB_RDONLY, 0644);
			setlocale(LC_ALL, argv[2]);
			break;
		case 1:
		default:
			db.open("worddb.wubi", NULL, DB_BTREE, DB_RDONLY, 0644);
			setlocale(LC_ALL, "zh_CN.GBK");
		}

	Word_Key wk;
	Word_Rec wr;
	char output[256];

	cout << "input key:";
	cin >> wk.key;
	
	Dbt key(&wk, wk.size());
	Dbt data(&wr, sizeof(Word_Rec));
	key.set_flags(DB_DBT_USERMEM);
	key.set_ulen(sizeof(Word_Key));
	data.set_flags(DB_DBT_USERMEM);
	data.set_ulen(sizeof(Word_Rec));

	Dbc * cursor;
	db.cursor(0, &cursor, 0);
        
        memset(&wr, 0, sizeof(wr));

	int i = 0;
	if (cursor->get(&key, &data, DB_SET)){
		cerr << "nothing found" << endl;
		goto UGLY_EXIT_GOTO;
		}

	for (;;){
		if (wcstombs(output, wr.w, 255) == (size_t)-1){
			cout << "failed converting word to mbs" << endl;
			}
		cout << wk.key << " --> " << output << " code = ";
		for (int j = 0; wr.w[j] != 0; j++){
			cout << wr.w[j] << " ";
			}
		cout << endl;
		i++;
	        memset(&wr, 0, sizeof(wr));
		if (cursor->get(&key, &data, DB_NEXT_DUP))
			break;
		}

	cout << i << " word(s) found" << endl;

UGLY_EXIT_GOTO:
	db.close(0);
	exit (0);
}
