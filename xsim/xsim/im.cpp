#include <dlfcn.h>

#include "im.h"

const wchar_t CONTROL_MASK_NAME[] = L"ctrl";
const wchar_t ALT_MASK_NAME[] = L"alt";
const wchar_t SHIFT_MASK_NAME[] = L"shift";

//ANSI C++ forbids implicit conversion from `void *' in assignment :(
typedef void * DLHANDLE;

typedef TIM * (* ClassCreatorProc)();

TIM * TIM::create(TWstring * plugins_path, TWstring * dict_path, Obj_Config * im_conf)
{                                                           
	TWstring path;
	
	path.copy(plugins_path->data());
	path.append('/');
	path.append(im_conf->ident);
	path.append(L".so");
	
	char dummy[256];
	DLHANDLE handle;
	const char * error;
	
	if ((handle = (DLHANDLE) dlopen(path.tombs(dummy, 255), RTLD_LAZY)) == NULL){
		TWstring err_msg;
		error = dlerror();
		err_msg.copy("Failed opening share library: ");
		err_msg.append(path);
		err_msg.append('\n');
		err_msg.append(error);
		throw err_msg;
		}

	ClassCreatorProc ClassCreator;
	ClassCreator = (ClassCreatorProc) dlsym(handle, "ClassCreator");
	if ((error = dlerror()) != NULL){
		TWstring err_msg;
		err_msg.copy("Failed opening share library function: ");
		err_msg.append(error);
		throw err_msg;
		}

	TIM * im = ClassCreator();

	if (!im)
		return NULL;
        
        TWstring * err;
	if ((err = im->create(dict_path, im_conf)) != NULL){
		delete im;
		TWstring err_msg;
		err_msg.copy(err->data());
		delete err;
		throw err_msg;
		}
	
	im->dlhandle = handle;

	return im;
}

void TIM::destory(TIM * im)
{       
	DLHANDLE handle = im->dlhandle;
	
	delete im;
	
	dlclose(handle);
}

TWstring * TIM::phrase_key(TWstring * str, XIMTriggerKey * keyret)
{
	assert(str);

	CARD32 keysym;
	char dummy[256];
	int pos1 = 0, pos2;
	size_t len = str->length();
	TWstring tmp;


	keyret->modifier = 0;
	keyret->modifier_mask = 0;
	str->trim();
	for (;;){
		if ((pos2 = str->find(' ', (size_t)pos1)) == -1){
			tmp.copy(str->data(), len - pos1, pos1);
			if ((keysym = XStringToKeysym(tmp.tombs(dummy, 255))) == 0)
				break;
			keyret->keysym = keysym;
			//keyret->modifier_mask = keyret->modifier;
			keyret->modifier_mask = ControlMask | Mod1Mask | ShiftMask;
			return NULL;
			}

		tmp.copy(str->data(), pos2 - pos1, pos1);
		pos1 = pos2 + 1;
		if (tmp.casecompare(CONTROL_MASK_NAME) == 0){
			keyret->modifier |= ControlMask;
			continue;
			}
		if (tmp.casecompare(ALT_MASK_NAME) == 0){
			keyret->modifier |= Mod1Mask;
			continue;
			}
		if (tmp.casecompare(SHIFT_MASK_NAME) == 0){
			keyret->modifier |= ShiftMask;
			continue;
			}

		if ((size_t)pos1 >= len)
			break;
		}

	TWstring * err_msg = new TWstring;
	err_msg->copy(L"failed to create key: ");
	err_msg->append(str->data());
	
	return err_msg;
}

int TIM::iskey(XKeyEvent * e, const XIMTriggerKey * t, int keycnt)
{
	uint32_t modifier;
	uint32_t modifier_mask;
	KeySym k;

	k = XLookupKeysym(e, 0);

	for (int i = 0; i < keycnt; i++){
		modifier = t[i].modifier;
		modifier_mask = t[i].modifier_mask;
		if ((t[i].keysym == k) && ((e->state & modifier_mask) == modifier))
			return i + 1;
		}
	return 0;
}

