include ../../Makefile.inc

EXE_ARGTABLE2	= argtable2.so
LIB_ARGTABLE2	= -L. -largtable2 /usr/lib$(LIB_SUFFIX)/liblua.a
EXE_APPFS	= appfs.so
LIB_APPFS	= -L. ../../appfs/libapp.a /usr/lib$(LIB_SUFFIX)/liblua.a -lfuse
CFLAGS         += -fPIC
CXXFLAGS       += -fPIC
OBJS_CLEAN      = argtable2.o appfs.o
SCRIPT_COPY     = ../libs/.native/

all: argtable2 appfs copy

argtable2: argtable2.o
	gcc -shared -Wl,-soname,$(EXE_ARGTABLE2) -o $(EXE_ARGTABLE2) argtable2.o $(LIB_ARGTABLE2)
	
appfs: appfs.o
	gcc -fPIC -shared -Wl,-soname,$(EXE_APPFS) -o $(EXE_APPFS) appfs.o $(LIB_APPFS)

copy: force_look
	mkdir -pv $(SCRIPT_COPY)
	cp -v *.so $(SCRIPT_COPY)

clean:
	@$(ECHO) cleaning up in .
	@-$(RM) -f $(EXE_ARGTABLE2) $(EXE_APPFS) $(OBJS_CLEAN)

force_look:
	@true
