#include "java.h"


int allocJavaArguments(lua_State* L) {
    int size = lua_tointeger(L, 1);
    lua_newuserdata(L, sizeof(jvalue)*size);
    return 1;
}

int setArgument(lua_State* L) {
    auto* args = (jvalue*) lua_touserdata(L, 1);
    int idx = lua_tointeger(L, 2);
    size_t sigLen;
    const char* signature = luaL_checklstring(L, 4, &sigLen);
    if (sigLen == 0) {
        switch (signature[0]) {
            case 'Z':
                args[idx].z = lua_toboolean(L, 3);
                return 0;
            case 'B':
                args[idx].b = (unsigned char)lua_tointeger(L, 3);
                return 0;
            case 'C':
                if (lua_isnumber(L, 3)) {
                    args[idx].c = (jchar) lua_tointeger(L, 3);
                } else if (lua_isstring(L, 3)) {
                    auto* data = luaL_checkstring(L, 3);
                    args[idx].c = (unsigned char)data[0];
                }
                return 0;
            case 'S':
                args[idx].s = lua_tointeger(L, 3);
                return 0;
            case 'I':
                args[idx].i = lua_tointeger(L, 3);
                return 0;
            case 'J':
                args[idx].j = lua_tointeger(L, 3);
                return 0;
            case 'F':
                args[idx].f = lua_tonumber(L, 3);
                return 0;
            case 'D':
                args[idx].d = lua_tonumber(L, 3);
                return 0;
        }
    } else {
        auto* obj = (jobject) lua_touserdata(L, 3);
        args[idx].l = obj;
    }
    return 0;
}

int setArgString(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* args = (jvalue*) lua_touserdata(L, 2);
    int idx = lua_tointeger(L, 3);
    size_t len;
    const char* s = luaL_checklstring(L, 4, &len);
    auto* javaStr = ctx->env->NewStringUTF(s);
    args[idx].l = javaStr;
    return 0;
}

int setArgObject(lua_State* L) {
    auto* args = (jvalue*) lua_touserdata(L, 2);
    int idx = lua_tointeger(L, 3);
    auto* obj = (jobject) lua_touserdata(L, 4);
    args[idx].l = obj;
    return 0;
}


int getClass(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    size_t classNameLen;
    const char* className = luaL_checklstring(L, 2, &classNameLen);
    auto* clazz = ctx->env->FindClass(className);
    lua_pushlightuserdata(L, clazz);
    return 1;
}

int getObjectClass(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* obj = (jobject) lua_touserdata(L, 1);
    lua_pushlightuserdata(L, (void *)ctx->env->GetObjectClass(obj));
    return 1;
}

int getMethodID(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    size_t methodLen, sigLen;
    auto clazz = (jclass) lua_touserdata(L, 2);
    const char* methodName = luaL_checklstring(L, 3, &methodLen);
    const char* signature = luaL_checklstring(L, 4, &sigLen);
    jmethodID mID = ctx->env->GetMethodID(clazz, methodName, signature);
    lua_pushlightuserdata(L, mID);
    return 1;
}

int getStaticMethodID(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    size_t methodLen, sigLen;
    auto clazz = (jclass) lua_touserdata(L, 2);
    const char* methodName = luaL_checklstring(L, 3, &methodLen);
    const char* signature = luaL_checklstring(L, 4, &sigLen);
    jmethodID mID = ctx->env->GetStaticMethodID(clazz, methodName, signature);
    lua_pushlightuserdata(L, mID);
    return 1;
}

int getFieldID(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    size_t fieldLen, signLen;
    auto clazz = (jclass) lua_touserdata(L, 2);
    const char* fieldName = luaL_checklstring(L, 3, &fieldLen);
    const char* signature = luaL_checklstring(L, 4, &signLen);
    jfieldID fID = ctx->env->GetFieldID(clazz, fieldName, signature);
    lua_pushlightuserdata(L, fID);
    return 1;
}

