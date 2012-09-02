name "AppTools.Filesystem"
description [[Provides access to the AppTools package filesystem.]]
author "James Rhodes"
lastmodified "16th May, 2012"
require "appfs"

-- Mark our variables as per instance.
context
{
	Load = static,
	Unload = static,
	Ls = static
}

-- Mark our visiblity for variables and functions.
visibility
{
	-- Public
	Load = public,
	Unload = public,
	Ls = public

	-- Private
}

--- Loads a package into the current context.
--  @param path The path of the package to load.
function Filesystem.Load(path)
	return appfs.load(path) or nil
end

--- Unloads the current package from the context.
function Filesystem.Unload()
	return appfs.unload() or nil
end

--- Lists all files in a specified directory.
--  @param path The path to the directory whose contents should be listed.
function Filesystem.Ls(path)
	return appfs.ls(path) or nil
end

