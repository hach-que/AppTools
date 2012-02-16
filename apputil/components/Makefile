include ../Makefile.inc

EXE_ARGTABLE2   = argtable2.so
LIB_ARGTABLE2	= -L. -largtable2 /usr/lib64/liblua.a
CFLAGS         += -fPIC
OBJS_CLEAN      = argtable2.o
SCRIPT_COPY     = ../scripts/libs/.native/

all: argtable2 copy

argtable2: argtable2.o
	gcc -shared -Wl,-soname,$(EXE_ARGTABLE2) -o $(EXE_ARGTABLE2) argtable2.o $(LIB_ARGTABLE2)

copy: force_look
	mkdir -pv $(SCRIPT_COPY)
	cp -v *.so $(SCRIPT_COPY)

clean:
	@$(ECHO) cleaning up in .
	@-$(RM) -f $(EXE_ARGTABLE2) $(OBJS_CLEAN)

force_look:
	@true
