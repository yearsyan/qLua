package fun.mfunc.demo;

import androidx.appcompat.app.AppCompatActivity;


import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Button;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;

public class MainActivity extends AppCompatActivity {

    Handler handler = new Handler(Looper.getMainLooper());
    fun.mfunc.qlua.VM vm;

    public static void targetFunc() {
        Log.i("target", "called from lua");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        reLoadContent();
    }

    private void reLoadContent() {

        ((Button)findViewById(R.id.call_button)).setOnClickListener(l -> {
            if (vm == null) {
                vm = fun.mfunc.qlua.VM.globalVM;
            }
            vm.nativeRunLuaFunc("showHelloToast", this);
            vm.nativeDoString("print(\"print from java\")");
        });

        ((Button)findViewById(R.id.call_bind_button)).setOnClickListener(l -> {
            Utils.bindTarget();
        });
        

    }


}