# logging.py - Standard logging class for applications that use the 
#              libapp infrastructure.
# 
# Licensed under an MIT license.  More information at http://code.google.com/p/apptools-dist
#
# Created:        Tuesday, 15th June 2010 by James Rhodes
# Last Modified:  <same>

class Logger():
	namefull = ""
	nameblnk = ""

	def setName(self, name):
		self.namefull = name
		self.nameblnk = "".ljust(len(name))
	
	def showErrorW(self, msg):
		self.__showMsg("error", msg)
	
	def showErrorO(self, msg):
		self.__showBlankMsg(msg)
	
	def showWarningW(self, msg):
		self.__showMsg("warning", msg)
	
	def showInfoW(self, msg):
		self.__showMsg("info", msg)
	
	def showSuccessW(self, msg):
		self.__showMsg("success", msg)
	
	showWarningO = showErrorO
	showInfoO = showErrorO
	showSuccessO = showErrorO
	
	def __showMsg(self, status, msg):
		msg = msg.replace("\n", "\n" + self.nameblnk + "              ")
		print self.namefull + ": [ " + status.ljust(7) + " ] " + msg
	
	def __showBlankMsg(self, msg):
		msg = msg.replace("\n", "\n" + self.nameblnk + "              ")
		print self.nameblnk + "              " + msg

default_logger = Logger()
