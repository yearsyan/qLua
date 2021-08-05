#pragma once
#include <atomic>
#include <map>
#include <string>
#include "context.h"
#include "jni.h"
#include "lua.hpp"

namespace qLua {
    void bindLuaToJava(JNIEnv* env, jclass clazz, const char* javaName,
                       const char* signature, const char* luaName);
    void registerBindOnLua(lua_State* L);
}