int getStaticFieldID(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    size_t fieldLen, signLen;
    auto clazz = (jclass) lua_touserdata(L, 2);
    const char* fieldName = luaL_checklstring(L, 3, &fieldLen);
    const char* signature = luaL_checklstring(L, 4, &signLen);
    jfieldID fID = ctx->env->GetStaticFieldID(clazz, fieldName, signature);
    lua_pushlightuserdata(L, fID);
    return 1;
}

int getObjectField(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* obj = (jobject) lua_touserdata(L, 2);
    auto* fID = (jfieldID) lua_touserdata(L, 3);
    auto* value = (jobject) ctx->env->GetObjectField(obj, fID);
    lua_pushlightuserdata(L, value);
    return 1;
}

int setObjectField(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* obj = (jobject) lua_touserdata(L, 2);
    auto* fID = (jfieldID) lua_touserdata(L, 3);
    auto* value = (jobject) lua_touserdata(L, 4);
    ctx->env->SetObjectField(obj, fID, value);
    return 0;
}


int setField(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* obj = (jobject) lua_touserdata(L, 1);
    auto* fID = (jfieldID) lua_touserdata(L, 3);
    size_t sigLen;
    const char* data = luaL_checklstring(L, 5, &sigLen);
    if (sigLen == 1) {
        switch (data[0]) {
            case 'Z':
                ctx->env->SetBooleanField(obj, fID, lua_toboolean(L, 4));
                return 0;
            case 'B':
                ctx->env->SetByteField(obj, fID, lua_tointeger(L, 4));
                return 0;
            case 'C':
                if (lua_isstring(L, 4)) {
                    size_t len;
                    const char* charStr = luaL_checklstring(L, 4, &len);
                    ctx->env->SetCharField(obj, fID, charStr[0]);
                } else {
                    ctx->env->SetCharField(obj, fID, lua_tointeger(L, 4));
                }
                return 0;
            case 'S':
                ctx->env->SetShortField(obj, fID, lua_tointeger(L, 4));
                return 0;
            case 'I':
                ctx->env->SetIntField(obj, fID, lua_tointeger(L, 4));
                return 0;
            case 'J':
                ctx->env->SetLongField(obj, fID, lua_tointeger(L, 4));
                return 0;
            case 'F':
                ctx->env->SetFloatField(obj, fID, lua_tonumber(L, 4));
                return 0;
            case 'D':
                ctx->env->SetDoubleField(obj, fID, lua_tonumber(L, 4));
                return 0;
            default:
                return 0;
        }
    } else {
        ctx->env->SetObjectField(obj, fID, (jobject)lua_touserdata(L, 4));
    }
    return 0;
}

int callVoidMethod(lua_State* L) {
    int argc = lua_gettop(L);
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* obj = (jobject) lua_touserdata(L, 2);
    auto* mID = (jmethodID) lua_touserdata(L, 3);
    if (argc == 3) {
        ctx->env->CallVoidMethod(obj, mID);
        return 0;
    }
    auto* args = (jvalue*) lua_touserdata(L, 4);
    ctx->env->CallVoidMethodA(obj, mID, args);
    return 0;
}

int callObjectMethod(lua_State* L) {
    int argc = lua_gettop(L);
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* obj = (jobject) lua_touserdata(L, 2);
    auto* mID = (jmethodID) lua_touserdata(L, 3);
    if (argc == 3) {
        jobject res = ctx->env->CallObjectMethod(obj, mID);
        lua_pushlightuserdata(L, res);
        return 1;
    }
    auto* args = (jvalue*) lua_touserdata(L, 4);
    jobject res = ctx->env->CallObjectMethodA(obj, mID, args);
    lua_pushlightuserdata(L, res);
    return 1;
}

int callStaticObjectMethod(lua_State* L) {
    int argc = lua_gettop(L);
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* clazz = (jclass) lua_touserdata(L, 2);
    auto* mID = (jmethodID) lua_touserdata(L, 3);
    if (argc == 3) {
        jobject res = ctx->env->CallStaticObjectMethod(clazz, mID);
        lua_pushlightuserdata(L, res);
        return 1;
    }
    auto* args = (jvalue*) lua_touserdata(L, 4);
    jobject res = ctx->env->CallStaticObjectMethodA(clazz, mID, args);
    lua_pushlightuserdata(L, res);
    return 1;
}

