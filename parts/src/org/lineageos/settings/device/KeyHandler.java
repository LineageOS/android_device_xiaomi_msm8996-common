/*
 * Copyright (C) 2017-2019 The LineageOS Project
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

package org.lineageos.settings.device;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.UserHandle;
import android.provider.Settings;
import android.util.Log;
import android.view.KeyEvent;

import com.android.internal.os.DeviceKeyHandler;

import lineageos.providers.LineageSettings;

import org.lineageos.internal.util.FileUtils;

public class KeyHandler implements DeviceKeyHandler {

    private static final String TAG = KeyHandler.class.getSimpleName();

    private static final String FP_HOME_NODE = "/sys/devices/soc/soc:fpc_fpc1020/enable_key_events";

    private static boolean sScreenTurnedOn = true;
    private static final boolean DEBUG = false;

    private final Context mContext;

    private final BroadcastReceiver mScreenStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)) {
                sScreenTurnedOn = false;
            } else if (intent.getAction().equals(Intent.ACTION_SCREEN_ON)) {
                sScreenTurnedOn = true;
            }
        }
    };

    public KeyHandler(Context context) {
        mContext = context;

        IntentFilter screenStateFilter = new IntentFilter();
        screenStateFilter.addAction(Intent.ACTION_SCREEN_OFF);
        screenStateFilter.addAction(Intent.ACTION_SCREEN_ON);
        mContext.registerReceiver(mScreenStateReceiver, screenStateFilter);
    }

    public KeyEvent handleKeyEvent(KeyEvent event) {
        boolean virtualKeysEnabled = LineageSettings.System.getIntForUser(
                mContext.getContentResolver(),
                LineageSettings.System.FORCE_SHOW_NAVBAR, 0, UserHandle.USER_CURRENT) != 0;
        boolean fingerprintHomeButtonEnabled = FileUtils.isFileReadable(FP_HOME_NODE) &&
                FileUtils.readOneLine(FP_HOME_NODE).equals("1");

        if (!hasSetupCompleted()) {
            return event;
        }

        if (event.getKeyCode() == KeyEvent.KEYCODE_HOME) {
            if (event.getScanCode() == 96) {
                if (DEBUG) Log.d(TAG, "Fingerprint home button tapped");
                return virtualKeysEnabled ? null : event;
            }
            if (event.getScanCode() == 102) {
                if (DEBUG) Log.d(TAG, "Mechanical home button pressed");
                return sScreenTurnedOn &&
                        (virtualKeysEnabled || fingerprintHomeButtonEnabled) ? null : event;
            }
        }
        return event;
    }

    private boolean hasSetupCompleted() {
        return Settings.Secure.getInt(mContext.getContentResolver(),
                Settings.Secure.USER_SETUP_COMPLETE, 0) != 0;
    }
}
