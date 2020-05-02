/**
  * Created by MomoLuaNative.
  * Copyright (c) 2019, Momo Group. All rights reserved.
  *
  * This source code is licensed under the MIT.
  * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
  */
/**
 * Created by Xiong.Fangyu 2019/02/22
 */

#include "lauxlib.h"
#include "lualib.h"
#include "lobject.h"
#include "lstate.h"
#include "luajapi.h"
#include "debug_info.h"
#include "mlog.h"
#include "luasocket.h"
#include <time.h>
#include "m_mem.h"
#include "compiler.h"
#include "saes.h"
#include "llimits.h"
#include "lfunc.h"
#include "isolate.h"

extern JavaVM *g_jvm;

jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;

    jint result = -1;
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return JNI_ERR;
    }
    jclass cls = (*env)->FindClass(env, JAVA_PATH "LuaCApi");
    if (!cls) {
        return JNI_ERR;
    }

    int len = sizeof(jni_methods) / sizeof(jni_methods[0]);
    //LOGE("on load, SOURCE_LEN : %d, int : %d, void* : %d", SOURCE_LEN, sizeof(int), sizeof(void *));
    if ((*env)->RegisterNatives(env, cls, jni_methods, len) < 0) {
        LOGE("on load error");
        return JNI_ERR;
    }
    FREE(env, cls);
    initJavaInfo(env);
    g_jvm = vm;
    return JNI_VERSION_1_4;
}

int AndroidVersion = 0;

void jni_setAndroidVersion(JNIEnv *env, jobject jobj, jint v) {
    AndroidVersion = (int) v;
}

jboolean jni_check32bit(JNIEnv *env, jobject jobj) {
    return sizeof(size_t) == 4;
}

jboolean jni_isSAESFile(JNIEnv *env, jobject jobj, jstring path) {
    const char *file = GetString(env, path);
    int r = check_file(file);
    ReleaseChar(env, path, file);
    return r;
}

// --------------------------define field--------------------------
extern jclass LuaValue;
// --------------------------define private--------------------------
/**
 * 错误处理函数
 */
static int error_func_traceback(lua_State *);

/**
 * 获取pcall中处理错误的函数的栈位置
 */
int getErrorFunctionIndex(lua_State *L);

// --------------------------end--------------------------

const char *l_db_path = NULL;

void jni_setDatabasePath(JNIEnv *env, jobject jobj, jstring path) {
    l_db_path = GetString(env, path);
}

void jni_setBasePath(JNIEnv *env, jobject jobj, jlong LS, jstring path, jboolean autosave) {
    lua_State *L = (lua_State *) LS;
    lua_lock(L);
    const char *bp = GetString(env, path);
    lua_getglobal(L, LUA_LOADLIBNAME); //-1 package table
    lua_pushstring(L, bp);             //-1 bp -- table
    lua_setfield(L, -2, "path");       //-1 table
    lua_pushboolean(L, (int) autosave); //-1 bool --table
    lua_setfield(L, -2, AUTO_SAVE);    //-1 table
    lua_pop(L, 1);
    ReleaseChar(env, path, bp);
    lua_unlock(L);
}

void jni_setSoPath(JNIEnv *env, jobject jobj, jlong LS, jstring path) {
    lua_State *L = (lua_State *) LS;
    lua_lock(L);
    const char *bp = GetString(env, path);
    lua_getglobal(L, LUA_LOADLIBNAME); //-1 package table
    lua_pushstring(L, bp);             //-1 bp -- table
    lua_setfield(L, -2, "cpath");      //-1 table
    lua_pop(L, 1);
    ReleaseChar(env, path, bp);
    lua_unlock(L);
}
/// ------------------------------------------------------------------
// ---------------------------------gc--------------------------------
/// ------------------------------------------------------------------

#define GC_OFFSET_TIME (CLOCKS_PER_SEC >> 3)
static clock_t last_gc_time = 0;
static int gc_offset_time = 0;

static void gc_cb(lua_State *L, int type) {
	clock_t now;

	if (gc_offset_time <= 0) return;
    now = clock();
    if (last_gc_time != 0) {
        int offset = now - last_gc_time;
        if (offset < gc_offset_time) return;
    }
    last_gc_time = now;
    JNIEnv *env;
    int d = getEnv(&env);
    callbackLuaGC(env, L);
    if (d) detachEnv();
}

void jni_setGcOffset(JNIEnv *env, jobject jobj, int offset) {
    gc_offset_time = offset * GC_OFFSET_TIME;
}

/// ------------------------------------------------------------------
/// -------------------------------debug------------------------------
/// ------------------------------------------------------------------

