// Include file...

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
//#include <fcntl.h>
#include <argtable2.h>
#ifndef C_ONLY
#include <string>
#include <vector>
#endif
//#include <unistd.h>
//#include <linux/limits.h>
//#include <errno.h>
//#include <iostream>
//#include <fstream>
//#include <pthread.h>
//#include <sys/mount.h>
//#include <sys/signal.h>

struct program_state
{
    int argc;
    char** argv;
};

inline int apputil_get_argc(lua_State * L)
{
    lua_getglobal(L, "_APPUTIL_PROGRAM_STATE");
    void* ud = lua_touserdata(L, -1);
    if (ud == NULL) return 0;
    struct program_state* state = (struct program_state*)ud;
    return state->argc;
}

inline char** apputil_get_argv(lua_State * L)
{
    lua_getglobal(L, "_APPUTIL_PROGRAM_STATE");
    void* ud = lua_touserdata(L, -1);
    if (ud == NULL) return NULL;
    struct program_state* state = (struct program_state*)ud;
    return state->argv;
}
