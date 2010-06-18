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
import sys
from applib.general import AppType, AppFolders, MultipleVersionsException, NoVersionsException
from applib.general import InvalidApplicationException, InvalidApplicationTypeException, ApplicationNotFoundException
from applib.logging import default_logger as log

def rl(path):
	return os.path.realpath(path)

# Only paths which start with one of the entries
# in FolderTransformations will be linked.
FolderTransformations = {}
FolderTransformations["/bin"]     = rl("/bin")
FolderTransformations["/boot"]    = rl("/boot")
FolderTransformations["/etc"]     = rl("/etc")
FolderTransformations["/include"] = rl("/usr/include")
FolderTransformations["/libexec"] = rl("/bin")
FolderTransformations["/lib/"]    = rl("/lib") + "/"    # Ensure it doesn't get confused with /libexec
FolderTransformations["/man"]     = rl("/usr/man")
FolderTransformations["/sbin"]    = rl("/sbin")
FolderTransformations["/share"]   = rl("/usr/share")
FolderTransformations["/src"]     = rl("/src")
FolderTransformations["/tmp"]     = rl("/tmp")
FolderTransformations["/var"]     = rl("/var")

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
			try:
				entries = os.listdir(
					os.path.join(
						AppFolders.get(self.type),
						self.name
						)
					)
			except OSError:
				raise ApplicationNotFoundException()

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

	def size(self):
		# Calculate the size of the application.
		path = os.path.join(
					AppFolders.get(self.type),
					self.name + "/" + self.version
					)

		size_total = 0
                for root, dirs, files in os.walk(path):
                        size_total += sum(os.path.getsize(os.path.join(root, name)) for name in files)
                        size_total += sum(os.path.getsize(os.path.join(root, name)) for name in dirs)
                
		return size_total

	def oper_mkdir(self, path, mode):
		os.mkdir(path, mode)

	def oper_symlink(self, source, link_name):
		os.symlink(source, link_name)

	def oper_unlink(self, path):
		os.unlink(path)

	def oper_rmdir(self, path):
		os.rmdir(path)

	def getNormalizedApplicationPath(self, version, path):
		return path.replace("/" + version + "/", "/Current/", 1)

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

			# Normalize (replace version number with
			# "Current") the application path.
			sv = self.getNormalizedApplicationPath(self.version, i[1])
			i = (i[0], sv, i[2])

			if (i[0] == "directory"):
				try:
					self.oper_mkdir(i[2], 0755)
					attempt_successes.append(i[2])
				except:
					log.showErrorW("Unable to create directory " + i[2])
					attempt_failures.append(i[2])
			elif (i[0] == "file"):
				try:
					self.oper_symlink(i[1], i[2])
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
		
		safe_app_dir = os.path.join(
                                        AppFolders.get(self.type),
                                        self.name # We exclude the version here because we could be
                                                  # checking against a link that's under Current or
                                                  # a specific version.
                                        )
		
		# Preemptively go through the list of directories, removing those
		# that are symlinks to the application folder.  This is from the legacy
		# link system and unfortunatly if you let the block below this run
		# through a system with said symlinks, you'll end up annihilating the
		# the application files (because it'll walk through the symlink into
		# the application directory and start rm'ing stuff we don't want to)
		# The solution here is to go through and remove directory symlinks before
		# hand, with a reversed result list (in effect reversing the walk process
		# in adiff.scan) so that we elimate the top level symlinks first, preventing
		# it from annihilating symlinked directories inside the application folder.
		# Very annoying stuff.
		#
		# XXX: I almost hosed the entire Elementary system with this.  Apparently it
		#      that removing symlinked directories included some of the base ones
		#      such as /lib and /bin (because the Python install contains those dirs
		#      too :P).  The only_sub variable defines that only paths that resolve
		#      to a *subdirectory* of those specified can be removed if it's a symlinked
		#      directory.  This prevents removal of /bin, /lib, etc.. symlinks.
		#
		only_sub = [
				"/System/Utilities/Applications",
				"/System/Utilities/Libraries",
				"/Applications",
				"/Users"
			]
		results.reverse()
		trip_safety = False
		for i in results:
			# Legacy removal is a special case because directories will be detected
			# as file entries (because they are symlinks).  Therefore, we need to use
			# os.path.realpath and os.path.isdir to find out whether it's really a directory
			# or not.
			is_directory = os.path.isdir(os.path.realpath(i[2]))

			# Get file information.
			try:
				pstat = os.lstat(i[2])[stat.ST_MODE]
			except:
				# Likely broken when we removed a directory symlink.
				continue
			
			# Determine whether we should proceed with this entry.
			if (not is_directory):
				continue
			if (not stat.S_ISLNK(pstat)):
				continue

			# Determine whether it's safe to remove this symlinked dir.
			if (not self.isApplicationOwned(i[2], safe_app_dir)):
				continue
			
			# Double-check before we go unlinking (in case of a logic oversight).
			if (is_directory and stat.S_ISLNK(pstat)):
				trip_safety = True
				try:
					self.oper_unlink(i[2])
					log.showWarningW("Removed symlinked directory at: " + i[2])
					log.showWarningW("The full path was: " + rpath)
				except:
					pass
		results.reverse()		

		if (trip_safety):
			log.showErrorW("Legacy system safety switch was tripped.  This indicates you have")
			log.showErrorO("symlinked directories on your system (from legacy linkage systems).")
			log.showErrorO("The unlinking process has removed at least one of those symlinked")
			log.showErrorO("directories.  In order to make sure application files don't get")
			log.showErrorO("removed, you need to run the unlink process again to ensure the system")
			log.showErrorO("is scanned without symlinked directories.  If the process shows this")
			log.showErrorO("message twice, then STOP and REMOVE THE SYMLINKS MANUALLY.  You risk")
			log.showErrorO("destroying application installations if you continue.")
			sys.exit(1)
		

		# Now go through the results, removing directories (if they're
		# empty) and un-symlinking files (but making sure that we only
		# remove symlinks and not normal files).
		attempt_successes = list()
		attempt_failures = list()
		attempt_notexists = list()
		total_files = 0
		for i in results:
			total_files += 1
			try:
				pstat = os.lstat(i[2])[stat.ST_MODE]
			except:
				# File doesn't exist.  Likely got removed while we unlinked
				# a `symlinked' directory (from old linkage system).
				continue

			# Check to make sure that the file we're going to remove is located
			# within a safe directory.
			if (not self.isApplicationOwned(i[2], safe_app_dir)):
				# This check only applies to symlinks, not real directories.
				if ((i[0] == "file" or i[0] == "directory") and stat.S_ISLNK(pstat)):
					log.showInfoW("Ignoring " + i[2] + " because it's not owned by the application.")
					continue

			if (i[0] == "directory" and not stat.S_ISLNK(pstat)):
				try:
					self.oper_rmdir(i[2])
					attempt_successes.append(i[2])
				except:
					log.showInfoW("Still in use: " + i[2])
					# Failure to remove a directory should not be counted
					# as a failure since quite often directories will not be
					# removed because they are still in use by other applications.
					#attempt_failures.append(i[2])
			elif ((i[0] == "file" or i[0] == "directory") and stat.S_ISLNK(pstat)):
				try:
					self.oper_unlink(i[2])
					attempt_successes.append(i[2])
				except:
					log.showErrorW("Unable to symlink file " + i[2])
					attempt_failures.append(i[2])
			elif (i[0] == "notexists"):
				log.showInfoW("  N " + i[2])
				attempt_notexists.append(i[2])
			elif (i[0] != "notexists" and i[0] != "file" and i[0] != "directory"):
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
	
	def location_unversioned(self):
		return os.path.join(
			AppFolders.get(self.type),
			self.name
			)
	
	"""Checks to see whether the specified path is located within safe_root
	   when normalized (only works with symlinks)."""
	def isApplicationOwned(self, path, safe_root):
		only_sub = [
				safe_root
                        ]
		rpath = os.path.realpath(path)
		issafe = False
		for c in only_sub:
			if (rpath.startswith(c) and rpath != c):
				issafe = True
		return issafe
		

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
				dest_path = self.getTransformedPath(dest_path)
				if (dest_path == None):
					continue

				if (not inverse):
					if (not os.path.exists(dest_path)):
						results.append(("directory", src_path, dest_path))
					else:
						results.append(("exists", src_path, dest_path))
				else:
					# We use lexists in case the symbolic link is owned by this
					# application, but it's broken for some reason (since it still
					# needs to be removed anyway).
					if (os.path.lexists(dest_path)):
						results.append(("directory", src_path, dest_path))
					else:
						results.append(("notexists", src_path, dest_path))
			for f in files:
				dest_path, src_path = self.calculatePaths(path, os.path.join(root, f))
				dest_path = self.getTransformedPath(dest_path)
				if (dest_path == None):
					continue

				if (not inverse):
					if (not os.path.exists(dest_path)):
						results.append(("file", src_path, dest_path))
					else:
						results.append(("exists", src_path, dest_path))
				else:
					# See comment above for lexists explaination.
					if (os.path.lexists(dest_path)):
						results.append(("file", src_path, dest_path))
					else:
						results.append(("notexists", src_path, dest_path))

		return results
	
	def calculatePaths(self, scan_path, file_path):
		rel_path = os.path.relpath(file_path, scan_path)
		return os.path.join("/", rel_path), os.path.join(scan_path, file_path)

        def getTransformedPath(self, path):
                for k, i in FolderTransformations.items():
                        if path.startswith(k):
                                path = i + path[len(k):]
                                return path
                return None