int callStaticVoidMethod(lua_State* L) {
    int argc = lua_gettop(L);
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* clazz = (jclass) lua_touserdata(L, 2);
    auto* mID = (jmethodID) lua_touserdata(L, 3);
    if (argc == 3) {
        ctx->env->CallStaticVoidMethod(clazz, mID);
        return 0;
    }
    auto* args = (jvalue*) lua_touserdata(L, 4);
    ctx->env->CallStaticVoidMethodA(clazz, mID, args);
    return 0;
}

int newJavaString(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    size_t len;
    const char* s = luaL_checklstring(L, 2, &len);
    auto* javaStr = (jstring) ctx->env->NewStringUTF(s);
    lua_pushlightuserdata(L, javaStr);
    return 1;
}

int readJavaString(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* javaStr = (jstring) lua_touserdata(L, 2);
    jboolean c = false;
    const char * data = ctx->env->GetStringUTFChars(javaStr, &c);
    lua_pushstring(L, data);
    ctx->env->ReleaseStringUTFChars(javaStr, data);
    return 1;
}

int getStaticObjectField(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* clazz = (jclass) lua_touserdata(L, 2);
    auto fID = (jfieldID) lua_touserdata(L, 3);
    auto* obj = ctx->env->GetStaticObjectField(clazz, fID);
    lua_pushlightuserdata(L, obj);
    return 1;
}


int getStaticField(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* clazz = (jclass) lua_touserdata(L, 2);
    auto fID = (jfieldID) lua_touserdata(L, 3);
    size_t sigLen;
    const char* signature = luaL_checklstring(L, 4, &sigLen);
    if (sigLen == 1) {
        switch (signature[0]) {
            case 'Z':
                lua_pushboolean(L, ctx->env->GetStaticBooleanField(clazz, fID));
                return 1;
            case 'B':
                lua_pushinteger(L, ctx->env->GetStaticByteField(clazz, fID));
                return 1;
            case 'C':   
                lua_pushfstring(L, "%c", ctx->env->GetStaticCharField(clazz, fID));
                return 1;
            case 'S':
                lua_pushinteger(L, ctx->env->GetStaticShortField(clazz, fID));
                return 1;
            case 'I':
                lua_pushinteger(L, ctx->env->GetStaticIntField(clazz, fID));
                return 1;
            case 'J':
                lua_pushinteger(L, ctx->env->GetStaticLongField(clazz, fID));
                return 1;
            case 'F':
                lua_pushnumber(L, ctx->env->GetStaticFloatField(clazz, fID));
                return 1;
            case 'D':
                lua_pushnumber(L, ctx->env->GetStaticDoubleField(clazz, fID));
                return 1;
            default:
                lua_pushnil(L);
                return 0;
        }
    }
    auto* obj = ctx->env->GetStaticObjectField(clazz, fID);
    lua_pushlightuserdata(L, obj);
    return 1;
}

int setStaticObjectField(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* clazz = (jclass) lua_touserdata(L, 2);
    auto fID = (jfieldID) lua_touserdata(L, 3);
    auto* obj = (jobject) lua_touserdata(L, 4);
    ctx->env->SetStaticObjectField(clazz, fID, obj);
    return 0;
}

int setStaticField(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* clazz = (jclass) lua_touserdata(L, 2);
    auto fID = (jfieldID) lua_touserdata(L, 3);
    size_t sigLen;
    const char* s = lua_tolstring(L, 5, &sigLen);
    if (sigLen == 1) {
        switch (s[0]) {
            case 'Z':
                ctx->env->SetStaticBooleanField(clazz, fID, lua_toboolean(L, 4));
                return 0;
            case 'B':
                ctx->env->SetStaticByteField(clazz, fID, lua_tointeger(L, 4));
                return 0;
            case 'C':
                if (lua_isstring(L, 4)) {
                    size_t l;
                    const char *str = luaL_checklstring(L, 4, &l);
                    ctx->env->SetStaticCharField(clazz, fID, str[0]);
                } else {
                    ctx->env->SetStaticCharField(clazz, fID, lua_tointeger(L, 4));
                }
                return 0;
            case 'S':
                ctx->env->SetStaticShortField(clazz, fID, lua_tointeger(L, 4));
                return 0;
            case 'I':
                ctx->env->SetStaticIntField(clazz, fID, lua_tointeger(L, 4));
                return 0;
            case 'L':
                ctx->env->SetStaticLongField(clazz, fID, lua_tointeger(L, 4));
                return 0;
            case 'F':
                ctx->env->SetStaticFloatField(clazz, fID, lua_tonumber(L, 4));
                return 0;
            case 'D':
                ctx->env->SetStaticDoubleField(clazz, fID, lua_tonumber(L, 4));
                return 0;
            default:
                return 0;
        }
    }
    auto obj = (jobject) lua_touserdata(L, 4);
    ctx->env->SetStaticObjectField(clazz, fID, obj);
    return 0;
}

