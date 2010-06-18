# sandboxing.py - This module contains classes and functions that are used to create sandboxes
#                 using UnionFS-FUSE.  Relies on applib.environment module.
#
# Licensed under an MIT license.  More information at http://code.google.com/p/apptools-dist
#
# Created:        Thursday, 17th June 2010 by James Rhodes
# Last Modified:  <same>

import applib.environment as environment
from applib.logging import default_logger as log
import tempfile
import subprocess

class Sandbox():
	def __init__(self):
		self.root_enabled = True
		self.ro_overlays = list()
		self.rw_overlay = None
		self.command_chroot = None
		self.sandbox_active = False
		self.sandbox_mountpoint = None
		self.appname_prefix = "apptools"

		if (not self.isValidEnvironment()):
			raise NoSandboxingAvailableException()

	def isValidEnvironment(self):
		result = environment.checkBinaries([
			"unionfs-fuse",
			"fusermount",
			"uchroot",
			"fakechroot",
			"chroot",
			"mkfs.ext2",
			"appmount",
			"appattach"])
		
		base_requirements = (result["fusermount"] and
			result["mkfs.ext2"] and
			result["appmount"] and
			result["appattach"] and
			result["unionfs-fuse"])
		
		if (base_requirements and result["uchroot"]):
			self.command_chroot = ["uchroot"]
			return True
		elif (base_requirements and result["fakechroot"] and result["chroot"]):
			self.command_chroot = ["fakechroot", "chroot"]
			return True
		else:
			return False

	def setPrefix(self, prefix):
		self.appname_prefix = prefix

	def setRootAvailability(self, enabled):
		self.root_enabled = enabled

	def setReadWriteOverlay(self, path):
		self.rw_overlay = path
	
	def setReadOnlyOverlays(self, paths):
		self.ro_overlays = paths
	
	def isActive(self):
		return self.sandbox_active
	
	def getMountPoint(self):
		if (not self.sandbox_active):
			raise SandboxInactiveException()

		return self.sandbox_mountpoint

	def call(self, args):
		if (not isinstance(args, list)):
			raise TypeError("The 'args' argument must be a list (as with subprocess.call).")

		if (not isinstance(args, self.command_chroot)):
			raise ValueError("There is no user-level chroot functionality on this system.")
		
		combined_command = []
		for i in self.command_chroot:
			combined_command.append(i)
		for i in args:
			combined_command.append(i)
		
		return subprocess.call(combined_command)

	def activateSandbox(self):
		if (self.sandbox_active):
			raise SandboxAlreadyActiveException()
		
		# Create a temporary directory for the mountpoint.
		self.sandbox_mountpoint = tempfile.mkdtemp("", self.appname_prefix + "_sandbox.", "/tmp/")

		# Create the overlays string
		overlays = ""
		if (self.rw_overlay != None):
			overlays += self.rw_overlay + "=RW:"
		for i in self.ro_overlays:
			overlays += i + "=RO:"
		if (self.root_enabled):
			overlays += "/=RO:"
		overlays = overlays[:-1]
		
		# Now call the UnionFS-FUSE executable.
		result = subprocess.call(["unionfs-fuse", "-o", "cow,max_files=32768,allow_other,use_ino,suid,dev,nonempty", overlays, self.sandbox_mountpoint])
		if (result != 0):
			return False
		
		# Set the active variable
		self.sandbox_active = True

		return True
	
	def deactivateSandbox(self):
		if (not self.sandbox_active):
			raise SandboxInactiveException()

		if (not self.forceDirectoryUnmount(self.sandbox_mountpoint)):
			return False

		if (not self.forceDirectoryCleanup(self.sandbox_mountpoint)):
			return False
		
		self.sandbox_active = False
		
		return True

	def forceDirectoryUnmount(self, path):
		# Unmounts a directory (attempts 10 times).
		ret = runSilentProc(["fusermount", "-u", mount_point])
		i = 0
		while (ret != 0 and i < 10):
			time.sleep(1)
			ret = runSilentProc(["fusermount", "-u", mount_point])
			i += 1
		if (ret != 0):
			log.showErrorW("Unable to unmount " + mount_point + ".  Please unmount manually.")
			return False
		return True

	def forceDirectoryCleanup(self, directory, recursive=False):
		if (not os.path.exists(directory)):
			return True

		if (recursive):
			return forceDirectoryCleanupRecursive(directory)

		try:
			os.rmdir(directory)
			i = 0
			while (os.path.exists(directory) and i < 10):
				time.sleep(1)
				os.rmdir(directory)
				i += 1
			if (os.path.exists(directory)):
				# failed to remove
				log.showErrorW("Unable to remove directory " + directory + ".  Please remove manually.")
				return False
		except OSError:
			log.showErrorW("Unable to remove directory " + directory + ".  Please remove manually.")
			return False
		return True

class NoSandboxingAvailableException(Exception):
	pass

class SandboxInactiveException(Exception):
	pass

class SandboxAlreadyActiveException(Exception):
	pass
