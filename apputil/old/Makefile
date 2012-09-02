include ../Makefile.inc

DIRS		= components
EXE		= apputil
OBJS		= $(call uo, apputil.o)
LIBS		= -L. -lpthread -lstdc++ -ldl -lrt -largtable2 -lm ../appfs/libapp.a /usr/lib$(LIB_SUFFIX)/libfuse.a /usr/lib$(LIB_SUFFIX)/liblua.a
CXXFLAGS	+= -DLIB_PATH="\"$(PATH_LIB)/\""

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

install: apputil
	@$(ECHO) installing AppTools
	@$(ECHO) prefix is "$(prefix)"
	@$(ECHO) binaries are going into "$(PATH_BIN)"
	@$(ECHO) libraries are going into "$(PATH_LIB)"
	$(MKDIR) -pv $(PATH_LIB)
	$(MKDIR) -pv $(PATH_BIN)
	$(INSTALL) -c -m 0755 -s $(EXE) $(PATH_BIN)/$(EXE)
	for i in scripts/*; do \
		i=$$(basename $$i); \
		if [ "${i:0:3}" == "app" ]; then \
			$(INSTALL) -v -c -m 0755 scripts/$$i $(PATH_BIN)/$${i%\.*}; \
		else \
			$(INSTALL) -v -c -m 0755 scripts/$$i $(PATH_LIB)/$$i; \
		fi; \
	done
	$(CP) -R libs/* $(PATH_LIB)/
	$(CP) -R libs/.internal $(PATH_LIB)/
	$(CP) -R libs/.native $(PATH_LIB)/
	$(CHMOD) -R u=rwX,go=rX $(PATH_LIB)/
	$(CHOWN) -R root:root $(PATH_LIB)/

