#include <assert.h>
#include <iostream>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "xim.h"
#include "im.h"
#include "panel.h"
#include "statuspanel.h"

using std::cerr;
using std::endl;

static XIMStyle Styles[] = {
	XIMPreeditPosition  | XIMStatusNothing,
//	XIMPreeditPosition  | XIMStatusArea,
//	XIMPreeditArea      | XIMStatusArea,
	XIMPreeditNothing   | XIMStatusNothing,
#ifdef ENABLE_ON_THE_SPOT
	XIMPreeditCallbacks | XIMStatusCallbacks,
#endif
	0
};

static XIMEncoding SUPPORT_ENCODEINGS[] = {
	"COMPOUND_TEXT",
	NULL
	};

extern TXIM * xim;

TXIM::TXIM()
{
	panel = NULL;
	spanel = NULL;
	ims = NULL;
}

TXIM::~TXIM()
{
	TIC * ic;

	ic = TIC::get_focus_ic();
	if (ic && ic->alive() && TWinMan::win_alive(ic->get_client_win())){
		IMTriggerNotifyStruct call_data;
		call_data.connect_id = ic->get_IC()->connect_id;
		call_data.icid = ic->get_IC()->id;
		IMPreeditEnd(ims, (XPointer)(&call_data));

		IMSyncXlibStruct pass_data;

		pass_data.major_code = XIM_SYNC;
		pass_data.minor_code = 0;
		pass_data.connect_id = ic->get_IC()->connect_id;
		pass_data.icid = ic->get_IC()->id;
		IMSyncXlib(ims, (XPointer)&pass_data);
		XSync(ims->core.display, False);
		}

	if (ims)
		IMCloseIM(ims);

	// we must clear all IC here, 'coz all IMs will be unloaded next
	// after then we have no way to delete IMCs
	ics.destory_all();
	
	for(size_t i = 0; i < MAX_IM_COUNT; i++)
		if (imset[i])
			TIM::destory(imset[i]);

	TPanel::destory(panel);
	TStatusPanel::destory(spanel);
	delete dummywin;
}

int TXIM::build_keylist(TConf * val, XIMTriggerKeys * keys)
{
	TWstring * s;
	try{
		s = val->getpara(CP_TRIGGER_KEY);
		if (!s){
			cerr << "XSIM: failed on creating trigger key" << endl;
			return 0;
			}
		TIM::phrase_key(s, &triggerkey[0]);

		s = val->getpara(CP_SWITCH_KEY);
		if (!s){
			cerr << "XSIM: failed on creating im switch key" << endl;
			return 0;
			}
		TIM::phrase_key(s, &triggerkey[1]);
		}catch (TWstring s){
			char dummy[256];
			s.tombs(dummy, 255);
		
			cerr << "XSIM: " << dummy << '"'<< endl;
			return 0;
			}

	keys->count_keys = 2;
	keys->keylist = triggerkey;

	return 1;
}

int TXIM::build_encodelist(XIMEncodings * encodings)
{
	// by now only COMPOUND_TEXT be supported :P
	encodings->count_encodings = sizeof(SUPPORT_ENCODEINGS)/sizeof(XIMEncoding) - 1;
	encodings->supported_encodings = SUPPORT_ENCODEINGS;

	return 1;
}

int TXIM::build_styles(XIMStyles * styles)
{
	styles->count_styles = sizeof(Styles)/sizeof(XIMStyle) - 1;
	styles->supported_styles = Styles;

	return 1;
}

