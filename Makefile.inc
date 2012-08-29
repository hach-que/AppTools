# vim: set filetype=make:

####################
## Basic settings ##
####################

PATH_LIB = $(prefix)/lib/apptools
PATH_BIN = $(prefix)/bin

LDFLAGS	         = 
ECHO	         = echo
RANLIB           = ranlib
INSTALL          = install
MKDIR            = mkdir
CP               = cp
CHOWN            = chown
CHMOD            = chmod

######################
## Compiler control ##
######################

# Retrieve our architecture setting
ARCH = $(shell getconf LONG_BIT)
ARCH_SUFFIX = $(if $(firstword $(filter 64,$(ARCH))),,64)

# Define our switches for each compiler mode.
CPPFLAGS_optimal = -D_FILE_OFFSET_BITS=64 -I/usr/include -I../appfs
CXXFLAGS_optimal = -O3 -Os -Wno-write-strings
CFLAGS_optimal   = -O3 -Os
CPPFLAGS_debug   = -D_FILE_OFFSET_BITS=64 -I/usr/include/ -I../appfs -DDEBUG
CXXFLAGS_debug   = -g3 -Wno-write-strings -fno-inline -fdiagnostics-show-option
CFLAGS_debug	 = -g3

# Define our error messages.
ERROR_NOSTYLE    = No style "debug" or "optimal" set
ERROR_NOTARGET   = You must provide a build target (try "all")

# Ensure that the user is providing the correct arguments.
STYLE := $(firstword $(filter debug optimal,$(MAKECMDGOALS)))
$(if $(STYLE),,$(error $(ERROR_NOSTYLE)))
$(if $(filter 1, $(words $(MAKECMDGOALS))),$(error $(ERROR_NOTARGET)),)

# Set our compiler flags based on the style.
CPPFLAGS         = $(CPPFLAGS_$(STYLE))
CXXFLAGS         = $(CXXFLAGS_$(STYLE))
CFLAGS           = $(CFLAGS_$(STYLE))

# Tell make that these are phony targets (they don't do anything).
debug optimal:
.PHONY: debug optimal

#########################
## Object file control ##
#########################

# Create a rule for wrapping object names.
define uo
$(shell mkdir -p $(OBJDIR)/components >/dev/null 2>/dev/null; echo $(addprefix $(OBJDIR)/, $1))
endef

# Define where we want to store object files.
OBJDIR           = obj/$(STYLE)

# Override the implicit .o rules.
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $?
$(OBJDIR)/%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $?

