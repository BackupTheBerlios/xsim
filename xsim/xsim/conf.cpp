#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>

#include <X11/Xlib.h>
#include <IMdkit.h>

#include "conf.h"

using std::cerr;
using std::endl;
using std::istream;
using std::ostream;
using std::filebuf;
using std::ios_base;

const wchar_t DEFAULT_CONFIG_FILENAME[] = L"/.xsim/xsimrc";

enum CONF_STAT_emu
{
	COMMON_CONF = 1,
	IM_CONF,
	PANEL_CONF,
	SP_CONF,
};

TWstring * Obj_Config::get_val(const wchar_t * name, TWstring & val, const wchar_t * default_val)
{
	for (size_t i = 0; i < value_name.size(); i++)
		if (value_name[i].casecompare(name) == 0){
			val.copy(value_value[i]);
			return NULL;
			}
	if (default_val){
		val.copy(default_val);
		return NULL;
		}

	TWstring * err_msg = new TWstring;
	err_msg->copy(L"Input Method Property: \"");
	err_msg->append(name);
	err_msg->append(L"\" not found");
	return err_msg;
}

TConf::TConf()
{
	conf_stat = COMMON_CONF;
}

TConf::~TConf()
{
}

void TConf::usage()
{
	cerr << endl << " Usage:xsim [-h][-c <config_file>]." << endl << endl;
}

int TConf::init(int argc, char * argv[])
{
	int argval;

	while((argval = getopt(argc, argv, "c:h")) != EOF)
		switch(argval){
			case 'c':
				config_path.copy(optarg);
				break;

			case 'h':
				usage();
				return 0;

			default:
				usage();
				break;
			}

	struct stat st;
	char dummy[MAX_PATH_LEN + 1];

	if (!config_path.empty()){
		if (stat(config_path.tombs(dummy, MAX_PATH_LEN), &st) != 0){
			cerr << " XSIM: failed opening config file: " << dummy << endl;
			return 0;
			}
		}else{
			char * home = getenv("HOME");
			config_path.copy(home);
			config_path.append(DEFAULT_CONFIG_FILENAME);
			if (stat(config_path.tombs(dummy, MAX_PATH_LEN), &st) != 0){
				config_path.copy(DEFAULT_CONFIG);
				if (stat(config_path.tombs(dummy, MAX_PATH_LEN), &st) != 0){
					cerr << " XSIM: failed opening config file: " << dummy << endl;
					return 0;
					}
				}
			}
	return init();
}