int TXIM::run(TConf * val)
{
	conf = val;

	try{
		TWstring fg, bg;
		fg.copy(L"black");
		bg.copy(L"white");
		dummywin = new TWin;
		dummywin->create(0, 0, 1, 1, &fg, &bg);
		panel = TPanel::create(conf->getpara(CP_PLUGIN_LOCAL),
				conf->get_panel_conf());

		memset(imset, 0, sizeof(imset));

		for (size_t i = 0; i < conf->get_im_count(); i++){
			imset[i] = TIM::create(
					conf->getpara(CP_PLUGIN_LOCAL),
					conf->getpara(CP_DICT_LOCAL),
					conf->get_im_conf(i));
			if (!imset[i]){
				char dummy[256];
				conf->get_im_conf(i)->ident.tombs(dummy, 255);
				cerr << "XSIM: " << "failed creating Input Method: " << dummy << endl;
				return 0;
				}
			}

		TWstring * tmp = conf->getpara(CP_SHOW_STATUS_PANEL);
		if (tmp->casecompare(L"Yes") == 0){
			Status_Pack sp;
			sp.imc = NULL;
			sp.im_ident.copy(imset[0]->get_ident());
			sp.im_desc.copy(imset[0]->get_desc()->data());
			spanel = TStatusPanel::create(
				conf->getpara(CP_PLUGIN_LOCAL),
				conf->get_sp_conf(), &sp);
			}

		XIMTriggerKeys keys;
		XIMEncodings encodings;
		XIMStyles styles;

		if (!build_keylist(val, &keys))
			return 0;

		build_encodelist(&encodings);
		build_styles(&styles);

		char xim_name_buf[256];
		char xim_locale_buf[256];
		
		ims = IMOpenIM(TWinMan::get_display(),
			IMServerWindow,		dummywin->id(),
			IMModifiers,		"Xi18n",
			IMServerName,		conf->getpara(CP_XIM_NAME)->tombs(xim_name_buf, 255),
			IMLocale,		conf->getpara(CP_XIM_LOCALE)->tombs(xim_locale_buf, 255),
			IMServerTransport,	"X/",
			IMInputStyles,		&styles,
			IMProtocolHandler,	IMHandler,
			IMOnKeysList,		&keys,
			IMEncodingList,		&encodings,
			IMFilterEventMask,	KeyPressMask | KeyReleaseMask,
			NULL);

		}catch (TWstring s){
			char dummy[256];
			s.tombs(dummy, 255);
			cerr << "XSIM: " << dummy << endl;
			return 0;
			}
	if (ims)
	        cerr << "XSIM is started" << endl;
	        else{
			cerr << "IMOpenIM failed" << endl;
			return 0;
			}

	// enter window event loop,
	TWinMan::event_loop();

	return 1;
}

void TXIM::panel_update(TIC * ic, int update_status)
{
	panel->update(ic);
	if (update_status && spanel){
		Status_Pack sp;
		if (ic->has_focus() && ic->on() && ic->has_imc())
			sp.imc = ic->get_imc();
			else
				sp.imc = NULL;

		if (ic->has_focus() && ic->get_im()){
			sp.im_ident.copy(ic->get_im()->get_ident());
			sp.im_desc.copy(ic->get_im()->get_desc()->data());
			}else{
				sp.im_ident.copy(imset[0]->get_ident());
				sp.im_desc.copy(imset[0]->get_desc()->data());
				}

		spanel->update(&sp);
		}
}

int TXIM::open_handler(XIMS ims, IMOpenStruct *call_data)
{
#ifdef DEBUG
    	cerr << "XIM_OPEN" << endl;
#endif
	return 1;
}

int TXIM::close_handler(XIMS ims, IMCloseStruct *call_data)
{
#ifdef DEBUG
	cerr << "XIM_CLOSE" << endl;
#endif
	return 1;
}

int TXIM::create_ic_handler(XIMS ims, IMChangeICStruct *call_data)
{
#ifdef DEBUG
	cerr << "XIM_CREATE_IC" << endl;
#endif
	ics.newic(call_data, imset[0]);
	return 1;
}

int TXIM::destroy_ic_handler(XIMS ims, IMDestroyICStruct *call_data)
{
#ifdef DEBUG
	cerr << "XIM_DESTORY_IC" << endl;
#endif
	TIC * ic = ics.deleteic(call_data);
	assert(ic);

	panel_update(ic, 1);

	return 1;
}

