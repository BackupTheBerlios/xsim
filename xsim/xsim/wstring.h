#ifndef _WSTRING_H
#define _WSTRING_H

#include <inttypes.h>
#include <string.h>

const uint16_t MAX_PATH_LEN = 255;		//count as mb

//class TWstring:public basic_string <wchar_t>
class TWstring
{
private:
	wchar_t * str;
	size_t buf_len;
public:
	TWstring(const TWstring & val);
	TWstring();
	~TWstring();

	wchar_t & operator [](size_t index);

	int empty() { return (str[0] == 0) ?1 :0; }
	void rtrim();
	void ltrim();
	void trim() { rtrim(); ltrim(); }

	void copy(const char * val);
	void copy(TWstring & val) { copy(val.data()); }
	void copy(const wchar_t val) { str[0] = val; str[1] = 0; }
	void copy(const wchar_t * val);
	void copy(TWstring & val, size_t len, size_t pos = 0) { copy(val.data(), len, pos); }
	void copy(const wchar_t * val, size_t len, size_t pos = 0);

	void append(const char * val);
	void append(TWstring & val) { append(val.data()); }
	void append(const wchar_t * val);
	void append(const wchar_t val);

	void insert(const wchar_t val, size_t pos);

	void erase(size_t pos = 0);
	void erase(size_t pos, size_t len);

	int compare(TWstring & val) { return compare(val.data()); }
	int compare(const wchar_t * val);
	int compare(TWstring & val, size_t len) { return compare(val.data(), len); }
	int compare(const wchar_t * val, size_t len);

	int casecompare(TWstring & val) { return casecompare(val.data()); }
	int casecompare(const wchar_t * val);
	int casecompare(TWstring & val, size_t len) { return casecompare(val.data(), len); }
	int casecompare(const wchar_t * val, size_t len);

	int find(const wchar_t val, size_t pos = 0);
	int find(const wchar_t * val, size_t pos = 0);
	int find(TWstring & val, size_t pos = 0) { return find(val.data(), pos); }

	void copyto(wchar_t * dest, size_t size);
	wchar_t * data();
	size_t length();

	char * tombs(char * mbstr, size_t len);

	int toint();
	void fromint(int v);
	
#ifdef DEBUG
	void print();
#endif
};

#endif
