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
#include <fstream>
#include <string>
#include <db_cxx.h>
#include <clocale>

#include "../tableimc.h"

using std::cerr;
using std::cout;
using std::endl;
using std::filebuf;
using std::ios_base;
using std::istream;

void usage()
{
	cerr << "Usage: word2db src-file dest-file used-locale" << endl;
	exit (1);
}

#ifdef BDB_VERSION4
int db_compare(DB * db, const DBT * dbt1, const DBT * dbt2)
#else
int db_compare(const DBT * dbt1, const DBT * dbt2)
#endif
{
	Word_Rec * d1 = (Word_Rec *)dbt1->data;
	Word_Rec * d2 = (Word_Rec *)dbt2->data;
	
	uint8_t ret = d1->sort_index - d2->sort_index;
	if (ret != 0)
		return ret;
		
	return wcscmp(d1->w, d2->w);
	
}

int main(int argc, char * argv[])
{
	if (argc != 4)
		usage();

	if (setlocale(LC_ALL, argv[3]) == NULL){
                cerr << "Failed setting locale to " << argv[3] << endl;
                exit(1);
                }

	Db db(NULL, 0);
	db.set_flags(DB_DUP | DB_DUPSORT);
	db.set_dup_compare(db_compare);
	db.open(argv[2], NULL, DB_BTREE, DB_CREATE, 0644);

        filebuf f;

#ifdef _CPP_BITS_IOSBASE_H
	f.open(argv[1], ios_base::in);
#else
	f.open(argv[1], "r");
#endif
	istream in(&f);

	char mark;
	Word_Key wk;
	Word_Rec rec;
	char w[50];
	wchar_t wbuf[100];
	int ret;

	Dbt key(&wk, sizeof(wk));
	Dbt data(&rec, sizeof(rec));

	int i = 0;
	int j = 0;
	int errcnt = 0;
	size_t wc;
	memset(&rec, 0, sizeof(rec));
	for (;;){
		j++;
		in >> mark;
		in.unget();
		if ((mark == '%') || (mark == '#')){			//for comments
			in.getline((char *)wbuf, 1000);
			if (in.eof())
				break;
			//cerr << " skip line:" << j << endl;
			continue;
			}

		in >> wk.key >> w;

		if (in.eof())
			break;

		if ((wk.key[0] < 'a') || (wk.key[0] > 'z')){
			//cerr << " error on line:" << j << endl;
			errcnt ++;
			continue;
			}

		wc = mbstowcs(wbuf, w, 99);
		if (wc > MAX_WORD_LEN){
			//cerr << " too long word at line:" << j << endl;
			continue;
			}
		wbuf[wc] = 0;
		wcscpy(rec.w, wbuf);

		key.set_size(wk.size());
		data.set_size(rec.size());
		ret = db.put(0, &key, &data, 0);
		if ((ret != 0) && (ret != DB_KEYEXIST)){
			db.err(ret, NULL);
			cout << "error on key: " << wk.key << " data " << w << " line : " << j << " ret: " << ret << endl;
			db.close(0);
			exit(1);
			}
		i++;
		}

	db.close(0);
	if (i == 0){
		cout << "Nothing converted, maybe the word file format not correct" << endl;
		exit (1);
		}
	cout << i << " line(s) converted" << endl;
	if (errcnt)
		cout << errcnt << " error(s) when convert, ignored." << endl;
	exit (0);
}
