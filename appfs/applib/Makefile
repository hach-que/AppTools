include ../Makefile.inc

CPPFLAGS+= -I.. -I/usr/include/fuse
OBJLIBS = ../libapp.a
OBJS    = endian.o fs.o fsfile.o fuse.o logging.o
 
all: $(OBJLIBS)

../libapp.a: $(OBJS)
	@$(ECHO) $(AR) rv ../libapp.a $?
	@$(AR) rv ../libapp.a $?
	@$(ECHO) $(RANLIB) ../libapp.a
	@$(RANLIB) ../libapp.a

clean:
	@$(ECHO) cleaning up in applib
	@-$(RM) -f $(OBJS)

force_look:
	@true
