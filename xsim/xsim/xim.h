#ifndef _XIM_H
#define _XIM_H

#include <vector>
#include <X11/Xlib.h>

#include "IMdkit/IMdkit.h"
#include "IMdkit/Xi18n.h"

#include "icman.h"
#include "conf.h"
#include "win.h"

class TPanel;
class TStatusPanel;
class TFontSet;
class TGC;
class TIM;
class TIC;
class TConf;

class TXIM
{
private:
	XIMS ims;
	XIMTriggerKey triggerkey[2];		// on key, and switch key

	TICManager ics;
	TIM * imset[MAX_IM_COUNT];
	TConf * conf;

	TWinMan winman;
	TWin * dummywin;
	TPanel * panel;
	TStatusPanel * spanel;

private:
	static int IMHandler(XIMS ims, IMProtocol * call_data);

	int build_keylist(TConf * val, XIMTriggerKeys * keys);
	int build_encodelist(XIMEncodings * encodings);
	int build_styles(XIMStyles * styles);

	int open_handler(XIMS ims, IMOpenStruct *call_data);
	int close_handler(XIMS ims, IMCloseStruct *call_data);
	int create_ic_handler(XIMS ims, IMChangeICStruct *call_data);
	int destroy_ic_handler(XIMS ims, IMDestroyICStruct *call_data);
	int set_focus_handler(XIMS ims, IMChangeFocusStruct *call_data);
	int reset_focus_handler(XIMS ims, IMChangeFocusStruct *call_data);
	int unset_focus_handler(XIMS ims, IMChangeFocusStruct *call_data);
	int trigger_handler(XIMS ims, IMTriggerNotifyStruct *call_data);
	int forward_handler(XIMS ims, IMForwardEventStruct *call_data);
	int set_ic_values_handler(XIMS ims, IMChangeICStruct *call_data);
	int get_ic_values_handler(XIMS ims, IMChangeICStruct *call_data);
	int sync_reply_handler(XIMS ims, IMSyncXlibStruct *call_data);

#ifdef ENABLE_ON_THE_SPOT
	// not finished
	// for on the spot input style
	void on_the_spot_start(TIC * ic, CARD16 connect_id);
	void on_the_spot_draw(TIC * ic, CARD16 connect_id);
	void on_the_spot_stop(TIC * ic, CARD16 connect_id);
#endif

	void panel_update(TIC * ic, int update_status);
	TIM * next_im(TIM * im);
public:
	TXIM();
	~TXIM();

	int run(TConf * val);
};

#endif
