#include <iostream>
#include <string>
#include <db_cxx.h>
#include <locale.h>
#include <wchar.h>

#include "../simpleimc.h"

using std::cerr;
using std::cout;
using std::endl;
using std::cin;

int main(int argc, char * argv[])
{
	setlocale(LC_ALL, "zh_CN.GBK");

	Db db(NULL, 0);
	db.set_flags(DB_DUP | DB_DUPSORT);
	if (argc > 1)
		db.open(argv[1], NULL, DB_BTREE, DB_RDONLY, 0644);
		else
			db.open("chardb", NULL, DB_BTREE, DB_RDONLY, 0644);


	Char_Key ck;
	Char_Rec cr;
	char output[256];

	cout << "input key:";
	cin >> ck.py;
	Dbt key(&ck, sizeof(Char_Key));
	Dbt data(&cr, sizeof(Char_Rec));
	key.set_flags(DB_DBT_USERMEM);
	key.set_ulen(sizeof(Char_Key));
	data.set_flags(DB_DBT_USERMEM);
	data.set_ulen(sizeof(Char_Rec));

	Dbc * cursor;
	key.set_size(ck.size());
	db.cursor(0, &cursor, 0);

	int i = 0;
	if (cursor->get(&key, &data, DB_SET)){
		cerr << "nothing found" << endl;
		goto UGLY_EXIT_GOTO;
		}

	for (;;){
		if (wctomb(output, cr.c) == -1){
			cout << "failed converting char to mbs" << endl;
			exit (0);
			}
		output[2] = 0;
		cout << ck.py << " --> " << output << " accesscount = " << cr.access_count << " code = " << cr.c << endl;
		i++;
		if (cursor->get(&key, &data, DB_NEXT_DUP))
			break;
		}

	cout << i << " char(s) found" << endl;

UGLY_EXIT_GOTO:
	db.close(0);
	exit (0);
}
