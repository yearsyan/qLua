#pragma once
#include "lua.hpp"

namespace qLua {
    class LuaVM {
    public:
        static LuaVM* globalLuaVM;
        lua_State* vm;
        static void initGlobalLuaVM();
    };
}
