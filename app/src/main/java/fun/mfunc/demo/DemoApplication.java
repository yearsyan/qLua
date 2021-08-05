package fun.mfunc.demo;

import android.app.Application;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;

import fun.mfunc.qlua.VM;

public class DemoApplication extends Application {

    private String openAssetsFileString(String fileName) throws IOException {
        InputStream inputStream = getAssets().open(fileName);
        int sumBytes = inputStream.available();
        byte[] data = new byte[sumBytes];
        int nRead = 0;
        do {
            nRead += inputStream.read(data, nRead, sumBytes);
        } while (nRead < sumBytes);

        return new String(data);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        VM vm = VM.globalVM;
        try {
            String code = openAssetsFileString("init.lua");
            vm.nativeDoString(code);
            vm.nativeRunLuaFunc("init", this);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
