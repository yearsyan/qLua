//
// Created by Administrator on 2021/8/2.
//

#include <sys/mman.h>
#include <cstring>
#include <string>
#include <android/log.h>
#include "qLua.h"
#include "binding.h"
#include "java.h"

#define toExecutableMem(m) (getExecutableMem((void *)m, sizeof(m)))

#define FUNCTION_ADDR_OFFSET (76)
#define CALL_ID_OFFSET (44)
const unsigned char callNextFunc[] = {
        0xFF, 0x43, 0x01, 0xD1, // sub sp, sp, #0x50
        0xFD, 0x7B, 0x04, 0xA9,
        0xFD, 0x03, 0x01, 0x91,
        0x48, 0xD0, 0x3B, 0xD5,
        0x09, 0x15, 0x40, 0xF9,
        0xA9, 0x83, 0x1F, 0xF8,
        0xE0, 0x13, 0x00, 0xF9,
        0xE1, 0x0F, 0x00, 0xF9,
        0xE2, 0x0B, 0x00, 0xF9,
        0xE9, 0x13, 0x40, 0xF9,
        0xA9, 0x83, 0x1E, 0xF8,
        0x09, 0x00, 0x80, 0xD2, // MOV X9 --start index 11*4 = 44
        0x09, 0x00, 0xA0, 0xF2,
        0x09, 0x80, 0xC0, 0xF2,
        0x09, 0x00, 0xE0, 0xF2,
        0xA9, 0x03, 0x1F, 0xF8, // all move 0
        0xE1, 0x0F, 0x40, 0xF9,
        0xE2, 0x0B, 0x40, 0xF9,
        0xA0, 0x63, 0x00, 0xD1,
        0x09, 0x00, 0x80, 0xD2, // MOV X9 --start index 19*4 = 76
        0x09, 0x00, 0xA0, 0xF2, // offset 16
        0x09, 0x80, 0xC0, 0xF2, // offset 32
        0x09, 0x00, 0xE0, 0xF2, // offset 48 end set function
        0xE8, 0x07, 0x00, 0xF9,
        0x20, 0x01, 0x3F, 0xD6, // BLR X9
        0xE8, 0x07, 0x40, 0xF9,
        0x09, 0x15, 0x40, 0xF9,
        0xAA, 0x83, 0x5F, 0xF8,
        0x29, 0x01, 0x0A, 0xEB,
        0xFD, 0x7B, 0x44, 0xA9,
        0xFF, 0x43, 0x01, 0x91, // add sp, sp, #0x50
        0xC0, 0x03, 0x5F, 0xD6  // RET
};


typedef struct {
    JNIEnv* env;
    void* target;
} JumpCtx;
typedef void* (*EntryFunction)(JNIEnv* , void*, void*);
void* toLua(JumpCtx *p, void* obj, void* args);

void* getExecutableMem(void* src ,size_t size) {
    auto* res = mmap(nullptr, size, PROT_EXEC|PROT_READ|PROT_WRITE,
                MAP_PRIVATE|MAP_ANON, -1, 0);
    if (res == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, "native", "mmap fail");
        return nullptr;
    }
    memcpy(res, src, size); // execute after copy immediately will crash.
    return res;
}

void setMoveI64Instrument(void* address,int offset, uint64_t value) {
    for(int shiftNum = 0, instructionOffset = 0; shiftNum < 64; shiftNum+= 16, instructionOffset+=4) {
        auto* instructionPtr = (unsigned char *)address + offset + instructionOffset;
        int64_t mask = 0xffff;
        mask <<=shiftNum;
        int bits = (int)((value & mask) >> shiftNum);
        *instructionPtr = (*instructionPtr) | ((bits & 0b111) << 5);
        *(instructionPtr + 1) = ((bits & 0b11111111000) >> 3);
        *(instructionPtr + 2) = *(instructionPtr+2) | ((bits & 0xf800) >> 11); // 0xf800 = 0b1111100000000000
    }
}

void* getNewBridgeCode(void* id, void* nFunc) {
#ifdef __aarch64__
    void* res = toExecutableMem(callNextFunc);
    if (res == nullptr) {
        return nullptr;
    }
    setMoveI64Instrument(res, FUNCTION_ADDR_OFFSET, (uint64_t)nFunc);
    setMoveI64Instrument(res, CALL_ID_OFFSET, (uint64_t)id);
    return res;
#else
    __android_log_print(ANDROID_LOG_ERROR, "native", "bridge not implement on this platform");
#endif
    return nullptr;
}

void* toLua(JumpCtx *p, void* obj, void* args) {
    auto* L = qLua::LuaVM::globalLuaVM->vm;
    auto* funcName = (const char*)p->target;
    AndroidContext androidContext = {p->env, nullptr};
    lua_getglobal(L, funcName);
    lua_pushlightuserdata(L, &androidContext);
    lua_pushlightuserdata(L, obj);
    lua_pushlightuserdata(L, args);
    lua_call(L, 3, 1);
    return lua_touserdata(L, -1);
}


namespace qLua {

    void bindLuaToJava(JNIEnv *env, jclass clazz, const char* javaName,
                                   const char* signature, const char* luaName) {
        auto luaNameLen = strlen(luaName);
        char* cName = (char *) malloc(luaNameLen+1);
        memcpy(cName, luaName, luaNameLen);
        cName[luaNameLen] = '\0';
        auto* bridge = getNewBridgeCode((void *)cName, (void *)toLua);
        JNINativeMethod gMethod[] = {{javaName, signature, bridge}};
        env->RegisterNatives(clazz, gMethod, 1);
    }

    int bindJava(lua_State* L) {
        size_t jLen, sigLen, luaLen;
        auto* ctx = (AndroidContext*)lua_touserdata(L, 1);
        auto* clazz = (jclass) lua_touserdata(L, 2);
        auto* javaName = luaL_checklstring(L, 3, &jLen);
        auto* signature = luaL_checklstring(L, 4, &sigLen);
        auto* luaName= luaL_checklstring(L, 5, &luaLen);
        bindLuaToJava(ctx->env, clazz, javaName, signature, luaName);
        return 0;
    }

    void registerBindOnLua(lua_State* L) {
        lua_register(L, "bindJava", bindJava);
    }

}