#ifndef _CONF_H
#define _CONF_H

#include <vector>
using std::vector;

#include "wstring.h"

// para for XIM
const wchar_t CP_BG_COLOR[] 		= L"BG_COLOR";
const wchar_t CP_FG_COLOR[] 		= L"FG_COLOR";
const wchar_t CP_STATUS_COLOR[]		= L"STATUS_COLOR";
const wchar_t CP_FONT_NAME[]		= L"FONT_NAME";
const wchar_t CP_XIM_NAME[]		= L"XIM_NAME";
const wchar_t CP_XIM_LOCALE[]		= L"XIM_LOCALE";
const wchar_t CP_DICT_LOCAL[]		= L"DICT_LOCAL";
const wchar_t CP_TRIGGER_KEY[]		= L"TRIGGER_KEY";
const wchar_t CP_SWITCH_KEY[]		= L"SWITCH_KEY";
const wchar_t CP_PLUGIN_LOCAL[]		= L"PLUGIN_LOCAL";

const wchar_t CP_ON_SPOT_ENABLED[]	= L"SWITCH_KEY";
const wchar_t CP_SHOW_STATUS_PANEL[]	= L"SHOW_STATUS_PANEL";

const size_t MAX_IM_COUNT = 10;

typedef struct Obj_Config_tag
{
	TWstring ident;
	vector<TWstring> value_name;
	vector<TWstring> value_value;
	TWstring * get_val(const wchar_t * name, TWstring & val, const wchar_t * default_val = NULL);
}Obj_Config;

class TConf
{
friend int main(int argc, char * argv[]);
private:
	TWstring config_path;

private:
	int init(int argc, char * argv[]);
	void usage();

	vector<TWstring> value_name;
	vector<TWstring> value_value;

	size_t im_count;
	Obj_Config IM_conf[MAX_IM_COUNT];
	Obj_Config SP_conf;
	Obj_Config Panel_conf;

	int conf_stat;

private:
	size_t scanname(TWstring & buf, TWstring & name);
	int scanvalue(TWstring & buf, TWstring & val);
	int setvalue(TWstring & name, TWstring & buf);

public:
	TConf();
	~TConf();

	int init();
	TWstring * getpara(const wchar_t * name, const wchar_t * default_val = NULL);

	size_t get_im_count();
	Obj_Config * get_im_conf(size_t index);
	Obj_Config * get_sp_conf() { return &SP_conf; }
	Obj_Config * get_panel_conf() { return &Panel_conf; }

	static void save_config(const wchar_t * configname, vector<TWstring> & name, vector<TWstring> & value);
	static int load_config(const wchar_t * configname, vector<TWstring> & name, vector<TWstring> & value);
};

#endif
