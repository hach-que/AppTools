include Makefile.inc

DIRS		= components
EXE		= apputil
OBJS		= $(call uo, apputil.o)
LIBS		= -L. -lpthread -lstdc++ -ldl -lrt -largtable2 -lm ../appfs/libapp.a /usr/lib64/libfuse.a /usr/lib64/liblua.a

all: apputil components

apputil: $(OBJS)
	gcc -Wl,-E -o $(EXE) $(OBJS) $(LIBS)

components: force_look
	@$(ECHO) looking in components : $(MAKE) $(MFLAGS) $(MAKECMDGOALS)
	@cd components; $(MAKE) $(MFLAGS) $(MAKECMDGOALS)

clean:
	@$(ECHO) cleaning up in .
	@-$(RM) -f $(EXE) $(OBJS)
	@-for d in $(DIRS); do (cd $$d; $(MAKE) $(STYLE) clean ); done

force_look:
	@true

