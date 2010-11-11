include Makefile.inc

DIRS	= applib
EXE_APPFS	= appfs
EXE_APPMOUNT	= appmount
EXE_APPINSPECT	= appinspect
BUILD_APPFS	=
BUILD_APPMOUNT	= -DAPPMOUNT
OBJS_APPFS	= main.o appfs.o
OBJS_APPMOUNT	= main.o appmount.o
OBJS_APPINSPECT = appinspect.o
OBJLIBS	= libapp.a
LIBS	= -L. -lpthread -lstdc++ -ldl -lrt -largtable2 ./libapp.a /lib/libfuse.a

all: appfs appmount appinspect

appfs: main.o appfs.o $(OBJLIBS)
	@$(ECHO) g++ $(CPPFLAGS) $(BUILD_APPFS) -c -o main.o main.cpp
	@g++ $(CPPFLAGS) $(BUILD_APPFS) -c -o main.o main.cpp
	@$(ECHO) gcc -o $(EXE_APPFS) $(OBJS_APPFS) $(LIBS)
	@gcc -o $(EXE_APPFS) $(OBJS_APPFS) $(LIBS)
	@$(ECHO) rm main.o
	@rm main.o

appmount: main.o appmount.o $(OBJLIBS)
	@$(ECHO) g++ $(CPPFLAGS) $(BUILD_APPMOUNT) -c -o main.o main.cpp
	@g++ $(CPPFLAGS) $(BUILD_APPMOUNT) -c -o main.o main.cpp
	@$(ECHO) gcc -o $(EXE_APPMOUNT) $(OBJS_APPMOUNT) $(LIBS)
	@gcc -o $(EXE_APPMOUNT) $(OBJS_APPMOUNT) $(LIBS)
	@$(ECHO) rm main.o
	@rm main.o

appinspect: appinspect.o $(OBJLIBS)
	@$(ECHO) gcc -o $(EXE_APPINSPECT) $(OBJS_APPINSPECT) $(LIBS)
	@gcc -o $(EXE_APPINSPECT) $(OBJS_APPINSPECT) $(LIBS)

libapp.a: force_look
	@$(ECHO) looking in fs : $(MAKE) $(MFLAGS)
	@cd applib; $(MAKE) $(MFLAGS)

clean:
	@$(ECHO) cleaning up in .
	@-$(RM) -f $(EXE_APPFS) $(EXE_APPMOUNT) $(OBJS_APPFS) $(OBJS_APPMOUNT) $(OBJLIBS)
	@-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

force_look:
	@true
