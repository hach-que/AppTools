# packaging.py - This module contains classes and functions that are used to create and
#                mount AppFS packages.
#
# Licensed under an MIT license.  More information at http://code.google.com/p/apptools-dist
#
# Created:        Thursday, 17th June 2010 by James Rhodes
# Last Modified:  <same>

import applib.environment as environment
import applib.fs as fs
from applib.logging import default_logger as log
import tempfile
import subprocess
import os
import time
import signal
from xml.sax import saxutils

class Package():
	def __init__(self, filename):
		self.filename = filename
		self.appname_prefix = "apptools"
		self.mountpoint = None
		self.mountproc = None

		if (not self.isValidEnvironment()):
			raise NoPackagingAvailableException()

	def setPrefix(self, prefix):
		self.appname_prefix = prefix

	def exists(self):
		return os.path.exists(self.filename)
	
	def mount(self):
		# Check to see if we're already mounted.
		if (self.mountpoint != None):
			raise PackageAlreadyMountedException()

		# Check to make sure the package exists.
		if (not self.exists()):
			raise PackageNotFoundException()

		# Create a temporary folder for the mountpoint.
		mountpoint = tempfile.mkdtemp("", self.appname_prefix + "_mountpoint.", "/tmp/")

		# Now use appmount to mount it to the specified mountpoint.
		self.mountproc = subprocess.Popen(["appmount", self.filename, mountpoint])
		i = 0
		while (not os.path.exists(os.path.join(mountpoint, "lost+found")) and i < 10):
			time.sleep(1) # Give it a chance to bring up the mountpoint.
			i += 1

		# Check to see if the mountpoint was brought up.
		if (not os.path.exists(os.path.join(mountpoint, "lost+found"))):
			# We're kind of relying on the fact that Ext2 filesystems always have a lost+found
			# directory here... which probably isn't a good idea.  There really needs to be a
			# better way to detect whether the mount succeeded (hopefully backgrounding the
			# appmount process via Python is only temporary until appmount is fixed so that it
			# will save the data back to the image on close).
			log.showErrorW("Unable to mount the AppFS image.  See messages above.")
			raise PackageMountFailureException()

		#appmount_result = subprocess.call(["appmount", self.filename, mountpoint])
		#if (appmount_result != 0):
		#	log.showErrorW("Unable to mount the AppFS image.  See messages above.")
		#	raise PackageMountFailureException()

		self.mountpoint = mountpoint
		return self.mountpoint

	def unmount(self):
		# Check to make sure we're mounted.
		if (self.mountpoint == None):
			raise PackageNotMountedException()

		# Check to make sure the package exists (appmount will write back to it).
		if (not self.exists()):
			raise PackageNotFoundException()
		
		# Now unmount the image.
		#self.mountproc.send_signal(signal.SIGTERM)
		#self.mountproc.wait()

		result = self.forceDirectoryUnmount(self.mountpoint)
		if (not result):
			raise PackageUnmountFailureException()

		self.mountproc.wait()

		mntpnt = self.mountpoint
		self.mountpoint = None

		try:
			os.rmdir(mntpnt)
		except:
			raise IOError()

		return True

	def create(self, size_bytes):
		# Check to make sure the file doesn't already exist.
		if (self.exists()):
			raise PackageAlreadyExistsException()

		# Create a temporary folder for working in.
		tempfolder = tempfile.mkdtemp("", self.appname_prefix + "_build_area.", "/tmp/")
		log.showInfoW("Total size for Ext2 partition will be: " + str(float(size_bytes) / (1024 * 1024)) + " MB")

		# Create a zero'd file so that we can partition it.
		log.showInfoW("Creating zero'd file at " + os.path.join(tempfolder, "app.ext2") + "...")
		with open(os.path.join(tempfolder, "app.ext2"), "w") as f:
			f.write("".ljust(int(size_bytes), "\0"))

		# Now format the zero'd file.
		log.showInfoW("Formatting Ext2 partition at " + os.path.join(tempfolder, "app.ext2") + "...")
		if (environment.runSilent(["mkfs.ext2", "-F", "-t", "ext2", os.path.join(tempfolder, "app.ext2")]) != 0):
			log.showErrorW("An error occurred while formatting the Ext2 partition.")
			raise PackageCreationFailureException()

		# Attach the blank partition to the AppFS bootstrapping binary.
		log.showInfoW("Attaching Ext2 partition to AppFS file at " + os.path.join(tempfolder, "app.afs") + "...")
		if (subprocess.call(["appattach", os.path.join(tempfolder, "app.ext2"), os.path.join(tempfolder, "app.afs")]) != 0):
			log.showErrorW("An error occurred while attaching the Ext2 partition to")
			log.showErrorO("the AppFS binary.  Check above for error messages.")
			raise PackageCreationFailureException()

		# Move the newly created package to the new location.
		log.showInfoW("Moving package to " + self.filename + "...")
		try:
			os.rename(os.path.join(tempfolder, "app.afs"), self.filename)
		except:
			log.showErrorW("Failed to move AppFS package to target.")
			raise PackageCreationFailureException()
		log.showSuccessW("Package successfully created.")
		
		# Remove our temporary files
		try:
			self.forceFileRemoval(os.path.join(tempfolder, "app.afs"))
			self.forceFileRemoval(os.path.join(tempfolder, "app.ext2"))
			os.rmdir(tempfolder)
			log.showSuccessW("Cleaned up temporary working directory.")
		except:
			log.showErrorW("Unable to clean up working directory at: " + tempfolder)
			log.showErrorO("Please remove directory manually.")
		
		return True
	
	def forceFileRemoval(self, path):
		# Attempt 10 times to remove the specified path with os.remove
		successful = False
		i = 0
		while (not successful and i < 10 and os.path.exists(path)):
			try:
				os.remove(path)
				successful = True
			except:
				successful = False
				time.sleep(1)
			i += 1
		return successful
	
	def forceDirectoryUnmount(self, mount_point):
		ret = environment.runSilent(["fusermount", "-u", mount_point])
		i = 0
		while (ret != 0 and i < 10):
			time.sleep(1)
			ret = environment.runSilent(["fusermount", "-u", mount_point])
			i += 1
		if (ret != 0):
			log.showErrorW("Unable to unmount " + mount_point + ".  Please unmount manually.")
			return False
		return True

	def isValidEnvironment(self):
		result = environment.checkBinaries([
			"unionfs-fuse",
			"fusermount",
			"mkfs.ext2",
			"appmount",
			"appattach"])

		base_requirements = (result["fusermount"] and
			result["mkfs.ext2"] and
			result["appmount"] and
			result["appattach"] and
			result["unionfs-fuse"])

		return base_requirements