int TConf::init()
{
	im_count = 0;

        filebuf f;

	char dummy[MAX_PATH_LEN + 1];
#ifdef _CPP_BITS_IOSBASE_H
	if (!f.open(config_path.tombs(dummy, MAX_PATH_LEN), ios_base::in)){
#else
	if (!f.open(config_path.tombs(dummy, MAX_PATH_LEN), "r")){
#endif
		cerr << "XSIM: failed opening config file" << endl;
		return 0;
		}
	istream in(&f);

	TWstring value;
	TWstring name;
	TWstring buf;
	TWstring valbuf;
	size_t i;

	for (;;){
		dummy[0] = 0;
		in.getline(dummy, MAX_PATH_LEN);
		if (strlen(dummy) == 0)
			if (in.eof()) break;
				else continue;

		buf.copy(dummy);
		buf.ltrim();
		if (buf.empty())
			continue;
		if (!(i = scanname(buf, name)))
			continue;

		valbuf.copy(buf, buf.length() - i, i);
		valbuf.trim();
		if (valbuf.empty())
			return 0;

		if (!setvalue(name, valbuf))
			return 0;

		if (in.eof())
			break;
		}


	f.close();

	if (!im_count){
		cerr << "no imput mothed defined in config file" << endl;
		return 0;
		}

	return 1;
}

size_t TConf::scanname(TWstring & buf, TWstring & name)
{
	size_t j, len;
	len = buf.length();

	if (buf[0] == L'#')
		return 0;

	for (j = 0; j < len; j++)
		if ((buf[j] == L'\t') || (buf[j] == L' '))
			break;
	if (j == len)
		return 0;

	name.copy(buf, j);
	return j;
}

int TConf::scanvalue(TWstring & buf, TWstring & val)
{
	size_t i;

	if (buf[0] != L'"'){
		for (i = 0; i < buf.length(); i++)
			if ((buf[i] == L' ') || (buf[i] == L'\t'))
				break;

		val.copy(buf.data(), i);
		return 1;
		}

	for (i = 1; i < buf.length(); i++)
		if (buf[i] == L'"')
			break;

	if (i == buf.length())
		return 0;

	val.copy(buf.data(), i - 1, 1);

	return 1;
}

int TConf::setvalue(TWstring & name, TWstring & buf)
{
	TWstring tmp;
	if (!scanvalue(buf, tmp))
		return 0;

	if (name.casecompare(L"INPUT_METHOD") == 0){
		if (im_count == MAX_IM_COUNT){
			cerr << "too many input motheds defined, max is 10" << endl;
			return 0;
			}
		IM_conf[im_count].ident.copy(tmp);
		IM_conf[im_count].value_name.clear();
		IM_conf[im_count].value_value.clear();
		im_count++;
		conf_stat = IM_CONF;
		return 1;
		}

	if (name.casecompare(L"INPUT_PANEL") == 0){
		Panel_conf.ident.copy(tmp);
		Panel_conf.value_name.clear();
		Panel_conf.value_value.clear();
		conf_stat = PANEL_CONF;
		return 1;
		}

	if (name.casecompare(L"STATUS_PANEL") == 0){
		SP_conf.ident.copy(tmp);
		SP_conf.value_name.clear();
		SP_conf.value_value.clear();
		conf_stat = SP_CONF;
		return 1;
		}

	switch (conf_stat){
		case IM_CONF:
			IM_conf[im_count - 1].value_name.push_back(name);
			IM_conf[im_count - 1].value_value.push_back(tmp);
			break;
						
		case PANEL_CONF:
			Panel_conf.value_name.push_back(name);
			Panel_conf.value_value.push_back(tmp);
			break;

		case SP_CONF:
			SP_conf.value_name.push_back(name);
			SP_conf.value_value.push_back(tmp);
			break;

		case COMMON_CONF:
			value_name.push_back(name);
			value_value.push_back(tmp);
			break;
		}
	return 1;
}

TWstring * TConf::getpara(const wchar_t * name, const wchar_t * default_val)
{
	size_t i;
	for (i = 0; i < value_name.size(); i++)
		if (value_name[i].casecompare(name) == 0)
			return &value_value[i];
	if (!default_val){
		TWstring err_msg;
		
		err_msg.copy("failed get value of \"");
		err_msg.append(name);
		err_msg.append('"');
		throw err_msg;
		}
	TWstring v;
	v.copy(name);
	value_name.push_back(v);
	v.copy(default_val);
	value_value.push_back(v);
	return &value_value[i];
}

size_t TConf::get_im_count()
{
	return im_count;
}

Obj_Config * TConf::get_im_conf(size_t index)
{
	assert(index < im_count);

	return &IM_conf[index];
}

void TConf::save_config(const wchar_t * configname, vector<TWstring> & name, vector<TWstring> & value)
{
	TWstring path;
	char * home = getenv("HOME");
	char dummy[MAX_PATH_LEN + 1];

	path.copy(home);
	path.append(L"/.xsim");

	struct stat st;
	if (stat(path.tombs(dummy, MAX_PATH_LEN), &st) != 0)
		mkdir(dummy, 0x644);

	path.append('/');
	path.append(configname);

        filebuf f;

#ifdef _CPP_BITS_IOSBASE_H
	if (f.open(path.tombs(dummy, MAX_PATH_LEN), ios_base::out)){
#else
	if (f.open(path.tombs(dummy, MAX_PATH_LEN), "w")){
#endif
		ostream out(&f);
		for (size_t i = 0; i < name.size(); i++){
			out << name[i].tombs(dummy, MAX_PATH_LEN) << '\t';
			out << value[i].tombs(dummy, MAX_PATH_LEN) << endl;
			}
		f.close();
		}
}

int TConf::load_config(const wchar_t * configname, vector<TWstring> & name, vector<TWstring> & value)
{
	TWstring path;
	char * home = getenv("HOME");
	char dummy[MAX_PATH_LEN + 1];

	path.copy(home);
	path.append(L"/.xsim");

	struct stat st;
	if (stat(path.tombs(dummy, MAX_PATH_LEN), &st) != 0)
		return 0;

	path.append('/');
	path.append(configname);

        filebuf f;

#ifdef _CPP_BITS_IOSBASE_H
	if (f.open(path.tombs(dummy, MAX_PATH_LEN), ios_base::in)){
#else
	if (f.open(path.tombs(dummy, MAX_PATH_LEN), "r")){
#endif
		istream in(&f);
		char n[MAX_PATH_LEN + 1];
		char v[MAX_PATH_LEN + 1];
		TWstring wn, wv;
		for (;;){
			in >> n >> v;
			if ((strlen(n) == 0) || in.eof())
				break;
			wn.copy(n);
			wv.copy(v);
			name.push_back(wn);
			value.push_back(wv);
			}
		f.close();
		}else
			return 0;
	return 1;
}
