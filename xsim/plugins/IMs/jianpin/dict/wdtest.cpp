#include <iostream>
#include <string>
#include <db_cxx.h>
#include <locale.h>
#include <wchar.h>

#include "../simpleimc.h"

using std::cin;
using std::cerr;
using std::cout;
using std::endl;

int main(int argc, char * argv[])
{
	setlocale(LC_ALL, "zh_CN.GBK");

	Db db(NULL, 0);
	db.set_flags(DB_DUP | DB_DUPSORT);
	if (argc > 1)
		db.open(argv[1], NULL, DB_BTREE, DB_RDONLY, 0644);
		else
			db.open("worddb", NULL, DB_BTREE, DB_RDONLY, 0644);


	Word_Key wk;
	Word_Rec wr;
	char output[256];

	cout << "input key:";
	cin >> wk.py;
	wk.len = 2;
	
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
			//exit (0);
			}
		cout << wk.py << " --> " << output << " accesscount = " << wr.access_count << " code = ";
		for (int j = 0; wr.w[j] != 0; j++){
			cout << wr.w[j] << " ";
			}
		cout << endl;
		i++;
		if (cursor->get(&key, &data, DB_NEXT_DUP))
			break;
		}

	cout << i << " word(s) found" << endl;
UGLY_EXIT_GOTO:
	db.close(0);
	exit (0);

/*
	char a[50];
	Char_Rec cr;
	db_recno_t cnt;

	Dbt key(a, strlen(a));
	Dbt data(&cr, sizeof(Char_Rec));
	key.set_flags(DB_DBT_USERMEM);
	key.set_ulen(sizeof(a));
	data.set_flags(DB_DBT_USERMEM);
	data.set_ulen(sizeof(Char_Rec));

	Dbc * cursor;
	db.cursor(0, &cursor, 0);

	int i = 0;
	while (cursor->get(&key, &data, DB_NEXT_NODUP) == 0){
		a[key.get_size()] = 0;
		i++;
		cursor->get(&key, &data, DB_SET);
		cursor->count(&cnt, 0);
		cout << a << " --> " << (char *)&cr.c << " count = " << cnt <<endl;;
		}

	db.close(0);
	cout << i << " keys found" << endl;

	exit (0);
*/
}
