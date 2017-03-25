/*
 * Copyright (c) 2016 The CyanogenMod Project
 *               2017 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.cyanogenmod.pocketmode;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

import org.cyanogenmod.internal.util.FileUtils;

public class PocketModeService extends Service {
    private static final String TAG = "PocketModeService";
    private static final boolean DEBUG = false;

    private static final String FP_WAKEUP_NODE = "/sys/devices/soc/soc:fpc_fpc1020/enable_wakeup";
    private static final String FP_PROX_INTENT = "com.cyanogenmod.settings.device.FP_PROX_TOGGLE";
    private static final String FP_PROX_INTENT_EXTRA = "fingerprint_proximity";

    private static List<BroadcastReceiver> receivers = new ArrayList<BroadcastReceiver>();

    private ProximitySensor mProximitySensor;

    @Override
    public void onCreate() {
        if (DEBUG) Log.d(TAG, "Creating service");
        mProximitySensor = new ProximitySensor(this);

        IntentFilter custFilter = new IntentFilter();
        custFilter.addAction(FP_PROX_INTENT);
        registerReceiver(mUpdateReceiver, custFilter);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (DEBUG) Log.d(TAG, "Starting service");
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        if (DEBUG) Log.d(TAG, "Destroying service");
        super.onDestroy();
        this.unregisterReceiver(mScreenStateReceiver);
        this.unregisterReceiver(mUpdateReceiver);
        mProximitySensor.disable();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void onDisplayOn() {
        if (DEBUG) Log.d(TAG, "Display on");
        mProximitySensor.disable();
    }

    private void onDisplayOff() {
        if (DEBUG) Log.d(TAG, "Display off");
        if (FileUtils.isFileReadable(FP_WAKEUP_NODE) &&
                FileUtils.readOneLine(FP_WAKEUP_NODE).equals("1")) {
            mProximitySensor.enable();
        }
    }

    private BroadcastReceiver mScreenStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(Intent.ACTION_SCREEN_ON)) {
                onDisplayOn();
            } else if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)) {
                onDisplayOff();
            }
        }
    };

    private BroadcastReceiver mUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getBooleanExtra(FP_PROX_INTENT_EXTRA, false)) {
                IntentFilter screenStateFilter = new IntentFilter(Intent.ACTION_SCREEN_ON);
                screenStateFilter.addAction(Intent.ACTION_SCREEN_OFF);
                registerReceiver(mScreenStateReceiver, screenStateFilter);
                receivers.add(mScreenStateReceiver);
            } else if (receivers.contains(mScreenStateReceiver)) {
                unregisterReceiver(mScreenStateReceiver);
                receivers.remove(mScreenStateReceiver);
            }
        }
    };
}
