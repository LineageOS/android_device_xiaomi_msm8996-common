/*
 * Copyright (C) 2016 The CyanogenMod Project
 *               2017-2019,2021 The LineageOS Project
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

package org.lineageos.settings.buttons;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.UserHandle;
import android.util.Log;

import androidx.preference.PreferenceManager;
import androidx.preference.SwitchPreference;

import org.lineageos.internal.util.FileUtils;

public class ButtonUtils {

    private static final String TAG = "XiaomiParts";

    private static boolean isPreferenceEnabled(Context context, String key) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        return preferences.getBoolean(key, (Boolean) ButtonConstants.sNodeDefaultMap.get(key));
    }

    public static void restoreSavedPreferences(Context context) {
        for (String pref : ButtonConstants.sButtonPrefKeys) {
            String node = ButtonConstants.sBooleanNodePreferenceMap.get(pref);
            String value = isPreferenceEnabled(context, pref) ? "1" : "0";
            if (!FileUtils.writeLine(node, value)) {
                Log.w(TAG, "Write to node " + node +
                    " failed while restoring saved preference values");
            }
        }
    }

    public static void updateDependentPreference(Context context, SwitchPreference b,
            String key, boolean shouldSetEnabled) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        boolean prefActualValue = preferences.getBoolean(key, false);

        if (shouldSetEnabled) {
            if (ButtonConstants.sNodeUserSetValuesMap.get(key) != null &&
                    (Boolean) ButtonConstants.sNodeUserSetValuesMap.get(key)[1] &&
                    (Boolean) ButtonConstants.sNodeUserSetValuesMap.get(key)[1] !=
                    prefActualValue) {
                b.setChecked(true);
                ButtonConstants.sNodeUserSetValuesMap.put(key,
                        new Boolean[]{ prefActualValue, false });
            }
        } else {
            if (b.isEnabled() && prefActualValue) {
                ButtonConstants.sNodeUserSetValuesMap.put(key,
                        new Boolean[]{ prefActualValue, true });
            }
            b.setEnabled(false);
            b.setChecked(false);
        }
    }

    public static boolean hasButtonProcs() {
        return FileUtils.fileExists(ButtonConstants.FP_HOME_KEY_NODE) ||
                FileUtils.fileExists(ButtonConstants.FP_WAKEUP_NODE);
    }

    public static void checkPocketModeService(Context context) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        final boolean shouldEnablePocketMode =
                preferences.getBoolean(ButtonConstants.FP_WAKEUP_KEY, false) &&
                preferences.getBoolean(ButtonConstants.FP_POCKETMODE_KEY, false);

        if (shouldEnablePocketMode) {
            context.startServiceAsUser(new Intent(context, PocketModeService.class),
                    UserHandle.CURRENT);
        } else {
            context.stopServiceAsUser(new Intent(context, PocketModeService.class),
                    UserHandle.CURRENT);
        }
    }
}
