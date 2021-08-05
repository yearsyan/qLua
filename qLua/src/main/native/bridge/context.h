#pragma once
#include <jni.h>
#include <lua.hpp>

typedef struct {
    JNIEnv* env;
    jobject context;
} AndroidContext;

