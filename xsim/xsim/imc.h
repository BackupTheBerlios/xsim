#ifndef _IMC_H
#define _IMC_H

#include <inttypes.h>

#include "wstring.h"

class TIMC					// IM context
{
private:
	int display_size;
public:
	virtual ~TIMC(){};

	// clear all input data
	virtual void clear() = 0;
	
	// the input string that display on the input panel
	virtual TWstring * display_input() = 0;

	// words count with the input data
	virtual uint16_t list_count() = 0;

	// get caret postion, return -1 if no caret
	virtual int caret_pos(){ return -1; }

	// the list string,pass NULL to suffix if u dont need
	virtual TWstring * list_str(uint16_t index, TWstring * suffix) = 0;

	// set the first displayed string index, and the displayed string count
	virtual void setdisplay(uint16_t index, uint16_t count) = 0;
	
	// get displayed string count
	virtual uint16_t display_count() = 0;

	// get the first displayed string
	virtual uint16_t display_index() = 0;

	// displayed string, pass NULL to suffix if u dont need
	virtual TWstring * display_str(uint16_t index, TWstring * suffix)
		{ return list_str(display_index() + index, suffix); }

	// used by input panel, to set the panel size	
	virtual void set_display_size(int w) { display_size = w; }
	virtual int get_display_size() { return display_size; }

	// is the IM enabled
	virtual int enabled() { return 1;}

	// is mulitbyte input mode
	virtual int is_mb_input() { return 0; }

	// is preediting, if so, show input panel
	virtual int is_preedit() { return 0; }
};

#endif
