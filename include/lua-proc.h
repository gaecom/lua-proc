#ifndef LUA_PROC_H
#define LUA_PROC_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

LUALIB_API int
luaopen_proc(lua_State *L);

#endif /* ! LUA_PROC_H */
