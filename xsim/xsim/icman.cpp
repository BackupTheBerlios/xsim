#include <assert.h>
#include <iostream>

#include "icman.h"

using std::cerr;
using std::endl;

static int Is(char *attr, XICAttribute *attr_list)
{
	return !strcmp(attr, attr_list->name);
}

TICManager::TICManager()
{
	ics = NULL;
	freeics = NULL;

	icid_ = 0;
}

TICManager::~TICManager()
{
	destory_all();
}

void TICManager::destory_all()
{
	TIC * ic = ics;

	while (ic){
		ics = ic->next;
		delete ic;
		ic = ics;
		}

	ic = freeics;
	while (ic){
		freeics = ic->next;
		delete ic;
		ic = freeics;
		}
	ics = NULL;
	freeics = NULL;
}

TIC * TICManager::getic(CARD16 icid)
{
	TIC * ic = ics;
	while (ic){
		if (ic->ic_.id == icid)
			return ic;
		ic = ic->next;
		}
	return NULL;
}

int TICManager::saveic(TIC * ic, IMChangeICStruct *call_data)
{
	XICAttribute *ic_attr = call_data->ic_attr;
	XICAttribute *pre_attr = call_data->preedit_attr;
	XICAttribute *sts_attr = call_data->status_attr;
	register int i;

	IC * ic_ = &ic->ic_;
	ic_->connect_id = call_data->connect_id;

	for (i = 0; i < (int)call_data->ic_attr_num; i++, ic_attr++){
		if (Is (XNInputStyle, ic_attr)){
#ifdef DEBUG
			cerr << "ic_attr: name = " << ic_attr->name << " value = " << *(INT32*)ic_attr->value << endl;
#endif
			ic_->input_style = *(INT32*)ic_attr->value;
			}else if (Is (XNClientWindow, ic_attr)){
#ifdef DEBUG
				cerr << "ic_attr: name = " << ic_attr->name << " value = " << *(Window*)ic_attr->value << endl;
#endif
				ic_->client_win = *(Window*)ic_attr->value;
			}else if (Is (XNFocusWindow, ic_attr)){
#ifdef DEBUG
				cerr << "ic_attr: name = " << ic_attr->name << " value = " << *(Window*)ic_attr->value << endl;
#endif
				ic_->focus_win = *(Window*)ic_attr->value;
				// for Mozilla never give me SET_IC_FOCUS, so need do it myself
				ic->set_focus();
			}else if (ic_attr->name)
				cerr << "Set unknown ic_attr: " << ic_attr->name << endl;
		}

	for (i = 0; i < (int)call_data->preedit_attr_num; i++, pre_attr++){
		if (Is (XNArea, pre_attr)){
			ic_->pre_attr.area = *(XRectangle*)pre_attr->value;
#ifdef DEBUG
			cerr << "pre_attr: name = " << pre_attr->name
				<< " x = " << ic_->pre_attr.area.x << " y = " << ic_->pre_attr.area.y
				<< " width = " << ic_->pre_attr.area.width << " height = " << ic_->pre_attr.area.height << endl;
#endif
			}else if (Is (XNAreaNeeded, pre_attr)){
#ifdef DEBUG
				cerr << "pre_attr: name = " << pre_attr->name << endl;
#endif
				ic_->pre_attr.area_needed = *(XRectangle*)pre_attr->value;
			}else if (Is (XNSpotLocation, pre_attr)){
				ic_->pre_attr.spot_location = *(XPoint*)pre_attr->value;
#ifdef DEBUG
				cerr << "pre_attr: name = " << pre_attr->name
					<< " x = " << ic_->pre_attr.spot_location.x << " y = " << ic_->pre_attr.spot_location.y << endl;
#endif
			}else if (Is (XNColormap, pre_attr)){
#ifdef DEBUG
				cerr << "pre_attr: name = " << pre_attr->name << endl;
#endif
				ic_->pre_attr.cmap = *(Colormap*)pre_attr->value;
			}else if (Is (XNStdColormap, pre_attr)){
#ifdef DEBUG
				cerr << "pre_attr: name = " << pre_attr->name << endl;
#endif
				ic_->pre_attr.cmap = *(Colormap*)pre_attr->value;
			}else if (Is (XNForeground, pre_attr)){
#ifdef DEBUG
				cerr << "pre_attr: name = " << pre_attr->name << endl;
#endif
				ic_->pre_attr.foreground = *(CARD32*)pre_attr->value;
			}else if (Is (XNBackground, pre_attr)){
#ifdef DEBUG
				cerr << "pre_attr: name = " << pre_attr->name << endl;
#endif
				ic_->pre_attr.background = *(CARD32*)pre_attr->value;
			}else if (Is (XNBackgroundPixmap, pre_attr)){
#ifdef DEBUG
				cerr << "pre_attr: name = " << pre_attr->name << endl;
#endif
				ic_->pre_attr.bg_pixmap = *(Pixmap*)pre_attr->value;
			}else if (Is (XNFontSet, pre_attr)){
#ifdef DEBUG
				cerr << "pre_attr: name = " << pre_attr->name << endl;
#endif
				ic_->pre_attr.base_font.copy((char *)pre_attr->value);
			}else if (Is (XNLineSpace, pre_attr)){
#ifdef DEBUG
				cerr << "pre_attr: name = " << pre_attr->name << endl;
#endif
				ic_->pre_attr.line_space = *(CARD32*)pre_attr->value;
			}else if (Is (XNCursor, pre_attr)){
#ifdef DEBUG
				cerr << "pre_attr: name = " << pre_attr->name << endl;
#endif
				ic_->pre_attr.cursor = *(Cursor*)pre_attr->value;
			}else if (pre_attr->name)
				cerr << "Set unknown pre_attr: " << pre_attr->name << endl;
		}

	for (i = 0; i < (int)call_data->status_attr_num; i++, sts_attr++){
		if (Is (XNArea, sts_attr)){
#ifdef DEBUG
			cerr << "sts_attr: name = " << sts_attr->name << endl;
#endif
			ic_->sts_attr.area = *(XRectangle*)sts_attr->value;
			}else if (Is (XNAreaNeeded, sts_attr)){
#ifdef DEBUG
				cerr << "sts_attr: name = " << sts_attr->name << endl;
#endif
				ic_->sts_attr.area_needed = *(XRectangle*)sts_attr->value;
			}else if (Is (XNColormap, sts_attr)){
#ifdef DEBUG
				cerr << "sts_attr: name = " << sts_attr->name << endl;
#endif
				ic_->sts_attr.cmap = *(Colormap*)sts_attr->value;
			}else if (Is (XNStdColormap, sts_attr)){
#ifdef DEBUG
				cerr << "sts_attr: name = " << sts_attr->name << endl;
#endif
				ic_->sts_attr.cmap = *(Colormap*)sts_attr->value;
			}else if (Is (XNForeground, sts_attr)){
#ifdef DEBUG
				cerr << "sts_attr: name = " << sts_attr->name << endl;
#endif
				ic_->sts_attr.foreground = *(CARD32*)sts_attr->value;
			}else if (Is (XNBackground, sts_attr)){
#ifdef DEBUG
				cerr << "sts_attr: name = " << sts_attr->name << endl;
#endif
				ic_->sts_attr.background = *(CARD32*)sts_attr->value;
			}else if (Is (XNBackgroundPixmap, sts_attr)){
#ifdef DEBUG
				cerr << "sts_attr: name = " << sts_attr->name << endl;
#endif
				ic_->sts_attr.bg_pixmap = *(Pixmap*)sts_attr->value;
			}else if (Is (XNFontSet, sts_attr)){
#ifdef DEBUG
				cerr << "sts_attr: name = " << sts_attr->name << endl;
#endif
				ic_->sts_attr.base_font.copy((char *)sts_attr->value);
			}else if (Is (XNLineSpace, sts_attr)){
#ifdef DEBUG
				cerr << "sts_attr: name = " << sts_attr->name << endl;
#endif
				ic_->sts_attr.line_space= *(CARD32*)sts_attr->value;
			}else if (Is (XNCursor, sts_attr)){
#ifdef DEBUG
				cerr << "sts_attr: name = " << sts_attr->name << endl;
#endif
				ic_->sts_attr.cursor = *(Cursor*)sts_attr->value;
			}else if (sts_attr->name)
				cerr << "Set unknown sts_attr: " << sts_attr->name << endl;
		}

	return 1;
}