int TXIM::set_focus_handler(XIMS ims, IMChangeFocusStruct *call_data)
{
#ifdef DEBUG
	cerr << "XIM_SET_IC_FOCUS" << endl;
#endif
	TIC * ic = ics.getic(call_data->icid);
	assert(ic);

	ic->set_focus();
	panel_update(ic, 1);

	return 1;
}

int TXIM::reset_focus_handler(XIMS ims, IMChangeFocusStruct *call_data)
{
#ifdef DEBUG
	cerr << "XIM_RESET_IC_FOCUS" << endl;
#endif
	TIC * ic = ics.getic(call_data->icid);
	assert(ic);

	if (!ic->has_focus()){
		ic->set_focus();
		panel_update(ic, 1);
		}

	return 1;
}

int TXIM::unset_focus_handler(XIMS ims, IMChangeFocusStruct *call_data)
{
#ifdef DEBUG
	cerr << "XIM_UNSET_IC_FOCUS" << endl;
#endif
	TIC * ic = ics.getic(call_data->icid);
	assert(ic);

	if (ic->has_focus()){
		ic->unset_focus();
		panel_update(ic, 1);
		}


	return 1;
}

int TXIM::trigger_handler(XIMS ims, IMTriggerNotifyStruct *call_data)
{
#ifdef DEBUG
	cerr << "XIM_TRIGGER_NOTIFY" << endl;
#endif
	TIC * ic;

	if (call_data->flag == 0){
		ic = ics.getic(call_data->icid);
		if (call_data->key_index/3 == 1){
			TIM * im;
			//IMPreeditEnd(ims, (XPointer) call_data);
			if ((im = ic->get_im()) == NULL)
				im = imset[0];
				else
					im = next_im(im);
			ic->init(IC_RESET, im);
			}
		ic->turn_on();
		ic->set_focus();
		panel_update(ic, 1);
		return 1;
		}
	return 0;
}

#ifdef ENABLE_ON_THE_SPOT

void TXIM::on_the_spot_start(TIC * ic, CARD16 connect_id)
{
        IMGeometryCBStruct gs;

        gs.major_code = XIM_GEOMETRY;
        gs.connect_id = connect_id;

        IMCallCallback(ims, (XPointer)&gs);


	IMPreeditCBStruct ps;

	ps.major_code = XIM_PREEDIT_START;
	ps.connect_id = connect_id;
	ps.todo.return_value = 0;

	IMCallCallback(ims, (XPointer)&ps);
cerr << "ps.todo.return_value = " << ps.todo.return_value << endl;

}

void TXIM::on_the_spot_draw(TIC * ic, CARD16 connect_id)
{
	IMPreeditCBStruct ps;
	XIMPreeditDrawCallbackStruct * ds = &ps.todo.draw;
	XIMText xt;

	ps.major_code = XIM_PREEDIT_DRAW;
	ps.connect_id = connect_id;

	ds->caret = 0;
	ds->chg_first = 0;
	ds->chg_length = 0;
	ds->text = &xt;

	xt.length = 2;
	xt.feedback = NULL;
	xt.encoding_is_wchar = 0;
	xt.string.multi_byte = "≤‚ ‘";

	IMCallCallback(ims, (XPointer)&ps);
}

void TXIM::on_the_spot_stop(TIC * ic, CARD16 connect_id)
{
	IMPreeditCBStruct ps;

	ps.major_code = XIM_PREEDIT_DONE;
	ps.connect_id = connect_id;

	IMCallCallback(ims, (XPointer)&ps);
}

#endif

TIM * TXIM::next_im(TIM * im)
{
	size_t i;
	for (i = 0; i < MAX_IM_COUNT; i++)
		if (imset[i] == im){
			i++;
			break;
			}
	if ((i == MAX_IM_COUNT) || (imset[i] == NULL))
		return imset[0];
	return imset[i];
}