jlong jni_lvmMemUse(JNIEnv *env, jobject jobj, jlong L) {
#if defined(J_API_INFO)
    lua_State *LS = (lua_State *) L;
    return *(size_t *) (G(LS)->ud);
#else
    return 0;
#endif
}

/// ------------------------------------------------------------------
/// ------------------------------L State-----------------------------
/// ------------------------------------------------------------------
extern void openlibs_forlua(lua_State *L, int debug) {
    lua_lock(L);
    L->l_G->gc_callback = NULL;
    luaL_openlibs(L);
    luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
    lua_pushcfunction(L, isolate_open);
    lua_setfield(L, -2, ISOLATE_LIB_NAME);
    lua_pop(L, 1);

    if (debug) {
        luaopen_socket_core(L);
        lua_pop(L, 1);
    }
    lua_atpanic(L, error_func_traceback);
    lua_pushcfunction(L, error_func_traceback);
    lua_setglobal(L, ERROR_FUN);
    lua_getglobal(L, ERROR_FUN);
    JNIEnv *env;
    int need = getEnv(&env);
    initlog(env);
    if (need) detachEnv();
    init_cache(L);

    lua_getglobal(L, LUA_LOADLIBNAME);          //-1 package table
    luaL_getsubtable(L, -1, "searchers");       //-1 package.searchers table; -2 package table
    lua_pushcfunction(L,
                      searcher_Lua_asset);   //-1: fun;-2 package.searchers table; -3 package table
    int len = lua_objlen(L, -2);
    lua_rawseti(L, -2, ++len);                  //-1: package.searchers table; -2 package table
    lua_pushcfunction(L,
                      searcher_java);        //-1: fun;-2 package.searchers table; -3 package table
    lua_rawseti(L, -2, ++len);
    lua_pop(L, 2);
    if (gc_offset_time > 0)
        G(L)->gc_callback = gc_cb;
    lua_unlock(L);
}

jlong jni_createLState(JNIEnv *env, jobject jobj, jboolean debug) {
#if defined(J_API_INFO)
    size_t *ud = (size_t *) m_malloc(NULL, 0, sizeof(size_t));
    *ud = 0;
    lua_State *L = luaL_newstate1(m_alloc, ud);
#else
    lua_State *L = luaL_newstate();
#endif
    openlibs_forlua(L, (int) debug);

    return (jlong) L;
}

void jni_openDebug(JNIEnv *env, jobject jobj, jlong L) {
    luaopen_socket_core((lua_State *) L);
    lua_pop((lua_State *) L, 1);
}

void jni_close(JNIEnv *env, jobject jobj, jlong L) {
    lua_State *LS = (lua_State *) L;
#if defined(J_API_INFO)
    void *ud = G(LS)->ud;
#endif
    lua_close(LS);
#if defined(J_API_INFO)
    m_malloc(ud, sizeof(size_t), 0);
    cj_log();
#endif
}

/// ------------------------------------------------------------------
/// --------------------------stack for java--------------------------
/// ------------------------------------------------------------------
jobjectArray jni_dumpStack(JNIEnv *env, jobject jobj, jlong L) {
    lua_State *LS = (lua_State *) L;
    lua_lock(LS);
    int size = lua_gettop(LS);
    jobjectArray arr = (*env)->NewObjectArray(env, (jsize) size, LuaValue, NULL);
    int index, i;
    for (index = size; index > 0; --index) {
        i = size - index;
        jobject v = toJavaValue(env, LS, index);
        (*env)->SetObjectArrayElement(env, arr, i, v);
        FREE(env, v);
    }
    lua_unlock(LS);
    return arr;
}

void jni_lgc(JNIEnv *env, jobject jobj, jlong L) {
    lua_State * LS = (lua_State *)L;
    lua_gc(LS, LUA_GCCOLLECT, 0);
}

// --------------------------function--------------------------

void jni_callMethod(JNIEnv *env, jobject jobj, jlong L, jlong method, jlong arg) {
    lua_State *LS = (lua_State *) L;
    callback_method m = (callback_method) method;
    m(LS, (void *) arg);
}

// --------------------------error function--------------------------
int getErrorFunctionIndex(lua_State *L) {
    if (lua_iscfunction(L, 1) && lua_tocfunction(L, 1) == error_func_traceback) {
        return 1;
    }
    lua_getglobal(L, ERROR_FUN);
    return lua_gettop(L);
}

static int error_func_traceback(lua_State *L) {
    const char *msg = lua_isstring(L, 1) ? lua_tostring(L, 1) : "unknown error";
    luaL_traceback(L, L, msg, 2);
    return 1;
}
