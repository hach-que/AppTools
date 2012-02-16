/*

Code file for argtable2.c

This file is the code file for wrapping the ArgTable2
library to Lua.

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#define C_ONLY
#define LUA_LIB
#define LUA_USE_APICHECK

#include "lua.h"
#include "lauxlib.h"
#include "../config.h"

struct at2_entity
{
	const char* type;
	void* obj;
};

#define NEW_ENTITY() (struct at2_entity*)malloc(sizeof(struct at2_entity));

/**********************************
 * Structure definition functions *
 **********************************/

// I am so going to hell for these preprocessor macros ._.'

#define FUNC(f) lib_argtable2_arg_ ## f
#define DEF_ARGNAME(type, amount) lib_argtable2_arg_ ## type ## amount
#define DEF_ARGTYPE(type) arg_ ## type
#define DEF_ARGCALL(type, amount) arg_ ## type ## amount
#define WRAP_ENTITY(typename, e, s) \
	struct at2_entity *e = NEW_ENTITY(); \
	e->type = #typename; \
	e->obj = (void*)s;

#define DEF_ARG0(type, call) \
static int DEF_ARGNAME(call,0)(lua_State * L) \
{ \
	const char* shortopts = lua_tostring(L, 1); \
	const char* longopts = lua_tostring(L, 2); \
	const char* datatype = lua_tostring(L, 3); \
	const char* glossary = lua_tostring(L, 4); \
	\
	struct DEF_ARGTYPE(type) *s = DEF_ARGCALL(call,0)(shortopts, longopts, datatype, glossary); \
	WRAP_ENTITY(type, e, s); \
	lua_pushlightuserdata(L, (void*)e); \
	return 1; \
}

#define DEF_ARG1(type, call) \
static int DEF_ARGNAME(call,1)(lua_State * L) \
{ \
	const char* shortopts = lua_tostring(L, 1); \
	const char* longopts = lua_tostring(L, 2); \
	const char* datatype = lua_tostring(L, 3); \
	const char* glossary = lua_tostring(L, 4); \
	\
	struct DEF_ARGTYPE(type) *s = DEF_ARGCALL(call,1)(shortopts, longopts, datatype, glossary); \
	WRAP_ENTITY(type, e, s); \
	lua_pushlightuserdata(L, (void*)e); \
	return 1; \
}

#define DEF_ARGN(type, call) \
static int DEF_ARGNAME(call,n)(lua_State * L) \
{ \
	const char* shortopts = lua_tostring(L, 1); \
	const char* longopts = lua_tostring(L, 2); \
	const char* datatype = lua_tostring(L, 3); \
	int mincount = lua_tonumber(L, 4); \
	int maxcount = lua_tonumber(L, 5); \
	const char* glossary = lua_tostring(L, 6); \
	\
	struct DEF_ARGTYPE(type) *s = DEF_ARGCALL(call,n)(shortopts, longopts, datatype, mincount, maxcount, glossary); \
	WRAP_ENTITY(type, e, s); \
	lua_pushlightuserdata(L, (void*)e); \
	return 1; \
}

struct arg_lit* arg_litw0(const char* shortopts, const char* longopts, const char* datatype, const char* glossary) { return arg_lit0(shortopts, longopts, glossary); }
struct arg_lit* arg_litw1(const char* shortopts, const char* longopts, const char* datatype, const char* glossary) { return arg_lit1(shortopts, longopts, glossary); }
struct arg_lit* arg_litwn(const char* shortopts, const char* longopts, const char* datatype, int mincount, int maxcount, const char* glossary) { return arg_litn(shortopts, longopts, mincount, maxcount, glossary); }

DEF_ARG0(str, str);
DEF_ARG0(int, int);
DEF_ARG0(file, file);
DEF_ARG0(lit, litw);
DEF_ARG1(str, str);
DEF_ARG1(int, int);
DEF_ARG1(file, file);
DEF_ARG1(lit, litw);
DEF_ARGN(str, str);
DEF_ARGN(int, int);
DEF_ARGN(file, file);
DEF_ARGN(lit, litw);

/**********************************
 * General functions              *
 **********************************/

