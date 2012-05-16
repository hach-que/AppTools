/*

Code file for appfs.c

This file is the code file for wrapping the AppFS
library to Lua.

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#define C_ONLY
#define LUA_LIB
#define LUA_USE_APICHECK

#include "lua.hpp"
#include "../config.h"
#include "../../appfs/applib/config.h"
#include "../../appfs/applib/fs.h"
#include "../../appfs/applib/fsmacro.h"
#include "../../appfs/applib/fuselink.h"

/**********************************
 * General functions              *
 **********************************/

#define FUNC(f) lib_appfs_ ## f

int FUNC(load)(lua_State * L)
{
	const char* path = lua_tostring(L, 1);
	
	if (path == NULL)
	{
		std::cout << "error: path not provided." << std::endl;
		lua_pushboolean(L, false);
		return 1;
	}
	else
	{
		AppLib::FUSE::API::load(path);
		lua_pushboolean(L, true);
		return 1;
	}
}

int FUNC(unload)(lua_State * L)
{
	AppLib::FUSE::API::unload();
	return 0;
}

int FUNC(ls)(lua_State * L)
{
	const char* path = lua_tostring(L, 1);
	
	if (path == NULL)
	{
		std::cout << "error: path not provided." << std::endl;
		lua_pushnil(L);
		return 1;
	}
	
	// Check the path is valid.
	if (AppLib::FUSE::Macros::checkPathIsValid(path) != 0)
	{
		// TODO: return some kind of error...
		std::cout << "error: path invalid - " << path << std::endl;
		lua_pushnil(L);
		return 1;
	}
	
	// Check the path exists
	if (AppLib::FUSE::Macros::checkPathExists(path) != 0)
	{
		// TODO: return some kind of error...
		std::cout << "error: path doesn't exist - " << path << std::endl;
		lua_pushnil(L);
		return 1;
	}
	
	// Load buffer.
	AppLib::LowLevel::INode buf(0, "", AppLib::LowLevel::INodeType::INT_UNSET);
	if (AppLib::FUSE::Macros::retrievePathToINode(path, &buf) != 0)
	{
		// TODO: return some kind of error...
		std::cout << "error: can't retrieve path to inode." << std::endl;
		lua_pushnil(L);
		return 1;
	}

	// Check to make sure the inode is a directory.
	if (buf.type != AppLib::LowLevel::INodeType::INT_DIRECTORY)
	{
		// TODO: return -ENOTDIR
		std::cout << "error: path not a directory." << std::endl;
		lua_pushnil(L);
		return 1;
	}

	// Retrieve the INode's children.
	std::vector < AppLib::LowLevel::INode > children = AppLib::FUSE::FuseLink::filesystem->getChildrenOfDirectory(buf.inodeid);

	// Create table.
	lua_createtable(L, children.size() + 2, 0);
	int tbl = lua_gettop(L);
	
	// Use the filler() function to report the entries back to FUSE.
	lua_pushstring(L, ".");
	lua_rawseti(L, tbl, 1);
	lua_pushstring(L, "..");
	lua_rawseti(L, tbl, 2);
	for (int i = 0; i < children.size(); i += 1)
	{
		lua_pushstring(L, children[i].filename);
		lua_rawseti(L, tbl, i + 3);
	}

	// Return table.
	return 1;
}

/**********************************
 * Lua library interface          *
 **********************************/

#define COMMAND(NAME) { #NAME, lib_appfs_ ## NAME }

extern "C"
{
	static const luaL_reg appfslib[] =
	{
		COMMAND(load),
		COMMAND(unload),
		COMMAND(ls),
		{ NULL, NULL }
	};

	LUALIB_API int luaopen_appfs(lua_State *L)
	{
		luaL_register(L, "appfs", appfslib);
		return 1;
	}
}
