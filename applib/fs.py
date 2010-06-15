# fs.py - This module contains classes and functions relating to applications that already
#         reside on the filesystem (i.e. installed applications).
#
# Licensed under an MIT license.  More information at http://code.google.com/p/apptools-dist
#
# Created:        Tuesday, 15th June 2010 by James Rhodes
# Last Modified:  <same>

import os
import shutil
import stat
from applib.general import AppType, AppFolders, MultipleVersionsException, NoVersionsException
from applib.general import InvalidApplicationException, InvalidApplicationTypeException
from applib.logging import default_logger as log

class InstalledApplication():
	"""Class for containing information about an installed
	   application and performing operations on it."""
	def __init__(self, type, name, version):
		self.type = type
		self.name = name
		self.version = version

		if (self.type == None or self.name == None):
			raise InvalidApplicationException();
		if (AppFolders.get(self.type) == None):
			raise InvalidApplicationTypeException();
	
	def autodetect(self):
		"""Automatically fill in the version parameter."""
		if (self.version == None):
			entries = os.listdir(
				os.path.join(
					AppFolders.get(self.type),
					self.name
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

	def link(self):
		"""Link an application to the base filesystem."""
		adiff = ApplicationDifferencer()

		# Determine the differences between what's in the
		# application's directory and what's currently
		# available from the root filesystem (in relation
		# to this application).
		results = adiff.scan(
				os.path.join(
					AppFolders.get(self.type),
					self.name + "/" + self.version
					)
				);
		
		# Now go through the results, creating directories and
		# symlinking files where required.
		attempt_successes = list()
		attempt_failures = list()
		attempt_exists = list()
		total_files = 0
		for i in results:
			total_files += 1
			if (i[0] == "directory"):
				try:
					os.mkdir(i[2], 0755)
					attempt_successes.append(i[2])
				except:
					log.showErrorW("Unable to create directory " + i[2])
					attempt_failures.append(i[2])
			elif (i[0] == "file"):
				try:
					os.symlink(i[1], i[2])
					attempt_successes.append(i[2])
				except:
					log.showErrorW("Unable to symlink file " + i[2])
					attempt_failures.append(i[2])
			elif (i[0] == "exists"):
				attempt_exists.append(i[2])
			else:
				log.showWarningW("Unknown operation for " + i[1])
		
		return attempt_successes, attempt_failures, total_files

	def unlink(self):
		"""Unlink an application from the base filesystem."""
		adiff = ApplicationDifferencer()

		# Determine the differences between what's in the
		# application's directory and what's currently
		# available from the root filesystem (in relation
		# to this application).
		results = adiff.scan(
				os.path.join(
					AppFolders.get(self.type),
					self.name + "/" + self.version
					),
				True
				);
		
		# Now go through the results, removing directories (if they're
		# empty) and un-symlinking files (but making sure that we only
		# remove symlinks and not normal files).
		attempt_successes = list()
		attempt_failures = list()
		attempt_notexists = list()
		total_files = 0
		for i in results:
			total_files += 1
			if (i[0] == "directory"):
				try:
					os.rmdir(i[2])
					attempt_successes.append(i[2])
				except:
					log.showWarningW("Unable to remove directory " + i[2] + ".  Other applications may be using it.")
					# Failure to remove a directory should not be counted
					# as a failure since quite often directories will not be
					# removed because they are still in use by other applications.
					#attempt_failures.append(i[2])
			elif (i[0] == "file" and stat.S_ISLNK(os.lstat(i[2])[stat.ST_MODE])):
				try:
					os.unlink(i[2])
					attempt_successes.append(i[2])
				except:
					log.showErrorW("Unable to symlink file " + i[2])
					attempt_failures.append(i[2])
			elif (i[0] == "notexists"):
				attempt_notexists.append(i[2])
			else:
				log.showWarningW("Unknown operation for " + i[1])

		return attempt_successes, attempt_failures, total_files
	
	def exists(self):
		return os.path.exists(
				os.path.join(
					AppFolders.get(self.type),
					self.name + "/" + self.version
					)
				);
	
	def location(self):
		return os.path.join(
			AppFolders.get(self.type),
			self.name + "/" + self.version
			)

class ApplicationDifferencer():
	"""A class which determines the differences between what
	   an application makes available in it's folder and what's
	   currently available through the root filesystem."""
	def __init__(self):
		pass
	
	def scan(self, path, inverse = False):
		results = list()
		for root, dirs, files in os.walk(path, topdown = (not inverse)):
			for d in dirs:
				dest_path, src_path = self.calculatePaths(path, os.path.join(root, d))
				if (not inverse):
					if (not os.path.exists(dest_path)):
						results.append(("directory", src_path, dest_path))
					else:
						results.append(("exists", src_path, dest_path))
				else:
					if (os.path.exists(dest_path)):
						results.append(("directory", src_path, dest_path))
					else:
						results.append(("notexists", src_path, dest_path))
			for f in files:
				dest_path, src_path = self.calculatePaths(path, os.path.join(root, f))
				if (not inverse):
					if (not os.path.exists(dest_path)):
						results.append(("file", src_path, dest_path))
					else:
						results.append(("exists", src_path, dest_path))
				else:
					if (os.path.exists(dest_path)):
						results.append(("file", src_path, dest_path))
					else:
						results.append(("notexists", src_path, dest_path))

		return results
	
	def calculatePaths(self, scan_path, file_path):
		rel_path = os.path.relpath(file_path, scan_path)
		return os.path.join("/", rel_path), os.path.join(scan_path, file_path)

