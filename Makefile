include Makefile.inc

# See Makefile.inc for configuration settings.

DIRS		= appfs apputil

all: appfs apputil

appfs: force_look
	@$(ECHO) looking in appfs : $(MAKE) $(MFLAGS) $(MAKECMDGOALS)
	@cd appfs; $(MAKE) $(MFLAGS) $(MAKECMDGOALS)

apputil: force_look
	@$(ECHO) looking in apputil : $(MAKE) $(MFLAGS) $(MAKECMDGOALS)
	@cd apputil; $(MAKE) $(MFLAGS) $(MAKECMDGOALS)

clean:
	@$(ECHO) cleaning up in .
	@-for d in $(DIRS); do (cd $$d; $(MAKE) $(STYLE) clean ); done

force_look:
	@true

