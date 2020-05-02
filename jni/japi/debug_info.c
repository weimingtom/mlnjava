/**
  * Created by MomoLuaNative.
  * Copyright (c) 2019, Momo Group. All rights reserved.
  *
  * This source code is licensed under the MIT.
  * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
  */
//
// Created by Xiong.Fangyu 2019/03/13.
//

#include "debug_info.h"

#if defined(J_API_INFO)

#include "mlog.h"
#include "llimits.h"
#include "lauxlib.h"

void _printTable(lua_State *L, int idx) {
    lua_lock(L);
    if (!lua_istable(L, idx)) {
        LOGE("%d in stack is not a table, it's a %s", idx, LUA_TYPENAME(L, idx));
        lua_unlock(L);
        return;
    }
    idx = idx < 0 ? lua_gettop(L) + idx + 1 : idx;
    lua_pushnil(L);
    LOGI("table-(%d):{", idx);
    int keyType;
    while (lua_next(L, idx)) {
        keyType = lua_type(L, -2);
        if (keyType == LUA_TNUMBER) {
            lua_pushvalue(L, -1);
            LOGI("\t%d:'%s',", (int) lua_tonumber(L, -3), luaL_tolstring(L, -1, NULL));
            lua_pop(L, 2);
        } else {
            lua_pushvalue(L, -1);
            LOGI("\t'%s':'%s',", lua_tostring(L, -3), luaL_tolstring(L, -1, NULL));
            lua_pop(L, 2);
        }

        lua_pop(L, 1);
    }
    LOGI("}");
    lua_unlock(L);
}

void _dumpStack(lua_State *L) {
    LOGI("-------------栈顶start-------------");
    int index, type;
    lua_lock(L);
    for (index = lua_gettop(L); index > 0; --index) {
        type = lua_type(L, index);
        if (type == LUA_TNUMBER) {
            LOGI("(%d) %s %f\n", index, lua_typename(L, type), lua_tonumber(L, index));
        } else {
            lua_pushvalue(L, index);
            LOGI("(%d) %s\n", index, luaL_tolstring(L, -1, NULL));
            lua_pop(L, 2);
        }
    }
    lua_unlock(L);
    LOGI("-------------栈底end-------------");
}

#if !defined(_MSC_VER)
#include <time.h>

static struct timeval start;

void _startTick() {
    gettimeofday(&start, NULL);
}

void _endTick() {
    struct timeval now;
    gettimeofday(&now, NULL);
    LOGI("cast: %.2f", (now.tv_usec - start.tv_usec) / 1000.0);
}
#else
#include <windows.h>

DWORD start;

void _startTick() {
	start = timeGetTime();
}

void _endTick() {
	DWORD now = timeGetTime();
	fprintf(stdout, "cast: %.2f", (now - start) / 1000.0);
}
#endif

#endif