int TICManager::loadic(TIC * ic, IMChangeICStruct *call_data)
{
	XICAttribute *ic_attr = call_data->ic_attr;
	XICAttribute *pre_attr = call_data->preedit_attr;
	XICAttribute *sts_attr = call_data->status_attr;
	register int i;
	char dummy[256];

	IC * ic_ = &ic->ic_;

	for (i = 0; i < (int)call_data->ic_attr_num; i++, ic_attr++){
#ifdef DEBUG
		cerr << "ic_attr: name = " << ic_attr->name << endl;
#endif
		if (Is (XNFilterEvents, ic_attr)) {
			ic_attr->value = (void *)malloc(sizeof(CARD32));
			*(CARD32*)ic_attr->value = KeyPressMask|KeyReleaseMask;
			ic_attr->value_length = sizeof(CARD32);
			}
			else if (Is (XNInputStyle, ic_attr)) {
				ic_attr->value = (void *)malloc(sizeof(INT32));
				ic_attr->value_length = sizeof(INT32);
				*(INT32*)ic_attr->value = ic_->input_style;
			}else if (Is (XNSeparatorofNestedList, ic_attr)){
				ic_attr->value = (void *)malloc(sizeof(CARD16));
				ic_attr->value_length = sizeof(CARD16);
				*(CARD16*)ic_attr->value = 0;
			}else{
				cerr << "Get unknown attr: " << ic_attr->name << endl;
				ic_attr->value = NULL;
				}
		}

	/* preedit attributes */
	for (i = 0; i < (int)call_data->preedit_attr_num; i++, pre_attr++)
		if (Is (XNArea, pre_attr)) {
			pre_attr->value = (void *)malloc(sizeof(XRectangle));
			*(XRectangle*)pre_attr->value = ic_->pre_attr.area;
			pre_attr->value_length = sizeof(XRectangle);

			} else if (Is (XNAreaNeeded, pre_attr)) {
				pre_attr->value = (void *)malloc(sizeof(XRectangle));
				*(XRectangle*)pre_attr->value = ic_->pre_attr.area_needed;
				pre_attr->value_length = sizeof(XRectangle);

			} else if (Is (XNSpotLocation, pre_attr)) {
				pre_attr->value = (void *)malloc(sizeof(XPoint));
				*(XPoint*)pre_attr->value = ic_->pre_attr.spot_location;
				pre_attr->value_length = sizeof(XPoint);

			} else if (Is (XNFontSet, pre_attr)) {
				ic_->pre_attr.base_font.tombs(dummy, 255);
				CARD16 base_len = (CARD16)(strlen(dummy));
				int total_len = sizeof(CARD16) + (CARD16)base_len;
				char *p;

				pre_attr->value = (void *)malloc(total_len);
				p = (char *)pre_attr->value;
				memmove(p, &base_len, sizeof(CARD16));
				p += sizeof(CARD16);
				strncpy(p, dummy, base_len);
				pre_attr->value_length = total_len;

			} else if (Is (XNForeground, pre_attr)) {
				pre_attr->value = (void *)malloc(sizeof(long));
				*(long*)pre_attr->value = ic_->pre_attr.foreground;
				pre_attr->value_length = sizeof(long);

			} else if (Is (XNBackground, pre_attr)) {
				pre_attr->value = (void *)malloc(sizeof(long));
				*(long*)pre_attr->value = ic_->pre_attr.background;
				pre_attr->value_length = sizeof(long);

			} else if (Is (XNLineSpace, pre_attr)) {
				pre_attr->value = (void *)malloc(sizeof(long));
#if 0
				*(long*)pre_attr->value = ic_->pre_attr.line_space;
#endif
				*(long*)pre_attr->value = 18;
				pre_attr->value_length = sizeof(long);
			} else {
				cerr << "Get unknown pre_attr: " << pre_attr->name << endl;
				pre_attr->value = NULL;
				}

	/* status attributes */
	for (i = 0; i < (int)call_data->status_attr_num; i++, sts_attr++)
		if (Is (XNArea, sts_attr)) {
			sts_attr->value = (void *)malloc(sizeof(XRectangle));
			*(XRectangle*)sts_attr->value = ic_->sts_attr.area;
			sts_attr->value_length = sizeof(XRectangle);

			} else if (Is (XNAreaNeeded, sts_attr)) {
				sts_attr->value = (void *)malloc(sizeof(XRectangle));
				*(XRectangle*)sts_attr->value = ic_->sts_attr.area_needed;
				sts_attr->value_length = sizeof(XRectangle);

			} else if (Is (XNFontSet, sts_attr)) {
				ic_->sts_attr.base_font.tombs(dummy, 255);
				CARD16 base_len = (CARD16)(strlen(dummy));
				int total_len = sizeof(CARD16) + (CARD16)base_len;
				char *p;

				sts_attr->value = (void *)malloc(total_len);
				p = (char *)sts_attr->value;
				memmove(p, &base_len, sizeof(CARD16));
				p += sizeof(CARD16);
				strncpy(p, dummy, base_len);
				sts_attr->value_length = total_len;

			} else if (Is (XNForeground, sts_attr)) {
				sts_attr->value = (void *)malloc(sizeof(long));
				*(long*)sts_attr->value = ic_->sts_attr.foreground;
				sts_attr->value_length = sizeof(long);

			} else if (Is (XNBackground, sts_attr)) {
				sts_attr->value = (void *)malloc(sizeof(long));
				*(long*)sts_attr->value = ic_->sts_attr.background;
				sts_attr->value_length = sizeof(long);

			} else if (Is (XNLineSpace, sts_attr)) {
				sts_attr->value = (void *)malloc(sizeof(long));
#if 0
				*(long*)sts_attr->value = ic_->sts_attr.line_space;
#endif
				*(long*)sts_attr->value = 18;
				sts_attr->value_length = sizeof(long);
			} else {
				cerr << "Get unknown sts_attr: " << sts_attr->name << endl;
				sts_attr->value = NULL;
				}
	return 1;
}

