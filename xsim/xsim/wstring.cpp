#include <wchar.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "wstring.h"

const uint16_t DEFAULT_STR_LEN = 5;

wchar_t * alloc_new_buf(size_t &len)
{
	wchar_t * tmp;
	len = (len > DEFAULT_STR_LEN) ? len + DEFAULT_STR_LEN :DEFAULT_STR_LEN;
	tmp = new wchar_t[len + 1];
	tmp[0] = wchar_t(0);
	return tmp;
}

TWstring::TWstring()
{
	buf_len = DEFAULT_STR_LEN;
	str = alloc_new_buf(buf_len);
}

TWstring::TWstring(const TWstring & val)
{
	TWstring * tmp = (TWstring *)&val;

	buf_len = tmp->length();
	str = alloc_new_buf(buf_len);
	copy(tmp->data());
}

TWstring::~TWstring()
{
	delete str;
}

wchar_t & TWstring::operator [](size_t index)
{
	assert(index < wcslen(str));

	return str[index];
}

int TWstring::find(const wchar_t val, size_t pos)
{
	for (size_t i = pos; i < wcslen(str); i++)
		if (str[i] == val)
			return i;

	return -1;
}

int TWstring::find(const wchar_t * val, size_t pos)
{
	wchar_t * loc = wcsstr(&str[pos], val);
	if (!loc)
		return -1;
	return loc - str;
}

void TWstring::rtrim()
{
	size_t i;
	for (i = length() - 1; i >=0; i--)
		if ((str[i] != ' ') && (str[i] != '\t'))
			break;

	str[i + 1] = 0;
}

void TWstring::ltrim()
{
	size_t i, j;
	for ( i = 0; i < length(); i++)
		if ((str[i] != ' ') && (str[i] != '\t'))
			break;

	if (i == 0)
		return;

	for (j = 0; i < length(); j++, i++)
		str[j] = str[i];

	str[j] = 0;
}

void TWstring::copy(const char * val)
{
	wchar_t dummy[strlen(val) + 1];
	size_t len;

	len = mbstowcs(dummy, val, strlen(val));
	dummy[len] = wchar_t(0);
	copy(dummy);
}

void TWstring::copy(const wchar_t * val)
{
	size_t len;

	len = wcslen(val);

	if (buf_len < len){
		delete str;
		buf_len = len;
		str = alloc_new_buf(buf_len);
		}
	wcscpy(str, val);
}

void TWstring::copy(const wchar_t * val, size_t len, size_t pos)
{
	if (buf_len < len){
		delete str;
		buf_len = len;
		str = alloc_new_buf(buf_len);
		}
	wcsncpy(str, &val[pos], len);
	str[len] = wchar_t(0);
}

void TWstring::append(const char * val)
{
	wchar_t dummy[strlen(val) + 1];
	size_t len;

	len = mbstowcs(dummy, val, strlen(val));
	dummy[len] = wchar_t(0);
	append(dummy);
}

void TWstring::append(const wchar_t * val)
{
	wchar_t *tmp;
	size_t len;

	len = wcslen(str);
	len += wcslen(val);

	if (buf_len < len){
		buf_len = len;
		tmp = alloc_new_buf(buf_len);
		wcscpy(tmp, str);
		wcscat(tmp, val);
		delete str;
		str = tmp;
		}else
			wcscat(str, val);
}

void TWstring::append(const wchar_t val)
{
	wchar_t buf[2];
	buf[0] = val;
	buf[1] = wchar_t(0);

	append(buf);
}

void TWstring::insert(const wchar_t val, size_t pos)
{
	TWstring tmp;
	tmp.copy(str, pos);
	tmp.append(val);
	tmp.append(&str[pos]);
	copy(tmp);
}

void TWstring::erase(size_t pos)
{
	str[pos] = wchar_t(0);
}

void TWstring::erase(size_t pos, size_t len)
{
	assert(pos < length());

	size_t str_len = length();
	if (str_len <= pos + len){
		str[pos] = 0;
		return;
		}
		
	size_t i = pos;
	size_t j = pos + len;
	for (; j < str_len; j++, i++)
		str[i] = str[j];
	str[i] = 0;
}

int TWstring::compare(const wchar_t * val)
{
	return wcscmp(str, val);
}

int TWstring::compare(const wchar_t * val, size_t len)
{
	return wcsncmp(str, val, len);
}

int TWstring::casecompare(const wchar_t * val)
{
#ifdef __USE_GNU
	return wcscasecmp(str, val);
#else		// maybe need
	size_t len1 = wcslen(val);
	size_t len2 = length();
	return casecompare(val, (len1 >= len2) ?len1 :len2);
#endif
}

int TWstring::casecompare(const wchar_t * val, register size_t len)
{
#ifdef __USE_GNU
	return wcsncasecmp(str, val, len);
#else		// maybe need
	register int cmp;
	register wchar_t c1;
	register wchar_t c2;
	for (register size_t i = 0; i < len; i++){
		c1 = str[i];
		c2 = val[i];
		if ((c1 >= 'A') && (c1 <= 'Z'))
			c1 |= 0x60;
		if ((c2 >= 'A') && (c2 <= 'Z'))
			c2 |= 0x60;
		if ((cmp = c1 - c2) != 0)
			return cmp;
		}
	return 0;		
#endif
}

void TWstring::copyto(wchar_t * dest, size_t size)
{
	wcsncpy(dest, str, size);
}

wchar_t * TWstring::data()
{
	return str;
}

size_t TWstring::length()
{
	return wcslen(str);
}

char * TWstring::tombs(char * mbstr, size_t len)
{
	int ret;

	ret = wcstombs(mbstr, str, len);
	if (ret == -1)
		mbstr[0] = 0;

	mbstr[ret] = 0;
	return mbstr;
}

int TWstring::toint()
{
//	return wcstol(str, NULL, 10);
	char dummy[10];
	int i;
	tombs(dummy, 9);
	sscanf(dummy, "%d", &i);
	return i;
}

void TWstring::fromint(int v)
{
//	wchar_t dummy[5];
//	swprintf(dummy, 5, L"%d");
	char dummy[10];
	sprintf(dummy, "%d", v);
	copy(dummy);	
}

#ifdef DEBUG
#include <iostream>
using std::cerr;

void TWstring::print()
{
	char dummy[length() + 1];

	tombs(dummy, length());
	cerr << dummy;
}
#endif
