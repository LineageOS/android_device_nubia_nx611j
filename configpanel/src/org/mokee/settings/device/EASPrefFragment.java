/*
 * Copyright (C) 2016 The CyanogenMod Project
 *           (C) 2017 The LineageOS Project
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

package org.mokee.settings.device;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Build;
import android.os.SystemProperties;
import androidx.preference.PreferenceFragment;
import androidx.preference.SwitchPreference;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.Preference.OnPreferenceChangeListener;
import androidx.preference.PreferenceCategory;
import androidx.preference.PreferenceManager;
import android.text.TextUtils;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import org.mokee.internal.util.FileUtils;
import org.mokee.internal.util.PackageManagerUtils;

import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class EASPrefFragment extends PreferenceFragment
        implements OnPreferenceChangeListener, Preference.OnPreferenceClickListener {

    ListPreference mPrefMode;

    Preference mLittleCore,mBigCore;

    SharedPreferences preferences;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.eas_pref);
        final ActionBar actionBar = getActivity().getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        preferences = PreferenceManager.getDefaultSharedPreferences(getContext());
        mPrefMode = (ListPreference) findPreference("eas_preformance");
        updateModeValue();
        mPrefMode.setOnPreferenceChangeListener(this);
        mBigCore = findPreference("cpufreq_big");
        mLittleCore = findPreference("cpufreq_little");
        mBigCore.setOnPreferenceClickListener(this);
        mLittleCore.setOnPreferenceClickListener(this);
    }

    public void updateModeValue(){
        String mode = SystemProperties.get("persist.eas.mode", "0");
        mPrefMode.setValue(mode);
        mPrefMode.setSummary(mPrefMode.getEntries()[mPrefMode.findIndexOfValue(mode)]);
    }

    @Override
    public boolean onPreferenceChange(Preference pref, Object newValue) {
        if (pref == mPrefMode) {
            String newMode = (String) newValue;
            if (Integer.valueOf(newMode) < 4) {
                SystemProperties.set("persist.eas.mode", newMode);
                updateModeValue();
                return true;
            }            
        }

        return false;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            getActivity().onBackPressed();
            return true;
        }
        return false;
    }

    @Override
    public boolean onPreferenceClick(Preference preference) {
        if (preference == mBigCore){
            showFreqDialog(true);
            return true;
        }else if (preference == mLittleCore){
            showFreqDialog(false);
            return true;
        }
        return false;
    }

    private void showFreqDialog(final boolean isBigCore) {
        String max = FreqTweaks.getMaxFreq(isBigCore);
        String min = FreqTweaks.getMinFreq(isBigCore);


        final List<String> freqs =  Arrays.asList(isBigCore ? FreqTweaks.getBigAvailableFreq() : FreqTweaks.getLittleAvailableFreq());


        ArrayAdapter<String> adapter = new ArrayAdapter(getContext(),R.layout.item_freq, freqs);
        View view = View.inflate(getContext(), R.layout.layout_cpu_freq, null);
        Spinner minSpinner = view.findViewById(R.id.freq_spinner_min);
        Spinner maxSpinner = view.findViewById(R.id.freq_spinner_max);
        minSpinner.setAdapter(adapter);
        maxSpinner.setAdapter(adapter);
        minSpinner.setSelection(freqs.indexOf(min),true);
        maxSpinner.setSelection(freqs.indexOf(max), true);
        minSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                FreqTweaks.setMinFreq(isBigCore, freqs.get(position));
                preferences.edit().putString("cpu_min_freq" + (isBigCore ? "_big" : "_little"), freqs.get(position)).apply();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
        maxSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                FreqTweaks.setMaxFreq(isBigCore, freqs.get(position));
                preferences.edit().putString("cpu_max_freq" + (isBigCore ? "_big" : "_little"), freqs.get(position)).apply();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
        AlertDialog.Builder builder = new AlertDialog.Builder(getContext());
        builder.setTitle((isBigCore ? "Big" : "Little") + "Core");
        builder.setView(view);
        builder.create().show();
    }

}
