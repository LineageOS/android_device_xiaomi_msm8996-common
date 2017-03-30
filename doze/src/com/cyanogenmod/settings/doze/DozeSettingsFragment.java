/*
 * Copyright (C) 2015 The CyanogenMod Project
 * Copyright (C) 2017 The LineageOS Project
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

package com.cyanogenmod.settings.doze;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.database.ContentObserver;
import android.os.Bundle;
import android.os.Handler;
import android.support.v14.preference.PreferenceFragment;
import android.support.v14.preference.SwitchPreference;
import android.support.v7.preference.Preference;
import android.support.v7.preference.Preference.OnPreferenceChangeListener;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.Switch;

public class DozeSettingsFragment extends PreferenceFragment implements OnPreferenceChangeListener,
        CompoundButton.OnCheckedChangeListener {

    private SharedPreferences mPreferences;

    private SwitchPreference mPickUpPreference;
    private SwitchPreference mHandwavePreference;
    private SwitchPreference mPocketPreference;

    private ContentObserver mDozeObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            super.onChange(selfChange);

            updateSwitches(Utils.isDozeEnabled(getActivity()));
            DozeReceiver.notifyChanged(getActivity());
        }
    };

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.doze_settings);

        // get shared preference
        mPreferences = getActivity().getSharedPreferences("doze_settings", Activity.MODE_PRIVATE);

        if (savedInstanceState == null && !mPreferences.getBoolean("first_help_shown", false)) {
            showHelp();
        }

        mPickUpPreference = (SwitchPreference) findPreference(Utils.GESTURE_PICK_UP_KEY);
        mPickUpPreference.setOnPreferenceChangeListener(this);

        mHandwavePreference = (SwitchPreference) findPreference(Utils.GESTURE_HAND_WAVE_KEY);
        mHandwavePreference.setOnPreferenceChangeListener(this);

        mPocketPreference = (SwitchPreference) findPreference(Utils.GESTURE_POCKET_KEY);
        mPocketPreference.setOnPreferenceChangeListener(this);
    }

    @Override
    public void onResume() {
        super.onResume();
        getActivity().getContentResolver().registerContentObserver(
                Utils.DOZE_ENABLED_URI, false, mDozeObserver);
        updateSwitches(Utils.isDozeEnabled(getActivity()));
    }

    @Override
    public void onPause() {
        super.onPause();
        getActivity().getContentResolver().unregisterContentObserver(mDozeObserver);
    }

    private void updateSwitches(boolean enabled) {
        mPickUpPreference.setEnabled(enabled);
        mHandwavePreference.setEnabled(enabled);
        mPocketPreference.setEnabled(enabled);
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        View switchBar = view.findViewById(R.id.switch_bar);
        Switch switchWidget = (Switch) switchBar.findViewById(android.R.id.switch_widget);
        switchWidget.setChecked(Utils.isDozeEnabled(getActivity()));
        switchWidget.setOnCheckedChangeListener(this);

        switchBar.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                switchWidget.setChecked(!switchWidget.isChecked());
            }
        });
    }


    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        final String key = preference.getKey();
        final boolean value = (Boolean) newValue;
        if (Utils.GESTURE_PICK_UP_KEY.equals(key)) {
            mPickUpPreference.setChecked(value);
        } else if (Utils.GESTURE_HAND_WAVE_KEY.equals(key)) {
            mHandwavePreference.setChecked(value);
        } else if (Utils.GESTURE_POCKET_KEY.equals(key)) {
            mPocketPreference.setChecked(value);
        } else {
            return false;
        }

        Utils.startService(getActivity());
        return true;
    }

    @Override
    public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
        Utils.enableDoze(b, getActivity());
    }

    private static class HelpDialogFragment extends DialogFragment {
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            return new AlertDialog.Builder(getActivity())
                    .setTitle(R.string.doze_settings_help_title)
                    .setMessage(R.string.doze_settings_help_text)
                    .setNegativeButton(R.string.dialog_ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            dialog.cancel();
                        }
                    })
                    .create();
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            getActivity().getSharedPreferences("doze_settings", Activity.MODE_PRIVATE)
                    .edit()
                    .putBoolean("first_help_shown", true)
                    .commit();
        }
    }

    private void showHelp() {
        HelpDialogFragment fragment = new HelpDialogFragment();
        fragment.show(getFragmentManager(), "help_dialog");
    }
}