int TXIM::forward_handler(XIMS ims, IMForwardEventStruct *call_data)
{
#ifdef DEBUG
	cerr << "XIM_FORWARD_EVENT" << endl;
#endif
//	static int doing_switch_im = 0;
	XKeyEvent * e;
	TIC * ic;
	TIM * im;

	e = (XKeyEvent *)&(call_data->event);
	ic = ics.getic(call_data->icid);

	if (TIM::iskey(e, &triggerkey[0]))
		if (e->type == KeyPress){
			IMPreeditEnd(ims, (XPointer) call_data);
			ic->turn_off();
			panel_update(ic, 1);
			return 1;
			}

	assert(ic);

	ic->set_focus();
	im = ic->get_im();

#if 0
	if (TIM::iskey(e, &triggerkey[1])){
		if (e->type == KeyPress){
			doing_switch_im = 1;
			return 1;
			}else if (doing_switch_im){
				doing_switch_im = 0;
				ic->init(IC_RESET, next_im(im));
				ic->turn_on();
				panel_update(ic, 1);
				return 1;
				}
		}else
			doing_switch_im = 0;
#endif

	if (TIM::iskey(e, &triggerkey[1]))
		if (e->type == KeyPress){
			ic->init(IC_RESET, next_im(im));
			ic->turn_on();
			panel_update(ic, 1);
			return 1;
			}

#ifdef ENABLE_ON_THE_SPOT
	int is_on_the_spot;
	is_on_the_spot = ic->get_IC()->input_style & XIMPreeditCallbacks;
#endif
        if ((e->type == KeyRelease) && !(im->needed_release_event()))
		return 1;
	
	switch (im->processinput(ic->get_imc(), e)){
		case FORWARD_KEY:
			IMForwardEvent(ims, (XPointer)call_data);
			break;

		case SWITCH_IM_KEY:{
			ic->init(IC_CLEAR, NULL);

			size_t i;
			for (i = 0; i < MAX_IM_COUNT; i ++)
				if (imset[i] == ic->get_im())
					break;
			i ++;
			if ((i == MAX_IM_COUNT) || (imset[i] == 0))
				i = 0;

			ic->set_im(imset[i]);

#ifdef ENABLE_ON_THE_SPOT
			if (is_on_the_spot)
				on_the_spot_stop(ic, call_data->connect_id);
				else
#endif
					panel_update(ic, 1);

			break;
			}

		case STAT_KEY:
			panel_update(ic, 1);
			break;

		case LISTFORWARD_KEY:
			if (!panel->display_count_calc(CALC_FORWARD, ic))
				break;

#ifdef ENABLE_ON_THE_SPOT
			if (is_on_the_spot)
				on_the_spot_draw(ic, call_data->connect_id);
				else
#endif
					panel_update(ic, 0);
			break;

		case LISTBACKWARD_KEY:
			if (!panel->display_count_calc(CALC_BACKWARD, ic))
				break;

#ifdef ENABLE_ON_THE_SPOT
			if (is_on_the_spot)
				on_the_spot_draw(ic, call_data->connect_id);
				else
#endif
					panel_update(ic, 0);
			break;

		case COMMIT_KEY:{
			Display *display;
			XTextProperty tp;
#ifdef MDK_PATCH
			char * l;
			char dummy[256];

                        display = ims->core.display;
			l = im->get_commit()->tombs(dummy, 255);
			XmbTextListToTextProperty(display, &l, 1, XCompoundTextStyle, &tp);
#else
			wchar_t * ret;

                        display = ims->core.display;
			ret = im->get_commit()->data();
			XwcTextListToTextProperty(display, &ret, 1, XCompoundTextStyle, &tp);
#endif
			((IMCommitStruct*)call_data)->flag |= XimLookupChars;
			((IMCommitStruct*)call_data)->commit_string = (char *)tp.value;
			IMCommitString(ims, (XPointer)call_data);
			XFree(tp.value);
			}

		case HIDE_KEY:
#ifdef ENABLE_ON_THE_SPOT
			if (is_on_the_spot)
				on_the_spot_stop(ic, call_data->connect_id);
				else
#endif
					panel_update(ic, 0);
			break;

		case NORMAL_KEY:
			if (!panel->display_count_calc(CALC_INIT, ic))
				assert(0);		// it could not be true

#ifdef ENABLE_ON_THE_SPOT
			if (is_on_the_spot)
				on_the_spot_draw(ic, call_data->connect_id);
				else
#endif
					panel_update(ic, 0);
			break;

		case IGNORE_KEY:
			break;
		}

	return 1;
}

