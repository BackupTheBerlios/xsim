#include <dlfcn.h>

#include "panel.h"

//ANSI C++ forbids implicit conversion from `void *' in assignment :(
typedef void * DLHANDLE;

typedef TPanel * (* ClassCreatorProc)();

TPanel * TPanel::create(TWstring * plugins_path, Obj_Config * conf)
{
	TWstring path;
	
	path.copy(plugins_path->data());
	path.append(L"/panel_");
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

	TPanel * panel = ClassCreator();

	if (!panel)
		return NULL;
        
        TWstring * err;
	if ((err = panel->create(conf)) != NULL){
		delete panel;
		TWstring err_msg;
		err_msg.copy(err->data());
		delete err;
		throw err_msg;
		}
	
	panel->dlhandle = handle;

	return panel;
}

void TPanel::destory(TPanel * panel)
{       
	DLHANDLE handle = panel->dlhandle;
	
	delete panel;
	
	dlclose(handle);
}

