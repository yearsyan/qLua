function runNative(ctx)
    local clazz = Java.getClass(ctx, "fun/mfunc/changedyn/MainActivity")
    local method = Java.getStaticMethodID(ctx, clazz, "targetFunc", "()V")
    Java.callStaticVoidMethod(ctx, clazz, method)
end

function bindFunc(ctx)
    local clazz = Java.getClass(ctx, "fun/mfunc/demo/Utils")
    bindJava(ctx, clazz, "bindTarget", "()V", "qq")
end

function showToast(ctx, text)
    local activityThreadClass = Java.getClass(ctx, "android/app/ActivityThread")
    local fieldID = Java.getStaticFieldID(ctx, activityThreadClass, "sCurrentActivityThread", "Landroid/app/ActivityThread;")
    local activityThread = Java.getStaticObjectField(ctx, activityThreadClass, fieldID)
    local applicationFieldID = Java.getFieldID(ctx, activityThreadClass, "mInitialApplication", "Landroid/app/Application;")
    local application = Java.getObjectField(ctx, activityThread, applicationFieldID)
    local toastClass = Java.getClass(ctx, "android/widget/Toast")
    local makeTextMethodID = Java.getStaticMethodID(ctx, toastClass, "makeText", "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;")
    local args = Java.allocJavaArguments(3);
    Java.setArgObject(ctx, args,0, application)
    Java.setArgString(ctx, args, 1, text)
    Java.setArgument(args, 2, 1, "I")
    local toast = Java.callStaticObjectMethod(ctx, toastClass, makeTextMethodID, args)
    local showMethod = Java.getMethodID(ctx, toastClass, "show", "()V")
    Java.callVoidMethod(ctx, toast, showMethod)
end

function showHelloToast(ctx)
    showToast(ctx, "Hello from lua,call directly")
end

function qq(ctx, obj, args)
    print("java -> native -> lua, call at "..os.date("%c"))
    showToast(ctx, "Hello from lua, call by native method")
end

function init(ctx)
    bindFunc(ctx)
end