include ../../Makefile.inc

CPPFLAGS   += -I.. -I/usr/include/fuse
OBJLIBS     = ../libapp.a
OBJS        = endian.o fs.o fsfile.o fuselink.o logging.o util.o blockstream.o environment.o freelist.o

all: $(OBJLIBS)

../libapp.a: $(call uo,$(OBJS))
	@$(ECHO) $(AR) rv ../libapp.a $?
	@$(AR) rcsv ../libapp.a $?

clean:
	@$(ECHO) cleaning up in applib
	@-$(RM) -f $(call uo,$(OBJS))

force_look:
	@true