int getField(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    auto* obj = (jobject) lua_touserdata(L, 2);
    auto fID = (jfieldID) lua_touserdata(L, 3);
    size_t sigLen;
    const char* signature = luaL_checklstring(L, 4, &sigLen);
    if (sigLen == 1) {
        char s = signature[0];
        switch (s) {
            case 'I':
                lua_pushinteger(L, ctx->env->GetIntField(obj, fID));
                return 1;
            case 'J':
                lua_pushinteger(L, ctx->env->GetLongField(obj, fID));
                return 1;
            case 'F':
                lua_pushnumber(L, ctx->env->GetFloatField(obj, fID));
                return 1;
            case 'D':
                lua_pushnumber(L, ctx->env->GetDoubleField(obj, fID));
                return 1;
            case 'Z':
                lua_pushboolean(L, ctx->env->GetBooleanField(obj, fID));
                return 1;
            case 'B':
                lua_pushinteger(L, ctx->env->GetByteField(obj, fID));
                return 1;
            case 'S':
                lua_pushinteger(L, ctx->env->GetShortField(obj, fID));
                return 1;
            case 'C':
                {
                    char c = ctx->env->GetCharField(obj, fID);
                    lua_pushfstring(L, "%c", c);
                }
                return 1;
            default:
                lua_pushnil(L);
                return 1;
        }
    }
    lua_pushlightuserdata(L, ctx->env->GetObjectField(obj, fID));
    return 1;
}

int getAppContext(lua_State* L) {
    auto* ctx = (AndroidContext *)lua_touserdata(L, 1);
    lua_pushlightuserdata(L, ctx->context);
    return 1;
}

int nullUserData(lua_State* L) {
    lua_pushlightuserdata(L, nullptr);
    return 1;
}

typedef struct {
    const char* name;
    int (*method)(lua_State*);
} MethodItem;

static const MethodItem methods[] = {
        {"allocJavaArguments", allocJavaArguments},
        {"setArgument", setArgument},
        {"setArgString", setArgString},
        {"setArgObject", setArgObject},
        {"getMethodID", getMethodID},
        {"getClass", getClass},
        {"getObjectClass", getObjectClass},
        {"callVoidMethod", callVoidMethod},
        {"callStaticVoidMethod", callStaticVoidMethod},
        {"callStaticObjectMethod", callStaticObjectMethod},
        {"callObjectMethod", callObjectMethod},
        {"getStaticMethodID", getStaticMethodID},
        {"getFieldID", getFieldID},
        {"getStaticFieldID", getStaticFieldID},
        {"getStaticObjectField", getStaticObjectField},
        {"setStaticObjectField", setStaticObjectField},
        {"setStaticField", setStaticField},
        {"getStaticField", getStaticField},
        {"getObjectField", getObjectField},
        {"getField", getField},
        {"setField", setField},
        {"setObjectField", setObjectField},
        {"newJavaString", newJavaString},
        {"readJavaString", readJavaString},
        {"getAppContext", getAppContext},
        {"nullUserData", nullUserData}
};

namespace qLua {
    void registerJavaLuaLib(lua_State* L) {
        lua_newtable(L);
        for(auto item : methods) {
            lua_pushstring(L, item.name);
            lua_pushcfunction(L, item.method);
            lua_settable(L, -3);
        }
        lua_setglobal(L, "Java");
    }
}

