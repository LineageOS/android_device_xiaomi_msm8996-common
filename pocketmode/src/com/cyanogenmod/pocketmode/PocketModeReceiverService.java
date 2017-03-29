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
import android.os.UserHandle;
import android.util.Log;

public class PocketModeReceiverService extends Service {
    private static final String TAG = "PocketModeReceiverService";
    private static final boolean DEBUG = false;

    private static final String CUST_INTENT = "com.cyanogenmod.settings.device.CUST_UPDATE";
    private static final String CUST_INTENT_EXTRA = "pocketmode_service";

    @Override
    public void onCreate() {
        if (DEBUG) Log.d(TAG, "Creating service");

        IntentFilter custFilter = new IntentFilter(CUST_INTENT);
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
        this.unregisterReceiver(mUpdateReceiver);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private BroadcastReceiver mUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getBooleanExtra(CUST_INTENT_EXTRA, false)) {
                Log.d(TAG, "Starting main service");
                context.startServiceAsUser(new Intent(context, PocketModeService.class),
                        UserHandle.CURRENT);
            } else {
                Log.d(TAG, "Stopping main service");
                context.stopServiceAsUser(new Intent(context, PocketModeService.class),
                        UserHandle.CURRENT);
            }
        }
    };
}
