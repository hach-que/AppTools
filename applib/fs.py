# fs.py - This module contains classes and functions relating to applications that already
#         reside on the filesystem (i.e. installed applications).
#
# Licensed under an MIT license.  More information at http://code.google.com/p/apptools-dist
#
# Created:        Tuesday, 15th June 2010 by James Rhodes
# Last Modified:  <same>

import os
from applib.general import AppType, AppFolders, MultipleVersionsException, NoVersionsException

class InstalledApplication():
	"""Class for containing information about an installed
	   application and performing operations on it."""
	def __init__(self, type, name, version)
		self.type = type
		self.name = name
		self.version = version
	
	def autodetect(self):
		"""Automatically fill in the version parameter."""
		if (self.version == None):
			entries = os.listdir(
				os.path.join(
					AppFolders.get(self.type),
					name
					)
				)
			entry_has = False
			entry_ver = None
			for e in entries:
				if (e != "Current" and not entry_has):
					entry_has = True
					entry_ver = e
				elif (e != "Current" and entry_has):
					raise MultipleVersionsException()
			if (entry_has):
				self.version = entry_ver
			else:
				raise NoVersionsException()

	def link(self)
		"""Link an application to the base filesystem."""
		adiff = ApplicationDifferencer()

		# Determine the differences between what's in the
		# application's directory and what's currently
		# available from the root filesystem (in relation
		# to this application).
		aresults = 
			adiff.scan(
				os.path.join(
					AppFolders.get(self.type),
					name + "/" + version
					)
				);
		
		print aresults
	
	def exists(self):
		return os.path.exists(
				os.path.join(
					AppFolders.get(self.type),
					name + "/" + version
					)
				);
	
	def location(self):
		return os.path.join(
			AppFolders.get(self.type),
			name + "/" + version
			)

class ApplicationDifferencer():
	"""A class which determines the differences between what
	   an application makes available in it's folder and what's
	   currently available through the root filesystem."""
	def __init__(self):
		pass
	
	def scan(self, path):
		for root, dirs, files in os.walk(path):
			print root
			print "  " + str(dirs)
			print "  " + str(files)

		return ""
	
