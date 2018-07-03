
package com.silead.frrfar;

import java.io.File;
import java.util.HashSet;
import java.util.Set;

import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.MultiSelectListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.view.View;
import android.widget.Button;

public class FingerTestSettingActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener, View.OnClickListener {
    private Runnable mStatusUpdateRunnable;

    private EditTextPreference mEnrollSampleCountEdtPref;
    private Preference mEnrollDataPathPref;
    private MultiSelectListPreference mEnrollFingersMulSelectPref;
    private CheckBoxPreference mTestFrrFarImgResultCBoxPref;
    private CheckBoxPreference mTestFarRunFrrFirstCBoxPref;
    private CheckBoxPreference mTestGatherFingerCBoxPref;

    private Button mBackBtn;

    private static final int MSG_UPDATE_SAMPLE_COUNT_SUMMARY  = 1;
    private static final int MSG_UPDATE_SAMPLE_COUNT_VALUE    = 2;
    private static final int MSG_UPDATE_ENROLL_FINGER_SUMMARY = 3;
    private static final int MSG_UPDATE_ENROLL_FINGER_VALUE   = 4;
    private static final int MSG_UPDATE_TEST_FARRFAR_IMG_RESULT_VALUE = 5;
    private static final int MSG_UPDATE_TEST_FAR_RUN_FRR_FIRST_VALUE  = 6;
    private static final int MSG_UPDATE_TEST_GATHER_FINGER_VALUE      = 7;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.finger_settings);
        if (FingerSettingsConst.FP_UI_BACK_BTN_SUPPORT) {
            setContentView(R.layout.ui_include_preference_main);
        }

        mEnrollSampleCountEdtPref = (EditTextPreference) findPreference("finger_settings_enroll_sample_count");
        mEnrollDataPathPref = (Preference) findPreference("finger_settings_enroll_data_path");
        mEnrollFingersMulSelectPref = (MultiSelectListPreference) findPreference("finger_settings_enroll_finger");
        mTestFrrFarImgResultCBoxPref = (CheckBoxPreference) findPreference("finger_settings_test_farfrr_img_result");
        mTestFarRunFrrFirstCBoxPref = (CheckBoxPreference) findPreference("finger_settings_test_far_run_frr_first");
        mTestGatherFingerCBoxPref = (CheckBoxPreference) findPreference("finger_settings_test_gather_finger");

        mBackBtn = (Button) findViewById(R.id.ui_bottom_btn);

        mStatusUpdateRunnable = new Runnable() {
            public void run() {
                if (mEnrollSampleCountEdtPref != null) {
                    updatePreferenceStatus(mEnrollSampleCountEdtPref, MSG_UPDATE_SAMPLE_COUNT_SUMMARY);
                    updatePreferenceStatus(mEnrollSampleCountEdtPref, MSG_UPDATE_SAMPLE_COUNT_VALUE);
                }
                if (mEnrollFingersMulSelectPref != null) {
                    updatePreferenceStatus(mEnrollFingersMulSelectPref, MSG_UPDATE_ENROLL_FINGER_SUMMARY);
                    updatePreferenceStatus(mEnrollFingersMulSelectPref, MSG_UPDATE_ENROLL_FINGER_VALUE);
                }
                if (mTestFrrFarImgResultCBoxPref != null) {
                    updatePreferenceStatus(mTestFrrFarImgResultCBoxPref, MSG_UPDATE_TEST_FARRFAR_IMG_RESULT_VALUE);
                }
                if (mTestFarRunFrrFirstCBoxPref != null) {
                    updatePreferenceStatus(mTestFarRunFrrFirstCBoxPref, MSG_UPDATE_TEST_FAR_RUN_FRR_FIRST_VALUE);
                }
                if (mTestGatherFingerCBoxPref != null) {
                    updatePreferenceStatus(mTestGatherFingerCBoxPref, MSG_UPDATE_TEST_GATHER_FINGER_VALUE);
                }
            }
        };

        if (mEnrollDataPathPref != null) {
            File dataDir = FingerSettingsConst.getDataSavePath(this);
            mEnrollDataPathPref.setSummary(dataDir.getAbsolutePath());
        }
        if (mEnrollSampleCountEdtPref != null) {
            mEnrollSampleCountEdtPref.setOnPreferenceChangeListener(this);
        }
        if (mEnrollFingersMulSelectPref != null) {
            mEnrollFingersMulSelectPref.setOnPreferenceChangeListener(this);
        }
        if (mTestFrrFarImgResultCBoxPref != null) {
            mTestFrrFarImgResultCBoxPref.setOnPreferenceChangeListener(this);
        }
        if (mTestFarRunFrrFirstCBoxPref != null) {
            mTestFarRunFrrFirstCBoxPref.setOnPreferenceChangeListener(this);
        }
        if (mTestGatherFingerCBoxPref != null) {
            mTestGatherFingerCBoxPref.setOnPreferenceChangeListener(this);
        }
        if (mBackBtn != null) {
            mBackBtn.setOnClickListener(this);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        updateStatus();
    }

    private void updateStatus() {
        new Thread(mStatusUpdateRunnable).start();
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_UPDATE_SAMPLE_COUNT_SUMMARY: {
                    if (mEnrollSampleCountEdtPref != null) {
                        mEnrollSampleCountEdtPref.setSummary((String) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_SAMPLE_COUNT_VALUE: {
                    if (mEnrollSampleCountEdtPref != null) {
                        mEnrollSampleCountEdtPref.setText((String) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_ENROLL_FINGER_SUMMARY: {
                    if (mEnrollFingersMulSelectPref != null) {
                        mEnrollFingersMulSelectPref.setSummary((CharSequence) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_ENROLL_FINGER_VALUE: {
                    if (mEnrollFingersMulSelectPref != null) {
                        mEnrollFingersMulSelectPref.setValues((Set<String>) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_TEST_FARRFAR_IMG_RESULT_VALUE: {
                    mTestFrrFarImgResultCBoxPref.setChecked(((Boolean)msg.obj).booleanValue());
                    break;
                }
                case MSG_UPDATE_TEST_FAR_RUN_FRR_FIRST_VALUE: {
                    mTestFarRunFrrFirstCBoxPref.setChecked(((Boolean)msg.obj).booleanValue());
                    break;
                }
                case MSG_UPDATE_TEST_GATHER_FINGER_VALUE: {
                    mTestGatherFingerCBoxPref.setChecked(((Boolean)msg.obj).booleanValue());
                    break;
                }
            }
        }
    };

    private void updatePreferenceStatus(Preference preference, int msg) {
        if (preference == null) {
            return;
        }

        if (preference instanceof EditTextPreference) {
            String name = null;
            int value = FingerSettingsConst.FP_SETTINGS_SAMPLE_COUNT_DEFAULT;
            if (msg == MSG_UPDATE_SAMPLE_COUNT_SUMMARY || msg == MSG_UPDATE_SAMPLE_COUNT_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_SAMPLE_COUNT_NAME;
                value = FingerSettingsConst.FP_SETTINGS_SAMPLE_COUNT_DEFAULT;
            }
            if (!TextUtils.isEmpty(name)) {
                value = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG, name, value);
            }
            mHandler.sendMessage(mHandler.obtainMessage(msg, String.valueOf(value)));
        } else if (preference instanceof MultiSelectListPreference) {
            String name = null;
            int value = FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_DEFAULT;
            if (msg == MSG_UPDATE_ENROLL_FINGER_SUMMARY || msg == MSG_UPDATE_ENROLL_FINGER_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_NAME;
                value = FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_DEFAULT;
            }
            if (!TextUtils.isEmpty(name)) {
                value = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG, name, value);
            }
            if (msg == MSG_UPDATE_ENROLL_FINGER_SUMMARY) {
                CharSequence[] summarys = ((MultiSelectListPreference) preference).getEntries();
                String summary = "";
                if (summarys != null) {
                    for (int i = 0; i < summarys.length; i++) {
                        if ((value & (1 << i)) != 0) {
                            summary += summarys[i] + " ";
                        }
                    }
                }
                mHandler.sendMessage(mHandler.obtainMessage(msg, summary));
            } else {
                CharSequence[] EntryValues = ((MultiSelectListPreference) preference).getEntryValues();
                Set<String> values = new HashSet<String>();
                if (EntryValues != null) {
                    for (int i = 0; i < EntryValues.length; i++) {
                        if ((value & (1 << i)) != 0) {
                            values.add(EntryValues[i].toString());
                        }
                    }
                }
                mHandler.sendMessage(mHandler.obtainMessage(msg, values));
            }
        } else if (preference instanceof CheckBoxPreference) {
            String name = null;
            boolean value = FingerSettingsConst.FP_SETTINGS_FRRFAR_IMG_RESULT_DEFAULT;
            if (msg == MSG_UPDATE_TEST_FARRFAR_IMG_RESULT_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_FRRFAR_IMG_RESULT_NAME;
                value = FingerSettingsConst.FP_SETTINGS_FRRFAR_IMG_RESULT_DEFAULT;
            } else if (msg == MSG_UPDATE_TEST_FAR_RUN_FRR_FIRST_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_FAR_RUN_FRR_FIRST_NAME;
                value = FingerSettingsConst.FP_SETTINGS_FAR_RUN_FRR_FIRST_DEFAULT;
            } else if (msg == MSG_UPDATE_TEST_GATHER_FINGER_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_GATHER_FINGER_NAME;
                value = FingerSettingsConst.FP_SETTINGS_GATHER_FINGER_DEFAULT;
            }
            if (!TextUtils.isEmpty(name)) {
                value = SharedPreferencesData.loadDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG, name, value);
            }
            mHandler.sendMessage(mHandler.obtainMessage(msg, value));
        }
    }

    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (preference == mEnrollSampleCountEdtPref) {
            String value = (String) newValue;
            if (!TextUtils.isEmpty(value) && Integer.valueOf(value) > 0) {
                SharedPreferencesData.saveDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                        FingerSettingsConst.FP_SETTINGS_SAMPLE_COUNT_NAME, Integer.valueOf(value));
            }
            updatePreferenceStatus(mEnrollSampleCountEdtPref, MSG_UPDATE_SAMPLE_COUNT_SUMMARY);
            updatePreferenceStatus(mEnrollSampleCountEdtPref, MSG_UPDATE_SAMPLE_COUNT_VALUE);
            return true;
        } else if (preference == mEnrollFingersMulSelectPref) {
            int value = 0;
            CharSequence[] EntryValues = ((MultiSelectListPreference) preference).getEntryValues();
            Set<String> values = (Set<String>) newValue;
            if (values.size() > 0) {
                for (int i = 0; i < EntryValues.length; i++) {
                    if (values.contains(EntryValues[i].toString())) {
                        value |= (1 << i);
                    }
                }
            }
            if (value != 0) {
                SharedPreferencesData.saveDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                        FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_NAME, value);
                SharedPreferencesData.saveDataInt(this, FingerSettingsConst.ENROLL_ENV_CONFIG,
                        FingerSettingsConst.ENROLL_ENV_NEXT_FINGER_NAME, FingerSettingsConst.ENROLL_ENV_NEXT_FINGER_DEFAULT);
            }

            updatePreferenceStatus(mEnrollFingersMulSelectPref, MSG_UPDATE_ENROLL_FINGER_SUMMARY);
            updatePreferenceStatus(mEnrollFingersMulSelectPref, MSG_UPDATE_ENROLL_FINGER_VALUE);

            return true;
        } else if (preference == mTestFrrFarImgResultCBoxPref) {
            boolean value = ((Boolean)newValue).booleanValue();
            SharedPreferencesData.saveDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_FRRFAR_IMG_RESULT_NAME, value);
            updatePreferenceStatus(mTestFrrFarImgResultCBoxPref, MSG_UPDATE_TEST_FARRFAR_IMG_RESULT_VALUE);
        } else if (preference == mTestFarRunFrrFirstCBoxPref) {
            boolean value = ((Boolean)newValue).booleanValue();
            SharedPreferencesData.saveDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_FAR_RUN_FRR_FIRST_NAME, value);
            updatePreferenceStatus(mTestFarRunFrrFirstCBoxPref, MSG_UPDATE_TEST_FAR_RUN_FRR_FIRST_VALUE);
        } else if (preference == mTestGatherFingerCBoxPref) {
            boolean value = ((Boolean)newValue).booleanValue();
            SharedPreferencesData.saveDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_GATHER_FINGER_NAME, value);
            updatePreferenceStatus(mTestGatherFingerCBoxPref, MSG_UPDATE_TEST_GATHER_FINGER_VALUE);
        }
        return true;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        return false;
    }

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            finish();
        }
    }
}
