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

package org.lineageos.settings;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;

import org.lineageos.settings.buttons.ButtonSettingsActivity;
import org.lineageos.settings.buttons.ButtonUtils;

public class BootReceiver extends BroadcastReceiver {

    private static final String TAG = "XiaomiParts";

    @Override
    public void onReceive(Context context, Intent intent) {
        // Disable button settings if needed
        if (!ButtonUtils.hasButtonProcs()) {
            disableComponent(context, ButtonSettingsActivity.class.getName());
        } else {
            enableComponent(context, ButtonSettingsActivity.class.getName());

            // Restore nodes to saved preference values
            ButtonUtils.restoreSavedPreferences(context);

            // Start PocketMode service if applicable
            ButtonUtils.checkPocketModeService(context);
        }
    }

    private void disableComponent(Context context, String component) {
        ComponentName name = new ComponentName(context, component);
        PackageManager pm = context.getPackageManager();
        pm.setComponentEnabledSetting(name,
                PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                PackageManager.DONT_KILL_APP);
    }

    private void enableComponent(Context context, String component) {
        ComponentName name = new ComponentName(context, component);
        PackageManager pm = context.getPackageManager();
        if (pm.getComponentEnabledSetting(name)
                == PackageManager.COMPONENT_ENABLED_STATE_DISABLED) {
            pm.setComponentEnabledSetting(name,
                    PackageManager.COMPONENT_ENABLED_STATE_ENABLED,
                    PackageManager.DONT_KILL_APP);
        }
    }
}
