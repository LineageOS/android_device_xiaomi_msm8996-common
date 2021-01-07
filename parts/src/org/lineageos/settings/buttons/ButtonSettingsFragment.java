/*
 * Copyright (C) 2016 The CyanogenMod Project
 *               2017,2019-2020,2022-2023 The LineageOS Project
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

import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.text.TextUtils;

import androidx.preference.Preference;
import androidx.preference.PreferenceFragment;
import androidx.preference.PreferenceManager;
import androidx.preference.SwitchPreferenceCompat;

import org.lineageos.internal.util.FileUtils;
import org.lineageos.settings.R;

public class ButtonSettingsFragment extends PreferenceFragment
        implements Preference.OnPreferenceChangeListener {

    private Handler mHandler = new Handler();

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.button_panel);
    }

    @Override
    public void onResume() {
        super.onResume();
        updatePreferencesBasedOnDependencies();
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        String node = ButtonConstants.sBooleanNodePreferenceMap.get(preference.getKey());
        if (!TextUtils.isEmpty(node) && FileUtils.isFileWritable(node)) {
            Boolean value = (Boolean) newValue;
            FileUtils.writeLine(node, value ? "1" : "0");
        }

        mHandler.post(() -> ButtonUtils.checkPocketModeService(getActivity()));

        return true;
    }

    @Override
    public void addPreferencesFromResource(int preferencesResId) {
        super.addPreferencesFromResource(preferencesResId);
        // Initialize node preferences
        for (String pref : ButtonConstants.sBooleanNodePreferenceMap.keySet()) {
            SwitchPreferenceCompat b = findPreference(pref);
            if (b == null) continue;
            b.setOnPreferenceChangeListener(this);
            String node = ButtonConstants.sBooleanNodePreferenceMap.get(pref);
            if (FileUtils.isFileReadable(node)) {
                String curNodeValue = FileUtils.readOneLine(node);
                b.setChecked(curNodeValue.equals("1"));
            } else {
                b.setEnabled(false);
            }
        }

        // Initialize other preferences whose keys are not associated with nodes
        SwitchPreferenceCompat b = findPreference(ButtonConstants.FP_POCKETMODE_KEY);
        b.setOnPreferenceChangeListener(this);
    }

    private void updatePreferencesBasedOnDependencies() {
        for (String pref : ButtonConstants.sNodeDependencyMap.keySet()) {
            SwitchPreferenceCompat b = findPreference(pref);
            if (b == null) continue;
            String dependencyNode = ButtonConstants.sNodeDependencyMap.get(pref)[0];
            if (FileUtils.isFileReadable(dependencyNode)) {
                String dependencyNodeValue = FileUtils.readOneLine(dependencyNode);
                boolean shouldSetEnabled = dependencyNodeValue.equals(
                        ButtonConstants.sNodeDependencyMap.get(pref)[1]);
                ButtonUtils.updateDependentPreference(getContext(), b, pref, shouldSetEnabled);
            }
        }
    }
}