class PackageAlreadyExistsException(Exception):
	pass

class PackageNotFoundException(Exception):
	pass

class PackageAlreadyMountedException(Exception):
	pass

class PackageNotMountedException(Exception):
	pass

class PackageMountFailureException(Exception):
	pass

class PackageUnmountFailureException(Exception):
	pass

class PackageCreationFailureException(Exception):
	pass

class NoPackagingAvailableException(Exception):
	pass

class PackageInvalidException(Exception):
	pass

# TODO: Make this use the XML writer classes in Python.
class PackageInfoWriter():
	def __init__(self, app):
		if (not isinstance(app, fs.InstalledApplication)):
			raise PackageInvalidException()
		
		self.app = app
		self.author = None
		self.updateURL = None
		self.archs = []
	
	def setAuthor(self, author):
		self.author = author

	def setArchs(self, archs):
		if (isinstance(archs, list)):
			self.archs = archs
		else:
			raise ValueError()

	def getXML(self):
		updatexml = ""
		if (self.updateURL != None):
			updatexml = "<UpdateURL>" + saxutils.escape(self.updateURL) + """</UpdateURL>
  """
		archxml = ""
		for i in self.archs:
			if (isinstance(i, PackageArchitecture)):
				archxml = "  " + i.getXMLRepresentation() + """
  """
		authorxml = ""
		if (self.author != None):
			authorxml = "<Author>" + saxutils.escape(self.author) + """</Author>
  """
		xml = """<?xml version="1.0" encoding="UTF-8" ?>
<Application>
  <Name>""" + saxutils.escape(self.app.name) + """</Name>
  <Version>""" + saxutils.escape(self.app.version) + """</Version>
  """ + authorxml + updatexml + """<Architectures>
  """ + archxml + """</Architectures>
</Application>"""
		return xml

class PackageArchitecture():
	def __init__(self, arch, deltapatch = None):
		self.arch = arch
		if (deltapatch != None):
			self.onrequest = True
			self.deltapatch = deltapatch
		else:
			self.onrequest = False
			self.deltapatch = None
	
	def getXMLRepresentation(self):
		attribs = ""
		if (self.onrequest):
			attribs += " OnRequest=\"true\""
		if (self.deltapatch != None):
			attribs += " DeltaPatch=\"" + saxutils.escape(self.deltapatch) + "\""
		return "<Arch" + attribs +">" + saxutils.escape(self.arch) + "</Arch>"
