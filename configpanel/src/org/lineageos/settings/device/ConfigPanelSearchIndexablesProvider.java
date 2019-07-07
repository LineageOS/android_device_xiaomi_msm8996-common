/*
 * Copyright (C) 2016 The CyanogenMod Project
 *           (C) 2017-2018 The LineageOS Project
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

import android.content.Context;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.provider.SearchIndexableResource;
import android.provider.SearchIndexablesProvider;

import static android.provider.SearchIndexablesContract.COLUMN_INDEX_NON_INDEXABLE_KEYS_KEY_VALUE;
import static android.provider.SearchIndexablesContract.COLUMN_INDEX_XML_RES_CLASS_NAME;
import static android.provider.SearchIndexablesContract.COLUMN_INDEX_XML_RES_ICON_RESID;
import static android.provider.SearchIndexablesContract.COLUMN_INDEX_XML_RES_INTENT_ACTION;
import static android.provider.SearchIndexablesContract.COLUMN_INDEX_XML_RES_INTENT_TARGET_CLASS;
import static android.provider.SearchIndexablesContract.COLUMN_INDEX_XML_RES_INTENT_TARGET_PACKAGE;
import static android.provider.SearchIndexablesContract.COLUMN_INDEX_XML_RES_RANK;
import static android.provider.SearchIndexablesContract.COLUMN_INDEX_XML_RES_RESID;
import static android.provider.SearchIndexablesContract.INDEXABLES_RAW_COLUMNS;
import static android.provider.SearchIndexablesContract.INDEXABLES_XML_RES_COLUMNS;
import static android.provider.SearchIndexablesContract.NON_INDEXABLES_KEYS_COLUMNS;

import org.lineageos.internal.util.FileUtils;
import org.lineageos.internal.util.PackageManagerUtils;

import java.util.ArrayList;
import java.util.List;

public class ConfigPanelSearchIndexablesProvider extends SearchIndexablesProvider {
    private static final String TAG = "ConfigPanelSearchIndexablesProvider";

    public static final int SEARCH_IDX_BUTTON_PANEL = 0;

    private static SearchIndexableResource[] INDEXABLE_RES = new SearchIndexableResource[]{
            new SearchIndexableResource(1, R.xml.button_panel,
                    ButtonSettingsActivity.class.getName(),
                    R.drawable.ic_settings_additional_buttons),
    };

    @Override
    public boolean onCreate() {
        return true;
    }

    @Override
    public Cursor queryXmlResources(String[] projection) {
        MatrixCursor cursor = new MatrixCursor(INDEXABLES_XML_RES_COLUMNS);
        if (BootCompletedReceiver.hasButtonProcs() /* show button panel */) {
            cursor.addRow(generateResourceRef(INDEXABLE_RES[SEARCH_IDX_BUTTON_PANEL]));
        }
        return cursor;
    }

    private static Object[] generateResourceRef(SearchIndexableResource sir) {
        final Object[] ref = new Object[INDEXABLES_XML_RES_COLUMNS.length];
        ref[COLUMN_INDEX_XML_RES_RANK] = sir.rank;
        ref[COLUMN_INDEX_XML_RES_RESID] = sir.xmlResId;
        ref[COLUMN_INDEX_XML_RES_CLASS_NAME] = null;
        ref[COLUMN_INDEX_XML_RES_ICON_RESID] = sir.iconResId;
        ref[COLUMN_INDEX_XML_RES_INTENT_ACTION] = "com.android.settings.action.EXTRA_SETTINGS";
        ref[COLUMN_INDEX_XML_RES_INTENT_TARGET_PACKAGE] = "org.lineageos.settings.device";
        ref[COLUMN_INDEX_XML_RES_INTENT_TARGET_CLASS] = sir.className;
        return ref;
    }

    private List<String> getNonIndexableKeys(Context context) {
        List<String> keys = new ArrayList<>();
        if (!PackageManagerUtils.isAppInstalled(context, "org.lineageos.pocketmode")) {
            keys.add(Constants.FP_POCKETMODE_KEY);
        }
        if (!FileUtils.fileExists(Constants.FP_HOME_KEY_NODE) &&
                !FileUtils.fileExists(Constants.FP_WAKEUP_NODE)) {
            keys.add(Constants.FP_HOME_KEY);
            keys.add(Constants.FP_WAKEUP_KEY);
        }
        return keys;
    }

    @Override
    public Cursor queryRawData(String[] projection) {
        MatrixCursor cursor = new MatrixCursor(INDEXABLES_RAW_COLUMNS);
        return cursor;
    }

    @Override
    public Cursor queryNonIndexableKeys(String[] projection) {
        MatrixCursor cursor = new MatrixCursor(NON_INDEXABLES_KEYS_COLUMNS);
        final Context context = getContext();

        List<String> nonIndexableKeys = getNonIndexableKeys(context);
        for (String nik : nonIndexableKeys) {
            final Object[] ref = new Object[NON_INDEXABLES_KEYS_COLUMNS.length];
            ref[COLUMN_INDEX_NON_INDEXABLE_KEYS_KEY_VALUE] = nik;
            cursor.addRow(ref);
        }
        return cursor;
    }
}
