# general.py - This module contains classes and functions that are used generally throughout
#              the AppTools suite (such as application types).
#
# Licensed under an MIT license.  More information at http://code.google.com/p/apptools-dist
#
# Created:        Tuesday, 15th June 2010 by James Rhodes
# Last Modified:  <same>

import os
import pwd

class AppType():
	SYSTEM_ESSENTIAL_APP = 0    # Essential system application (i.e. binutils).
	SYSTEM_ESSENTIAL_LIB = 1    # Essential system library (i.e. libc).
	SYSTEM_OPTIONAL_APP = 2     # Non-essential application (i.e. lynx).
	SYSTEM_HW_DRIVER = 3        # Hardware driver.
	SYSTEM_GLOBAL_APP = 4       # Global system data (such as kernel / filesystem structure updates).
	USER_OPTIONAL_APP = 5       # Non-essential application installed to user's directory.
	APPLICATION_EXTENSION = 6   # Application extension.

class AppFolders():
	@classmethod
	def get(cls, type, user = None):
		if (type == AppType.SYSTEM_ESSENTIAL_APP):
			return "/System/Utilities/Applications"
		elif (type == AppType.SYSTEM_ESSENTIAL_LIB):
			return "/System/Utilities/Libraries"
		elif (type == AppType.SYSTEM_OPTIONAL_APP):
			return "/Applications"
		elif (type == AppType.SYSTEM_GLOBAL_APP):
			return "/"
		elif (type == AppType.SYSTEM_HW_DRIVER):
			# TODO: Get the real kernel information
			#       from uname.
			return "/System/Utilities/Applications/Linux/2.6.32.7/lib/modules/2.6.32.7/kernel/drivers"
		elif (type == AppType.USER_OPTIONAL_APP):
			if (user == None):
				user = pwd.getpwuid(os.getuid()).pw_name
			if (not os.path.exists("/Users/" + user)):
				return None
			return "/Users/" + user + "/Applications"
		elif (type == AppType.APPLICATION_EXTENSION):
			return None
		else:
			return None

class CommandLine():
	@classmethod
	def applyOptions(cls, parser):
		parser.add_option("-l", "--library", dest="apptype_library", action="store_true", default=False,
		                                     help="The application is a library.");
		parser.add_option("-a", "--application", dest="apptype_library", action="store_false",
		                                     help="The application is not a library.");
		parser.add_option("-e", "--essential", dest="apptype_essential", action="store_true", default=True,
		                                     help="The application is essential to system operation.")
		parser.add_option("-o", "--optional", dest="apptype_essential", action="store_false",
		                                     help="The application is not essential.");
		parser.add_option("-s", "--system", dest="apptype_system", action="store_true", default=True,
		                                     help="The application is installed system-wide.")
		parser.add_option("-u", "--user", dest="apptype_system", action="store_false",
		                                     help="The application resides in the user's directory.")

	@classmethod	
	def getTypeFromOptions(cls, options):
		if (not options.apptype_library and options.apptype_essential and options.apptype_system):
			return AppType.SYSTEM_ESSENTIAL_APP
		elif (options.apptype_library and options.apptype_essential and options.apptype_system):
			return AppType.SYSTEM_ESSENTIAL_LIB
		elif (not options.apptype_library and not options.apptype_essential and options.apptype_system):
			return AppType.SYSTEM_OPTIONAL_APP
		elif (not options.apptype_library and not options.apptype_essential and not options.apptype_system):
			return AppType.USER_OPTIONAL_APP
		else:
			return None

# Exceptions
class MultipleVersionsException(Exception):
	pass

class NoVersionsException(Exception):
	pass

class InvalidApplicationException(Exception):
	pass

class InvalidApplicationTypeException(Exception):
	pass

class ApplicationNotFoundException(Exception):
	pass
