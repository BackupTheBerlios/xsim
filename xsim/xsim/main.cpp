#include <signal.h>
#include <locale.h>
#include <iostream>

#include "xim.h"
#include "conf.h"
#include "wstring.h"

using std::cerr;
using std::endl;

TXIM * xim;
TConf * conf;

void signalhandler(int sig)
{
	delete xim;
	delete conf;
	cerr << "XSIM stopped." << endl;
	exit (0);
}

int localeset()
{
	return (int) setlocale(LC_CTYPE, "");
}

#include <stdexcept>
int main(int argc, char * argv[])
{
	if (!localeset()){
		cerr << "Can't set locale." << endl;
		return 1;
		}

	unsetenv("XMODIFIERS");
	conf = new TConf;

        if (!conf->init(argc, argv)){
		cerr << "Failed reading config file" << endl;
        	return 1;
		}

	try{
		xim = new TXIM;
		}catch (TWstring s){
			char dummy[256];
			cerr << "XSIM : " << s.tombs(dummy, 255) << endl;
			return 1;
			}

	signal(SIGQUIT, signalhandler);
	signal(SIGTERM, signalhandler);
	signal(SIGINT,  signalhandler);

	if (!xim->run(conf))
		return 1;

	delete xim;
	delete conf;

        return 0;
}
