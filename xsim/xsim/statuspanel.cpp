#include <dlfcn.h>

#include "statuspanel.h"

//ANSI C++ forbids implicit conversion from `void *' in assignment :(
typedef void * DLHANDLE;

typedef TStatusPanel * (* ClassCreatorProc)();

TStatusPanel * TStatusPanel::create(TWstring * plugins_path, Obj_Config * conf, Status_Pack * status_pack)
{
	TWstring path;
	
	path.copy(plugins_path->data());
	path.append(L"/statuspanel_");
	path.append(conf->ident);
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

	TStatusPanel * sp = ClassCreator();

	if (!sp)
		return NULL;
        
        TWstring * err;
	if ((err = sp->create(conf, status_pack)) != NULL){
		delete sp;
		TWstring err_msg;
		err_msg.copy(err->data());
		delete err;
		throw err_msg;
		}
	
	sp->dlhandle = handle;

	return sp;
}

void TStatusPanel::destory(TStatusPanel * sp)
{       
	DLHANDLE handle = sp->dlhandle;
	
	delete sp;
	
	dlclose(handle);
}

