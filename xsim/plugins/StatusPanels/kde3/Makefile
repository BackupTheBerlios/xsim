CXX             = g++
CFLAGS          = -g -Wall -O2 -DDEBUG -I/usr/X11R6/include
CPPFLAGS        = 
CXXFLAGS	= -I/usr/local/BerkeleyDB.4.0//include -g -Wall -O2 -DDEBUG -I/usr/X11R6/include -DBDB_VERSION4 -DOS_LINUX
DEFS            = -DHAVE_CONFIG_H
LIBS            = 
LDFLAGS         = -L/usr/local/BerkeleyDB.4.0//lib -g -L/usr/X11R6/lib
INSTALL         = @INSTALL@
INSTALL_PROGRAM = @INSTALL_PROGRAM@
INSTALL_DATA    = @INSTALL_DATA@
LD_SHAREEXE     = ld
MOC		= @MOC@

prefix          = /usr/local
topdir		= /mini/xsim-0.3.8.6-pre
pthreadlib	= -lpthread
gcclib		= `$(CXX) -print-libgcc-file-name`

xsim_libp       = $(prefix)/xsim/plugins
include_path	= $(topdir)/xsim/
kdeinc_path	= NONE
qtinc_path	= NONE
kdelib_path	= NONE
qtlib_path	= NONE
gcc_lib		= $(shell $(CXX) -print-libgcc-file-name)

OBJS=statuspanel_kde3.o
TARGET=statuspanel_kde3.so

all:$(TARGET)

statuspanel_kde3.so:$(OBJS)
	$(LD_SHAREEXE) -shared -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS) $(gcclib) -L$(kdelib_path) -L$(qtlib_path) -lqt-mt -lkdeui $(pthreadlib)

statuspanel_kde3.o:statuspanel_kde3.h statuspanel_kde3.cpp $(include_path)/statuspanel.h $(include_path)/wstring.h $(include_path)/conf.h  $(include_path)/win.h
	$(CXX) -c statuspanel_kde3.cpp -I$(kdeinc_path) -I$(qtinc_path) -I$(include_path) -I$(include_path)/IMdkit $(CXXFLAGS)

clean:
	rm -f *.o $(TARGET) core *~ *.bak *.moc

distclean:clean
	rm -rf Makefile

install:all
	$(topdir)/mkinstalldirs $(xsim_libp)
	cp -f $(TARGET) $(xsim_libp)

uninstall:
	cd $(xsim_libp); rm -f $(TARGET)

