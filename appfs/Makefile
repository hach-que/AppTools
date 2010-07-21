include Makefile.inc

DIRS	= applib
EXE_APPFS	= appfs
EXE_APPMOUNT	= appmount
BUILD_APPFS	=
BUILD_APPMOUNT	= -DAPPMOUNT
OBJS_APPFS	= main.o appfs.o
OBJS_APPMOUNT	= main.o appmount.o
OBJLIBS	= libapp.a
LIBS	= -L. -lpthread -lstdc++ -ldl -lrt -largtable2 -lfuse ./libapp.a

all: appfs appmount

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

libapp.a: force_look
	@$(ECHO) looking in fs : $(MAKE) $(MFLAGS)
	@cd applib; $(MAKE) $(MFLAGS)

clean:
	@$(ECHO) cleaning up in .
	@-$(RM) -f $(EXE_APPFS) $(EXE_APPMOUNT) $(OBJS) $(OBJLIBS)
	@-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

force_look:
	@true
