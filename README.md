AppTools
======================

AppTools is a new package management system to Linux.  Read more
at http://code.google.com/p/apptools-dist.

Foundations
--------------

AppTools is founded on the principle of *universality*, that an object (such as a package) should run anywhere, under any conditions, without requiring great effort on part of the user.  Following this, we derive that:
  * Packages should be able to be shared amongst distributions without issue.
  * Packages should be able to run in place, installed as the user or installed system-wide without requiring separate versions for each.
  * Packages should be able to refer to different versions of the same library without dependency conflicts.
  * Packages should be able to be uninstalled completely, without remnant files being left behind.
  * The package management system must be able to refer to multiple sources to resolve dependencies on-the-fly, including any base package management system included with the distribution.
  * The package management system must support third-party packages and third-party repository sources.