static int FUNC(end)(lua_State * L)
{
	int errcount = lua_tonumber(L, 1);
	if (errcount <= 0) errcount = 20;
	struct arg_end *s = arg_end(errcount);
	WRAP_ENTITY(end, e, s);
	lua_pushlightuserdata(L, (void*)e);
	return 1;
}

static int FUNC(parse)(lua_State * L)
{
	// Check to see if a table is the first argument.
	if (!lua_istable(L, 1))
		return luaL_error(L, "argtable2 parse requires table as first argument");

	// Get the data out of the table.
	void** argtable = (void**)calloc(256, sizeof(void*));
	argtable[0] = arg_str1(NULL, NULL, "script", "the name of the script to execute");
	size_t size = lua_objlen(L, 1);
	int i;
	for (i = 0; i < size; i += 1)
	{
		lua_rawgeti(L, 1, i + 1);
		void* ud = lua_touserdata(L, -1);
		lua_pop(L, 1);
		if (ud == NULL)
			break;
		argtable[i+1] = ((struct at2_entity*)ud)->obj;
	}

	// Check to see if the argument definitions were allocated
	// correctly.
	if (arg_nullcheck(argtable))
		return luaL_error(L, "invalid table passed to argtable2 parse");
	
	// Now parse the arguments.
	int nerrors = arg_parse(apputil_get_argc(L), apputil_get_argv(L), argtable);
	free(argtable);

	// Push the result.
	lua_pushnumber(L, nerrors);
	return 1;
}

static int FUNC(print_syntax)(lua_State * L)
{
	// Check to see if a table is the first argument.
	const char* lead = luaL_optstring(L, 1, "Usage: apputil");
	if (!lua_istable(L, 2))
		return luaL_error(L, "argtable2 print_syntax requires table as second argument");
	const char* sep = luaL_optstring(L, 3, "\n");

	// Get the data out of the table.
	void** argtable = (void**)calloc(256, sizeof(void*));
	size_t size = lua_objlen(L, 2);
	int i;
	for (i = 0; i < size; i += 1)
	{
		lua_rawgeti(L, 2, i + 1);
		void* ud = lua_touserdata(L, -1);
		lua_pop(L, 1);
		if (ud == NULL)
			break;
		argtable[i] = ((struct at2_entity*)ud)->obj;
	}

	// Check to see if the argument definitions were allocated
	// correctly.
	if (arg_nullcheck(argtable))
		return luaL_error(L, "invalid table passed to argtable2 print_syntax");
	
	// Now print the syntax.
	printf(lead);
	arg_print_syntax(stdout, argtable, sep);
	free(argtable);

	return 0;
}

static int FUNC(print_glossary)(lua_State * L)
{
	// Check to see if a table is the first argument.
	if (!lua_istable(L, 1))
		return luaL_error(L, "argtable2 print_glossary requires table as first argument");
	const char* fmt = luaL_optstring(L, 2, "    %-25s %s\n");

	// Get the data out of the table.
	void** argtable = (void**)calloc(256, sizeof(void*));
	size_t size = lua_objlen(L, 1);
	int i;
	for (i = 0; i < size; i += 1)
	{
		lua_rawgeti(L, 1, i + 1);
		void* ud = lua_touserdata(L, -1);
		lua_pop(L, 1);
		if (ud == NULL)
			break;
		argtable[i] = ((struct at2_entity*)ud)->obj;
	}

	// Check to see if the argument definitions were allocated
	// correctly.
	if (arg_nullcheck(argtable))
		return luaL_error(L, "invalid table passed to argtable2 print_glossary");
	
	// Now print the syntax.
	arg_print_glossary(stdout, argtable, fmt);
	free(argtable);

	return 0;
}

static int FUNC(print_errors)(lua_State * L)
{
	// Check to see if a table is the first argument.
	if (!lua_islightuserdata(L, 1))
		return luaL_error(L, "argtable2 print_errors requires arg_end as first argument");
	struct at2_entity* e = (struct at2_entity*)lua_touserdata(L, 1);
	if (e == NULL || strcmp(e->type, "end") != 0)
		return luaL_error(L, "argtable2 print_errors requires arg_end as first argument");
	struct arg_end* end = (struct arg_end*)(e->obj);
	const char* name = luaL_optstring(L, 2, "apputil");

	// Now print the errors.
	arg_print_errors(stdout, end, name);

	return 0;
}

