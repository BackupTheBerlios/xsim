#include <X11/Xlib.h>
#include "IMdkit/IMdkit.h"
#include "IMdkit/Xi18n.h"

#include "im.h"
#include "ic.h"
#include "imc.h"

TIC * TIC::focused_ic = NULL;

TIC::TIC(TIM * val)
{
	imc = NULL;

// this line does not worked, so i must add lines below, why??
//	init(IC_RESET);

	memset(&ic_, 0, sizeof(IC));
	im = val;
	on_stat = 0;
}


void TIC::init(int init_mode, TIM * val)
{
	switch (init_mode){
		case IC_CLEAR:
			if (!imc)
				get_imc();
			imc->clear();
			break;

		case IC_RESET:
			if (im == val)
				return;
			if (imc)
				delete imc;
			imc = NULL;
			im = val;
			on_stat = 0;
			break;

		case IC_ERASE:
			memset(&ic_, 0, sizeof(IC));
			if (imc)
				delete imc;
			im = NULL;
			imc = NULL;
			on_stat = 0;
			unset_focus();
			break;
		}
}

TIC::~TIC()
{
	if (imc)
		delete imc;
}

TIMC * TIC::get_imc()
{
	if (!imc && im)
		imc = im->create_imc();

	return imc;
}

void TIC::set_im(TIM * val)
{
	init(IC_RESET, val);
}

void TIC::turn_on()
{
	init(IC_CLEAR, NULL);
	on_stat = 1;
}

void TIC::turn_off()
{
	init(IC_CLEAR, NULL);
	on_stat = 0;
}

int TIC::alive()
{
	return (int)im;
}
