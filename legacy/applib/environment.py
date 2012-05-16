# environment.py - This module contains functions that are used to evaluate the environment
#                  for suitability
#
# Licensed under an MIT license.  More information at http://code.google.com/p/apptools-dist
#
# Created:        Thursday, 17th June 2010 by James Rhodes
# Last Modified:  <same>

from applib.logging import default_logger as log
import os
import subprocess

def checkBinaries(apps):
	paths = os.environ["PATH"].split(":")
	appsfound = dict()
	for a in apps:
		appsfound[a] = False
	for i in paths:
		for a in apps:
			if os.path.exists(os.path.join(i, a)):
				appsfound[a] = True
	return appsfound

def runSilent(args):
	proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
	proc.wait()
	return proc.returncode
