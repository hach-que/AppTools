/*

Code file for main.cpp

This file is the code file for AppUtil.

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include "config.h"
#include <applib/logging.h>

#ifndef LIB_PATH
#error LIB_PATH not defined.
#endif

std::string concat_str(const char* a, const char* b)
{
    std::string str = a;
    str += b;
    return str;
}

int run_lua(const char* name, int argc, char* argv[])
{
    // Create a new Lua instance.
    lua_State * L = lua_open();

    // Open libraries.
    luaL_openlibs(L);

    // Set our internal constants.
    lua_pushstring(L, LIB_PATH);
    lua_setglobal(L, "_APPUTIL_LIB_PATH");
    struct program_state* state = (struct program_state*)malloc(sizeof(struct program_state*));
    state->argc = argc;
    state->argv = argv;
    lua_pushlightuserdata(L, state);
    lua_setglobal(L, "_APPUTIL_PROGRAM_STATE");

    // Execute the bootstrap file.
    int bret = luaL_dofile(L, concat_str(LIB_PATH, ".internal/Bootstrap.lua").c_str());
    if (bret != 0)
    {
        const char* str = lua_tostring(L, -1);
        printf("bootstrap: %s\n", str);
        return bret;
    }

    // Execute file.
    int ret = luaL_dofile(L, name);

    if (ret != 0)
    {
        const char* str = lua_tostring(L, -1);
        printf("%s\n", str);
        return ret;
    }
    else
    {
        if (lua_isnumber(L, -1))
            ret = lua_tonumber(L, -1);
        else
            ret = 0;
        return ret;
    }
}

int main(int argc, char* argv[])
{
    const char* script_path = NULL;

    // Set the application name.
    AppLib::Logging::setApplicationName(std::string("apputil"));

    // Parse the arguments provided.
    struct arg_file *script = arg_file1(NULL, NULL, "script", "the path of the script to execute");
    struct arg_end *end = arg_end(20);
    void *argtable[] = { script, end };
    script->filename[0] = NULL;

    // Check to see if the argument definitions were allocated
    // correctly.
    if (arg_nullcheck(argtable))
    {
        AppLib::Logging::showErrorW("Insufficient memory.");
        return 1;
    }

    // Now parse the arguments.
    int nerrors = arg_parse(argc, argv, argtable);

    // Check to see if there were errors.
    if (nerrors > 0 && script->filename[0] == NULL)
    {
        printf("Usage: apputil");
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_errors(stdout, end, "apputil");
        
        printf("AppUtil - The application scripting utility.\n\n");
        arg_print_glossary(stdout, argtable, "    %-25s %s\n");
        return 1;
    }

    // Store the script path the user has provided.
    script_path = script->filename[0];
    
    // Now execute the script.
    return run_lua(script_path, argc, argv);
}
