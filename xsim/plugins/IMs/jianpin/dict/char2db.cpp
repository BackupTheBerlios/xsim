#include <iostream>
#include <string>
#include <db_cxx.h>
#include <locale.h>
#include <vector>
#include <fstream>

#include "../simpleimc.h"

using std::vector;
using std::cerr;
using std::cout;
using std::endl;
using std::filebuf;
using std::ios_base;
using std::istream;

void usage()
{
	cerr << "Usage: char2db src-file dest-file used-locale charfreq-file" << endl;
	exit (1);
}

#ifdef BDB_VERSION4
int db_compare(DB * db, const DBT * dbt1, const DBT * dbt2)
#else
int db_compare(const DBT * dbt1, const DBT * dbt2)
#endif
{
	Char_Rec *d1 = (Char_Rec *)dbt1->data;
	Char_Rec *d2 = (Char_Rec *)dbt2->data;

	return wcsncmp(&d1->c, &d2->c, 1);
}

typedef struct Char_Freq_tag
{
	wchar_t c;
	unsigned short freq;	// more small more freq @@
}Char_Freq;

vector<Char_Freq> char_freq_tab;

void build_freq_tab(char * filename)
{
        filebuf f;

#ifdef _CPP_BITS_IOSBASE_H
	f.open(filename, ios_base::in);
#else
	f.open(filename, "r");
#endif
	istream in(&f);

	char cbuf[10];
	unsigned long cnt, prev_cnt;
	unsigned short freq_num;
	Char_Freq ch;
	int i = 0;

	freq_num = MAX_ACCESS_COUNT - 10;
	prev_cnt = 9999;

	for (;;){
		in >> cnt >> cbuf;

		if (in.eof())
			break;

		if (strlen(cbuf) != 2){
			cout << "error in freq file on line: " << i << endl;
			exit(1);
			}

		if (mbtowc(&ch.c, cbuf, 4) == -1){
			cout << "error converting char in freq file on line: " << i << endl;
			exit(1);
			}

		ch.freq = freq_num;
		char_freq_tab.push_back(ch);
		
		if (cnt != prev_cnt){
			prev_cnt = cnt;
			freq_num--;
			}
		i++;
		}
	f.close();
	
	cout << " min freq is: " << freq_num << endl;
}

unsigned get_freq(wchar_t ch)
{
	for (unsigned i = 0; i < char_freq_tab.size(); i++)
		if (char_freq_tab[i].c == ch)
			return char_freq_tab[i].freq;

	return MAX_ACCESS_COUNT;
}

int main(int argc, char * argv[])
{
	if (argc != 5)
		usage();

	if (setlocale(LC_ALL, argv[3]) == NULL){
		cerr << "Failed setting locale to " << argv[3] << endl;
		exit(1);
		}

	build_freq_tab(argv[4]);

	Db db(NULL, 0);
	Dbc * cur;
	db.set_flags(DB_DUP | DB_DUPSORT);
	db.set_dup_compare(db_compare);
	db.open(argv[2], NULL, DB_BTREE, DB_CREATE, 0644);
	db.cursor(0, &cur, 0);

        filebuf f;

#ifdef _CPP_BITS_IOSBASE_H
	f.open(argv[1], ios_base::in);
#else
	f.open(argv[1], "r");
#endif
	istream in(&f);

	char mark;
	Char_Key ckey;
	Char_Rec rec;
	char dummy[256];
	char c[10];

	Dbt key(&ckey, sizeof(ckey));
	Dbt data(&rec, sizeof(Char_Rec));

	int i = 0;
	for (;;){
		in >> mark;
		in.unget();
		if (mark == '%'){			//for comments
			in.getline(dummy, 255);
			if (in.eof())
				break;
			continue;
			}
		in >> ckey.py >> c;

		if (in.eof())
			break;

		if (strlen(c) != 2){
			cout << "error on line: " << i << endl;
			exit(1);
			}

		key.set_size(ckey.size());
		mbtowc(&rec.c, c, 4);

		rec.access_count = get_freq(rec.c);

		if (cur->put(&key, &data, DB_KEYFIRST)){
			cout << "error on key: " << ckey.py << " data " << rec.c << " at line " << i + 1 << endl;
			db.close(0);
			exit(1);
			}
		i++;
		}

	db.close(0);
	cout << i << " line(s) converted" << endl;
	exit (0);
}
