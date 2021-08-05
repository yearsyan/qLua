package fun.mfunc.qlua;

import android.content.Context;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

public class VM {

    static {
        System.loadLibrary("qLua");
    }

    // lua stack pointer
    private long p = 0;
    public static VM globalVM;

    public VM() {
        nativeInitVM();
    }
    private VM(long p) {
        this.p = p;
    }

    public void evalScript(File file) throws IOException {
        FileInputStream inputStream = new FileInputStream(file.getAbsolutePath());
        int sumBytes = inputStream.available();
        int readBytes = 0;
        byte[] data = new byte[sumBytes];
        do {
            readBytes += inputStream.read(data, readBytes, sumBytes);
        } while (readBytes < sumBytes);
        inputStream.close();
        String code = new String(data);
        nativeDoString(code);
    }

    public static native void nativeBindLuaToJava(String className,String jName,String sig,String luaName);
    private native void nativeInitVM();
    public native void nativeDoString(String code);
    public native void nativeRunLuaFunc(String name, Context ctx);

}