TIC * TICManager::newic(IMChangeICStruct *call_data, TIM * val)
{
	TIC * ic;

	if (freeics){
		ic = freeics;
		freeics = freeics->next;
		ic->init(IC_RESET, val);
		}else
			ic = new TIC(val);

	ic->next = ics;
	ics = ic;
	ic->ic_.id = ++icid_;

	saveic(ic, call_data);
	call_data->icid = ic->ic_.id;

	return ic;
}

TIC * TICManager::deleteic(IMDestroyICStruct *call_data)
{
	TIC *ic, *prev;

	prev = NULL;

	ic = ics;
	while (ic){
		if (ic->ic_.id == call_data->icid){
			if (prev)
				prev->next = ic->next;
				else
					ics = ic->next;

			ic->next = freeics;
			freeics = ic;
			ic->init(IC_ERASE, NULL);
			return ic;
			}

		prev = ic;
		ic = ic->next;
		}
	
	assert(0);
	return NULL;
}

int TICManager::seticvalue(IMChangeICStruct *call_data)
{
	TIC * ic = getic(call_data->icid);
	assert(ic);

	return saveic(ic, call_data);
}

int TICManager::geticvalue(IMChangeICStruct *call_data)
{
	TIC * ic = getic(call_data->icid);
	assert(ic);

	return loadic(ic, call_data);
}
