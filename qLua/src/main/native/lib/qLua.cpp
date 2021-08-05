#include <jni.h>
#include <android/log.h>
#include "qLua.h"
#include "java.h"
#include "binding.h"

#define CLASSNAME ("fun/mfunc/qlua/VM")
#define POINTER_FIELD_NAME ("p")

namespace qLua {

    LuaVM* LuaVM::globalLuaVM = nullptr;

    int luaPrint(lua_State* L) {
        size_t size = 0;
        const char* s = luaL_checklstring(L, 1, &size);
        __android_log_print(ANDROID_LOG_INFO, "lua", "%s", s);
        return 0;
    }

    namespace native {

        void nativeInitVM(JNIEnv* env, jobject thiz) {
            auto pointerID = env->GetFieldID(env->GetObjectClass(thiz), POINTER_FIELD_NAME, "J");
            auto* L = luaL_newstate();
            luaL_openlibs(L);
            registerJavaLuaLib(L);
            lua_register(L, "print", qLua::luaPrint);
            env->SetLongField(thiz, pointerID , (jlong)L);
        }

        lua_State* getLuaState(JNIEnv* env, jobject obj) {
            auto pointerID = env->GetFieldID(env->GetObjectClass(obj), POINTER_FIELD_NAME, "J");
            return (lua_State*) env->GetLongField(obj, pointerID);
        }

        void nativeDoString(JNIEnv* env, jobject thiz, jstring code) {
            auto data = env->GetStringUTFChars(code, nullptr);
            auto* l = getLuaState(env, thiz);
            luaL_dostring(l, data);
            env->ReleaseStringUTFChars(code, data);
        }

        void nativeRunLuaFunc(JNIEnv* env, jobject thiz, jstring name, jobject ctx) {
            auto funcName = env->GetStringUTFChars(name, nullptr);
            auto* L = getLuaState(env, thiz);
            lua_getglobal(L, funcName);
            AndroidContext context = {
                env, ctx
            };
            lua_pushlightuserdata(L, &context);
            lua_call(L, 1, 0);
        }

        void nativeBindLuaToJava(JNIEnv* env, jclass caller, jstring className,
                                 jstring javaName, jstring signature, jstring luaName) {
            auto* classNameCStr = env->GetStringUTFChars(className, nullptr);
            auto* javaNameCStr = env->GetStringUTFChars(javaName, nullptr);
            auto* signatureCStr = env->GetStringUTFChars(signature, nullptr);
            auto* luaNameCStr = env->GetStringUTFChars(luaName, nullptr);
            auto* clazz = env->FindClass(classNameCStr);
            if (clazz == nullptr) {
                return;
            }
            qLua::bindLuaToJava(env, clazz, javaNameCStr, signatureCStr, luaNameCStr);
        }

        static const JNINativeMethod gMethods[] = {
            {"nativeInitVM", "()V", (void*)nativeInitVM},
            {"nativeDoString", "(Ljava/lang/String;)V", (void *)nativeDoString},
            {"nativeRunLuaFunc", "(Ljava/lang/String;Landroid/content/Context;)V", (void  *)nativeRunLuaFunc},
            {
                    "nativeBindLuaToJava",
                    "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                    (void *)nativeBindLuaToJava
            }
        };
    }


    void setJavaGlobalVM(JNIEnv* env, jclass clazz) {
        auto* globalFieldID = env->GetStaticFieldID(clazz, "globalVM", "Lfun/mfunc/qlua/VM;");
        auto* constructor = env->GetMethodID(clazz, "<init>", "(J)V");
        auto* globalVMObj = env->NewObject(clazz, constructor, (jlong)qLua::LuaVM::globalLuaVM->vm);
        env->SetStaticObjectField(clazz, globalFieldID, globalVMObj);
    }

    int registerNativeFunctions(JNIEnv* env, jclass clazz) {
        return env->RegisterNatives(clazz, native::gMethods, sizeof(native::gMethods)/sizeof(JNINativeMethod));
    }

    void LuaVM::initGlobalLuaVM() {
        LuaVM::globalLuaVM = new LuaVM;
        auto* L = luaL_newstate();
        LuaVM::globalLuaVM->vm = L;
        luaL_openlibs(L);
        registerJavaLuaLib(L);
        registerBindOnLua(L);
        lua_register(L, "print", luaPrint);
        lua_toboolean(L, true);
        lua_setglobal(L, "isGlobal"); // set global flags
    }


}



JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = nullptr;
    if(vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    qLua::LuaVM::initGlobalLuaVM();
    auto* clazz = env->FindClass(CLASSNAME);
    if (!clazz) {
        __android_log_print(ANDROID_LOG_ERROR, "jni", "class not found");
        return -1;
    }

    // set global vm on java
    qLua::setJavaGlobalVM(env, clazz);
    if(qLua::registerNativeFunctions(env, clazz)) {
        __android_log_print(ANDROID_LOG_ERROR, "jni", "register lua native fail");
        return -1;
    }

    __android_log_print(ANDROID_LOG_INFO, "jni", "register success");
    return JNI_VERSION_1_6;
}
