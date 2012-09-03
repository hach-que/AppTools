# AppTools

AppTools is a new package management system to Linux.  Read more
at http://code.google.com/p/apptools-dist.

## Foundations

AppTools is founded on the principle of *universality*, that an object (such as a package) should run anywhere, under any conditions, without requiring great effort on part of the user.  Following this, we derive that:
  * Packages should be able to be shared amongst distributions without issue.
  * Packages should be able to run in place, installed as the user or installed system-wide without requiring separate versions for each.
  * Packages should be able to refer to different versions of the same library without dependency conflicts.
  * Packages should be able to be uninstalled completely, without remnant files being left behind.
  * The package management system must be able to refer to multiple sources to resolve dependencies on-the-fly, including any base package management system included with the distribution.
  * The package management system must support third-party packages and third-party repository sources.

## Building
Create a new build directory outside the source area and change to this directory.  Run `cmake ../path/to/source` followed by `make` to build a copy of AppTools.

You should use a compiler that supports C++11 such as LLVM/Clang to compile AppTools (you'll get better warnings and error messages with LLVM/Clang as well). 

## Usage
The AppTools infrastructure orientates around the AppFS package format.  Unlike other package formats, this is a full read-write filesystem with on-the-fly resizing which allows applications to be run directly from within their packages as well as installed on a system.

The binaries created by the `appfs` are the core utilities for interacting with the package format, while AppUtil provides a Python-based infrastructure for interacting with the package management system.  By exposing the installation and similar mechanism as easy-to-understand and easy-to-modify Python code, it allows developers and distributions to customize the package management system while still maintaining compatibility with cross-distribution packages.

### Example
First you will need to create an AppFS package.  Assuming you are in the root directory of the project and that you have built each component, try:

```bash
> appfs/appcreate test.afs
> mkdir mount
> appfs/appmount test.afs mount >/dev/null 2>/dev/null &  # See note.
> echo "my file" > mount/test
> # ... other operations here ...
> fusermount -u mount
> cd apputil
> ./apputil list ../test.afs
Opening ../test.afs ... 
.
..
test
```

At the moment the APIs are still under development (in particular the exposed APIs to Python for AppFS need to be more than load / unload / ls).

**NOTE:** AppMount currently runs in the foreground for debugging purposes, thus you need to use `>/dev/null 2>/dev/null &` in order to run it silently in the background.