int TXIM::set_ic_values_handler(XIMS ims, IMChangeICStruct *call_data)
{
#ifdef DEBUG
	cerr << "XIM_SET_IC_VALUES" << endl;
#endif
	TIC * focus_ic = TIC::get_focus_ic();
	int ret = ics.seticvalue(call_data);

	TIC * ic = ics.getic(call_data->icid);
	assert(ic);

	if (ic->has_focus())
		if (focus_ic == ic)
			panel_update(ic, 0);
			else
				// for mozilla never give SET_IC_FOCUS,
				// so need check focus changed and update status
				panel_update(ic, 1);

	return ret;
}

int TXIM::get_ic_values_handler(XIMS ims, IMChangeICStruct *call_data)
{
#ifdef DEBUG
	cerr << "XIM_GET_IC_VALUES" << endl;
#endif
	return ics.geticvalue(call_data);
}

int TXIM::sync_reply_handler(XIMS ims, IMSyncXlibStruct *call_data)
{
#ifdef DEBUG
	cerr << "XIM_SYNC_REPLY" << endl;
#endif
	return 1;
}

int TXIM::IMHandler(XIMS ims, IMProtocol * call_data)
{
	int ret;

	switch (call_data->major_code) {
		case XIM_OPEN:
			ret = xim->open_handler(ims, &(call_data->imopen));
			break;
		case XIM_CLOSE:
			ret = xim->close_handler(ims, &(call_data->imclose));
			break;
		case XIM_CREATE_IC:
			ret = xim->create_ic_handler(ims, &(call_data->changeic));
			break;
		case XIM_DESTROY_IC:
			ret = xim->destroy_ic_handler(ims, &(call_data->destroyic));
			break;
		case XIM_SET_IC_FOCUS:
			ret = xim->set_focus_handler(ims, &(call_data->changefocus));
			break;
		case XIM_UNSET_IC_FOCUS:
			ret = xim->unset_focus_handler(ims, &(call_data->changefocus));
			break;
		case XIM_TRIGGER_NOTIFY:
			ret = xim->trigger_handler(ims, &(call_data->triggernotify));
			break;
		case XIM_FORWARD_EVENT:
			ret = xim->forward_handler(ims, &(call_data->forwardevent));
			break;
		case XIM_SET_IC_VALUES:
			ret = xim->set_ic_values_handler(ims, &(call_data->changeic));
			break;
		case XIM_GET_IC_VALUES:
			ret = xim->get_ic_values_handler(ims, &(call_data->changeic));
			break;
		case XIM_SYNC_REPLY:
			ret = xim->sync_reply_handler(ims, &(call_data->sync_xlib));
			break;
		case XIM_RESET_IC:
			ret = xim->reset_focus_handler(ims, &(call_data->changefocus));
			break;

		//  Not implement yet.
		case XIM_PREEDIT_START_REPLY:
#ifdef DEBUG
			cerr << "XIM_PREEDIT_START_REPLY" << endl;
#endif
			ret = 1;
			break;
		case XIM_PREEDIT_CARET_REPLY:
#ifdef DEBUG
			cerr << "XIM_PREEDIT_CARET_REPLY" << endl;
#endif
			ret = 1;
			break;
		default:
#ifdef DEBUG
			cerr << "XIM Unknown" << endl;
#endif
			ret = 0;
			break;
		}
		
	return  1;
}
