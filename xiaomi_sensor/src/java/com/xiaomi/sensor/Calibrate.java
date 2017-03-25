package com.xiaomi.sensor;

import android.util.Log;

public class Calibrate
{
    private static final String TAG = "xiaomi_sensor_Calibrate";

    static {
        Log.e("xiaomi_sensor_Calibrate", "loadlibrary");
        System.loadLibrary("sensor_calJNI");
    }

    public static native boolean nativeCalibrate(int paramInt1, int paramInt2);

    public static native double nativeGetconfig(int paramInt1, int paramInt2);
}