static int FUNC(g_count)(lua_State *L)
{
	int count;
	struct at2_entity* e = (struct at2_entity*)lua_touserdata(L, 1);
	if (e->type == "lit")
		count = ((struct arg_lit*)(e->obj))->count;
	else if (e->type == "str")
		count = ((struct arg_str*)(e->obj))->count;
	else if (e->type == "dbl")
		count = ((struct arg_dbl*)(e->obj))->count;
	else if (e->type == "rex")
		count = ((struct arg_rex*)(e->obj))->count;
	else if (e->type == "file")
		count = ((struct arg_file*)(e->obj))->count;
	else if (e->type == "date")
		count = ((struct arg_date*)(e->obj))->count;
	else
		luaL_error(L, "count getter is not valid for type of argument");
	lua_pushnumber(L, count);
	return 1;
}

static int FUNC(g_dval)(lua_State *L)
{
	double value;
	struct at2_entity* e = (struct at2_entity*)lua_touserdata(L, 1);
	if (e->type == "dbl")
		value = ((struct arg_dbl*)(e->obj))->dval[(int)luaL_optnumber(L, 2, 0)];
	else
		luaL_error(L, "dval getter is not valid for type of argument");
	lua_pushnumber(L, value);
	return 1;
}

static int FUNC(g_sval)(lua_State *L)
{
	const char* value;
	struct at2_entity* e = (struct at2_entity*)lua_touserdata(L, 1);
	if (e->type == "str")
		value = ((struct arg_str*)(e->obj))->sval[(int)luaL_optnumber(L, 2, 0)];
	else if (e->type == "rex")
		value = ((struct arg_rex*)(e->obj))->sval[(int)luaL_optnumber(L, 2, 0)];
	else
		luaL_error(L, "sval getter is not valid for type of argument");
	lua_pushstring(L, value);
	return 1;
}

static int FUNC(g_filename)(lua_State *L)
{
	const char* value;
	struct at2_entity* e = (struct at2_entity*)lua_touserdata(L, 1);
	if (e->type == "file")
		value = ((struct arg_file*)(e->obj))->filename[(int)luaL_optnumber(L, 2, 0)];
	else
		luaL_error(L, "filename getter is not valid for type of argument");
	lua_pushstring(L, value);
	return 1;
}

static int FUNC(g_basename)(lua_State *L)
{
	const char* value;
	struct at2_entity* e = (struct at2_entity*)lua_touserdata(L, 1);
	if (e->type == "file")
		value = ((struct arg_file*)(e->obj))->basename[(int)luaL_optnumber(L, 2, 0)];
	else
		luaL_error(L, "filename getter is not valid for type of argument");
	lua_pushstring(L, value);
	return 1;
}

static int FUNC(g_extension)(lua_State *L)
{
	const char* value;
	struct at2_entity* e = (struct at2_entity*)lua_touserdata(L, 1);
	if (e->type == "file")
		value = ((struct arg_file*)(e->obj))->extension[(int)luaL_optnumber(L, 2, 0)];
	else
		luaL_error(L, "filename getter is not valid for type of argument");
	lua_pushstring(L, value);
	return 1;
}

/**********************************
 * Lua library interface          *
 **********************************/

#define COMMAND(NAME) { "arg_" #NAME, lib_argtable2_arg_ ## NAME }

static const luaL_reg argtable2lib[] =
{
	COMMAND(str0),
	COMMAND(int0),
	COMMAND(file0),
	COMMAND(litw0),
	COMMAND(str1),
	COMMAND(int1),
	COMMAND(file1),
	COMMAND(litw1),
	COMMAND(strn),
	COMMAND(intn),
	COMMAND(filen),
	COMMAND(litwn),
	COMMAND(end),
	COMMAND(parse),
	COMMAND(print_syntax),
	COMMAND(print_glossary),
	COMMAND(print_errors),
	COMMAND(g_count),
	COMMAND(g_dval),
	COMMAND(g_sval),
	COMMAND(g_filename),
	COMMAND(g_basename),
	COMMAND(g_extension),
	{ NULL, NULL }
};

LUALIB_API int luaopen_argtable2(lua_State *L)
{
	luaL_register(L, "argtable2", argtable2lib);
	return 1;
}
