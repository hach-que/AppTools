include ../Makefile.inc

DIRS	= applib
EXE_APPFS	= appfs
EXE_APPMOUNT	= appmount
EXE_APPINSPECT	= appinspect
EXE_APPCREATE   = appcreate
OBJS_APPFS	= $(call uo, appfs.o)
OBJS_APPMOUNT	= $(call uo, appmount.o)
OBJS_APPINSPECT = $(call uo, appinspect.o)
OBJS_APPCREATE  = $(call uo, appcreate.o)
OBJLIBS	= libapp.a
LIBS	= -L. -lpthread -lstdc++ -ldl -lrt -largtable2 -lm ./libapp.a /usr/lib$(LIB_SUFFIX)/libfuse.a

all: appfs appmount appinspect appcreate

appfs: $(OBJS_APPFS) $(OBJLIBS)
	gcc -o $(EXE_APPFS) $(OBJS_APPFS) $(LIBS)

appmount: $(OBJS_APPMOUNT) $(OBJLIBS)
	gcc -o $(EXE_APPMOUNT) $(OBJS_APPMOUNT) $(LIBS)

appinspect: $(OBJS_APPINSPECT) $(OBJLIBS)
	gcc -o $(EXE_APPINSPECT) $(OBJS_APPINSPECT) $(LIBS)

appcreate: $(OBJS_APPCREATE) $(OBJLIBS)
	gcc -o $(EXE_APPCREATE) $(OBJS_APPCREATE) $(LIBS)

libapp.a: force_look
	@$(ECHO) looking in fs : $(MAKE) $(MFLAGS) $(MAKECMDGOALS)
	@cd applib; $(MAKE) $(MFLAGS) $(MAKECMDGOALS)

clean:
	@$(ECHO) cleaning up in .
	@-$(RM) -f $(EXE_APPFS) $(EXE_APPMOUNT) $(EXE_APPINSPECT) $(EXE_APPCREATE) $(OBJS_APPFS) $(OBJS_APPMOUNT) $(OBJS_APPINSPECT) $(OBJS_APPCREATE) $(OBJLIBS)
	@-for d in $(DIRS); do (cd $$d; $(MAKE) $(STYLE) clean ); done

force_look:
	@true

install:
	@true
