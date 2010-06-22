include Makefile.inc

DIRS	= fs
EXE_APPFS	= appfs
EXE_APPMOUNT	= appmount
BUILD_APPFS	=
BUILD_APPMOUNT	= -DAPPMOUNT
OBJS	= main.o 
OBJLIBS	=  
LIBS	= -L. /lib/libfuse.a /lib/libargtable2.a -lpthread -lext2fs -lstdc++ -ldl -lrt

all: appfs appmount

appfs: main.o $(OBJLIBS)
	@$(ECHO) g++ $(CPPFLAGS) $(BUILD_APPFS) -c -o main.o main.cpp
	@g++ $(CPPFLAGS) $(BUILD_APPFS) -c -o main.o main.cpp
	@$(ECHO) gcc -o $(EXE_APPFS) $(OBJS) $(LIBS)
	@gcc -o $(EXE_APPFS) $(OBJS) $(LIBS)
	@$(ECHO) rm main.o
	@rm main.o

appmount: main.o $(OBJLIBS)
	@$(ECHO) g++ $(CPPFLAGS) $(BUILD_APPMOUNT) -c -o main.o main.cpp
	@g++ $(CPPFLAGS) $(BUILD_APPMOUNT) -c -o main.o main.cpp
	@$(ECHO) gcc -o $(EXE_APPMOUNT) $(OBJS) $(LIBS)
	@gcc -o $(EXE_APPMOUNT) $(OBJS) $(LIBS)
	@$(ECHO) rm main.o
	@rm main.o

#libfs.a: force_look
#	@$(ECHO) looking in fs : $(MAKE) $(MFLAGS)
#	@cd fs; $(MAKE) $(MFLAGS)

clean:
	@$(ECHO) cleaning up in .
	@-$(RM) -f $(EXE_APPFS) $(EXE_APPMOUNT) $(OBJS) $(OBJLIBS)
	@-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

force_look:
	@true
