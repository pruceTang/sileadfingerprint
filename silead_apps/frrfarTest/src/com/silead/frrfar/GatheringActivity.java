
package com.silead.frrfar;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Vibrator;
import android.util.Log;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.silead.manager.FingerManager;
import com.silead.manager.FingerFrrFarEnroll;

import android.content.Context;
import android.view.Gravity;
import android.graphics.PixelFormat;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.MotionEvent;

public class GatheringActivity extends Activity implements View.OnClickListener {
    public static final String EXTRA_KEY_USER_TOKEN = "user_token";
    public static final String EXTRA_KEY_HAND_TOKEN = "hand_token";
    public static final String EXTRA_KEY_FINGER_TOKEN = "finger_token";

    private TextView mEnrollInfoTextView;
    private ImageView mFingerImageView;
    private TextView mImageQulityTextView;
    private TextView mEffectiveAreaTextView;
    private TextView mNumEnrollInfoTextView;
    private NumberProgressBar mSampleProgressBar;
    private Button mEnrollingFinishBtn;
    private Button mEnrollingPreImageBtn;
    private Button mEnrollingNextFingerBtn;
    private Button mEnrollingPlasticFingerBtn;

    private int mSampleCount;
    private String mUser;
    private int mHand;
    private int mFinger;
    private boolean mGatherMode = false;
    // this flag is used to distinguish finger picture is original or after processed by ALG
    private int isOriginalData = 0;

    private int mSampleEnrolled = 0;
    private int mImageQuality = 0;
    private int mEffectiveArea = 0;
    private Bitmap mBitmapImage = null;
    private int mErrorCode = 0;
    private int mTplImageCount = -1;
    private String mCurrentImageFilePath = null;

    private FingerManager mFingerManager;
    private Vibrator mVibrator;

    private static final int ERR_RE_GET_ONE_IMAGE = -100;

    private WindowManager.LayoutParams lp;
    private ImageView mFingerpintImage;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.enrolling_main);

        Intent intent = getIntent();
        if (intent != null && intent.hasExtra(EXTRA_KEY_USER_TOKEN)) {
            mSampleEnrolled = 0;
            mSampleCount = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_SAMPLE_COUNT_NAME, FingerSettingsConst.FP_SETTINGS_SAMPLE_COUNT_DEFAULT);
            mGatherMode = SharedPreferencesData.loadDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_GATHER_FINGER_NAME,
                    FingerSettingsConst.FP_SETTINGS_GATHER_FINGER_DEFAULT);

            mUser = intent.getStringExtra(EXTRA_KEY_USER_TOKEN);
            mHand = intent.getIntExtra(EXTRA_KEY_HAND_TOKEN, FingerSettingsConst.ENROLL_ENV_NEXT_HAND_DEFAULT);
            mFinger = intent.getIntExtra(EXTRA_KEY_FINGER_TOKEN, FingerSettingsConst.ENROLL_ENV_NEXT_FINGER_DEFAULT);
        } else {
            finish();
            return;
        }
        mVibrator = (Vibrator)this.getSystemService(Context.VIBRATOR_SERVICE);
        mFingerManager = FingerManager.getDefault(this);

        mEnrollInfoTextView = (TextView) findViewById(R.id.enrolling_info_msg);
        mFingerImageView = (ImageView) findViewById(R.id.enrolling_finger_imageview);
        mImageQulityTextView = (TextView) findViewById(R.id.enrolling_image_quality_textview);
        mEffectiveAreaTextView = (TextView) findViewById(R.id.enrolling_effective_area_textview);
        mNumEnrollInfoTextView = (TextView) findViewById(R.id.enrolling_num_msg);
        mSampleProgressBar = (NumberProgressBar) findViewById(R.id.enrolling_sample_progressbar);
        mEnrollingFinishBtn = (Button) findViewById(R.id.enrolling_finish_btn);
        mEnrollingPreImageBtn = (Button) findViewById(R.id.enrolling_reenroll_btn);
        mEnrollingNextFingerBtn = (Button) findViewById(R.id.enrolling_next_btn);
        mEnrollingPlasticFingerBtn = (Button) findViewById(R.id.enrolling_plastic_finger_btn);

        if (mSampleProgressBar != null) {
            mSampleProgressBar.setMax(mSampleCount);
            mSampleProgressBar.setProgress(0);
        }
        if (mFingerImageView != null) {
            mFingerImageView.setScaleType(ImageView.ScaleType.FIT_CENTER);
        }

        if (mEnrollingFinishBtn != null) {
            mEnrollingFinishBtn.setOnClickListener(this);
            if (FingerSettingsConst.FP_UI_BACK_BTN_SUPPORT) {
                mEnrollingFinishBtn.setVisibility(View.VISIBLE);
            } else {
                mEnrollingFinishBtn.setVisibility(View.GONE);
            }
        }
        if (mEnrollingPreImageBtn != null) {
            mEnrollingPreImageBtn.setOnClickListener(this);
        }
        if (mEnrollingNextFingerBtn != null) {
            mEnrollingNextFingerBtn.setOnClickListener(this);
        }
        if (mEnrollingPlasticFingerBtn != null) {
            mEnrollingPlasticFingerBtn.setOnTouchListener(new OnTouchListener(){
                @Override
                public boolean onTouch(View v, MotionEvent event) {
                    if (v.getId() == R.id.enrolling_plastic_finger_btn) {
                        if (event.getAction() == MotionEvent.ACTION_DOWN) {
                            if (mGatherMode) {
                                testSendFingerDownMsg();
                            } else {
                                testGatherImageFingerDown();
                            }
                        } else if (event.getAction() == MotionEvent.ACTION_UP) {
                            if (mGatherMode) {
                                testSendFingerUpMsg();
                            } else {
                                testGatherImageFingerUp();
                            }
                        }
                    }
                    return true;
                }
            });
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        initData();
        updateUI();

        //testGetImage();

        initIcon();
        showIcon();
    }

    @Override
    public void onPause() {
        super.onPause();
        hideIcon();

        //testFinish();
        finish();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setNextEnv();
    }

    @Override
    public void onClick(View v) {
        if (v == mEnrollingFinishBtn) {
            finish();
        } else if (v == mEnrollingPreImageBtn) {
            if (mSampleEnrolled > mTplImageCount) {
                mSampleEnrolled--;
                mErrorCode = ERR_RE_GET_ONE_IMAGE;

                String fileName = FingerSettingsConst.getDataFileName(mSampleEnrolled - 1);
                File folder = getDataFile(mUser, mHand, mFinger, false);
                File file = new File(folder, fileName);
                if (file.exists()) {
                    mBitmapImage = BitmapFactory.decodeFile(file.getAbsolutePath(), null);
                }

                updateUI();
            }
        } else if (v == mEnrollingNextFingerBtn) {
            setNextEnv();
            initData();
            updateUI();

            //testGetImage();
        }
    }

    private String getErrorMsg(int err) {
        String errString = "unknow";
        if (err == FingerManager.TEST_RESULT_OK) {
            errString = getString(R.string.enrolling_image_save_succeed, mCurrentImageFilePath);
        } else if (err == FingerManager.TEST_RESULT_IMAGE_SAVE_FAILED) {
            errString = getString(R.string.enrolling_image_save_failed, mCurrentImageFilePath);
        } else if (err == FingerManager.TEST_RESULT_DATA_IMCOMPLITE) {
            errString = getString(R.string.globle_err_data_invalide);
        } else if (err == FingerManager.TEST_RESULT_SERVICE_FAILED) {
            errString = getString(R.string.globle_err_not_found_service);
        } else if (err == FingerManager.TEST_RESULT_BAD_PARAM) {
            errString = getString(R.string.globle_err_bad_param);
        } else if (err == FingerManager.TEST_RESULT_MOVE_TOO_FAST || err == FingerManager.TEST_RESULT_NO_FINGER) {
            errString = getString(R.string.globle_err_move_to_fast);
        } else if (err == FingerManager.TEST_RESULT_ENROLL_SAME_AREA) {
            errString = getString(R.string.globle_err_enroll_same_area);
        } else if (err == FingerManager.TEST_RESULT_ENROLL_QUALITY_FAILED) {
            errString = getString(R.string.globle_err_enroll_quality_fail);
        } else if (err == FingerManager.TEST_RESULT_ENROLL_COVERAREA_FAILED) {
            errString = getString(R.string.globle_err_enroll_coverarea_fail);
        } else if (err == FingerManager.TEST_RESULT_ENROLL_QUALITY_COVERAREA_FAILED) {
            errString = getString(R.string.globle_err_enroll_quality_coverarea_fail);
        } else if (err == FingerManager.TEST_RESULT_ENROLL_FAKE_FINGER) {
            errString = getString(R.string.globle_err_enroll_fake_finger);
        } else if (err == FingerManager.TEST_RESULT_ENROLL_GAIN_IMPROVE_TIMEOUT) {
            errString = getString(R.string.globle_err_enroll_gain_improve_timeout);
        } else if (err == FingerManager.TEST_RESULT_CANCELED) {
            errString = getString(R.string.globle_err_canceled);
        } else if (err == ERR_RE_GET_ONE_IMAGE) {
            errString = getString(R.string.enrolling_image_re_enroll_last_image, mSampleEnrolled + 1);
        } else {
            errString = getString(R.string.unknow);
        }
        return errString;
    }

    private void updateUI() {
        if (mEnrollInfoTextView != null) {
            if (mSampleEnrolled == 0 && (mErrorCode == FingerManager.TEST_RESULT_OK)) {
                String value = mUser;
                int fingers = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                        FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_NAME, FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_DEFAULT);
                String fingerFolder = FingerSettingsConst.getFingerPath(mHand, fingers, mFinger);
                value += fingerFolder;

                mEnrollInfoTextView.setText(getString(R.string.enrolling_begin_enroll_info, value));
            } else {
                mEnrollInfoTextView.setText(getErrorMsg(mErrorCode));
            }
        }
        if (mFingerImageView != null) {
            if (mBitmapImage != null) {
                mFingerImageView.setImageBitmap(mBitmapImage);
            }
        }
        if (mImageQulityTextView != null) {
            mImageQulityTextView.setText(String.valueOf(mImageQuality));
        }
        if (mEffectiveAreaTextView != null) {
            mEffectiveAreaTextView.setText(String.valueOf(mEffectiveArea));
        }
        if (mNumEnrollInfoTextView != null) {
            if (mSampleEnrolled < mSampleCount) {
                mNumEnrollInfoTextView.setText(getString(R.string.enrolling_image_num_title, mSampleEnrolled + 1));
            } else {
                mNumEnrollInfoTextView.setText(getString(R.string.enrolling_image_num_finish_title));
            }
        }
        if (mSampleProgressBar != null) {
            mSampleProgressBar.setProgress(mSampleEnrolled);
        }

        if (mEnrollingPreImageBtn != null) {
            if (mTplImageCount < 0 || mSampleEnrolled <= mTplImageCount) {
                mEnrollingPreImageBtn.setEnabled(false);
            } else {
                mEnrollingPreImageBtn.setEnabled(true);
            }
        }
        if (mEnrollingNextFingerBtn != null) {
            if (mSampleEnrolled == mSampleCount) {
                mEnrollingNextFingerBtn.setEnabled(true);
            } else {
                mEnrollingNextFingerBtn.setEnabled(false);
            }
        }
    }

    private void initData() {
        getDataFile(mUser, mHand, mFinger, true);
        mSampleEnrolled = 0;
        mImageQuality = 0;
        mEffectiveArea = 0;
        mBitmapImage = null;
        mErrorCode = 0;
        mTplImageCount = -1;
    }

    private void testGetImage() {
        if (FingerSettingsConst.LOG_DBG) {
            Log.v(FingerSettingsConst.LOG_TAG, "testGetImage: ");
        }
        if (mFingerManager != null) {
            mFingerManager.testGetImage(0, mTestCmdCallback);
        }
    }

    private void testFinish() {
        if (FingerSettingsConst.LOG_DBG) {
            Log.v(FingerSettingsConst.LOG_TAG, "testFinish: finish");
        }
        if (mFingerManager != null) {
            mFingerManager.testFinish(mTestCmdCallback);
        }
    }

    // Add for sending fingerDown/Up message 20180615 begin
    private void testSendFingerDownMsg() {
        isOriginalData = 1;
        if (FingerSettingsConst.LOG_DBG) {
            Log.v(FingerSettingsConst.LOG_TAG, "testSendFingerDownMsg: ");
        }
        if (mFingerManager != null) {
            mFingerManager.testSendFingerDownMsg(mTestCmdCallback);
        }
    }

    public void testSendFingerUpMsg() {
        isOriginalData = 0;
        if (FingerSettingsConst.LOG_DBG) {
            Log.v(FingerSettingsConst.LOG_TAG, "testSendFingerUpMsg");
        }
        if (mFingerManager != null) {
            mFingerManager.testSendFingerUpMsg(mTestCmdCallback);
        }
    }

    private void testGatherImageFingerDown() {
        isOriginalData = 1;
        if (FingerSettingsConst.LOG_DBG) {
            Log.v(FingerSettingsConst.LOG_TAG, "testGatherImageFingerDown");
        }
        if (mFingerManager != null) {
            mFingerManager.testGatherImageFingerDown(0, mTestCmdCallback);
        }
    }

    private void testGatherImageFingerUp() {
        isOriginalData = 0;
        if (FingerSettingsConst.LOG_DBG) {
            Log.v(FingerSettingsConst.LOG_TAG, "testGatherImageFingerUp");
        }
        if (mFingerManager != null) {
            mFingerManager.testGatherImageFingerUp(0, mTestCmdCallback);
        }
    }
    // Add for sending fingerDown/Up message 20180615 end

    private FingerManager.TestCmdCallback mTestCmdCallback = new FingerManager.TestCmdCallback() {
        @Override
        public void onError(int err) {
            Log.v(FingerSettingsConst.LOG_TAG, "onError: " + err);
            updateData(err);
        }

        @Override
        public void onGetImageSuccess(FingerFrrFarEnroll result) {
            if (FingerSettingsConst.LOG_DBG) {
                Log.v(FingerSettingsConst.LOG_TAG, "onGetImageSuccess: err = " + result.getErrCode() +
                        ",imageQuality=" + result.getImageQuality() + ",effectArea=" + result.getEffectiveArea() +
                        ",index=" + result.getIndex() + ",datalen=" + result.getData().length);
            }
            updateData(result);
        }
    };

    private void updateData(Object result) {
        mVibrator.vibrate(500);
        if (mSampleEnrolled >= mSampleCount) {
            Log.e(FingerSettingsConst.LOG_TAG, "mSampleEnrolled = " + mSampleEnrolled
                    + ", mSampleCount= " + mSampleCount);
            return;
        }

        if (result instanceof FingerFrrFarEnroll) {
            FingerFrrFarEnroll enrollResult = (FingerFrrFarEnroll) result;
            mErrorCode = enrollResult.getErrCode();
            mImageQuality = enrollResult.getImageQuality();
            mEffectiveArea = enrollResult.getEffectiveArea();
            mBitmapImage = BitmapFactory.decodeByteArray(enrollResult.getData(), 0, enrollResult.getData().length);
            if (mErrorCode == FingerManager.TEST_RESULT_OK) {
                if (!enrollResult.isTplImage() && mTplImageCount < 0) {
                    mTplImageCount = mSampleEnrolled;
                    Log.e(FingerSettingsConst.LOG_TAG, "mTplImageCount = " + mTplImageCount);
                }

                if (!saveDataFile(mSampleEnrolled, enrollResult.getData())) {
                    mErrorCode = FingerManager.TEST_RESULT_IMAGE_SAVE_FAILED;
                } else {
                    // because each touch it will upload twice result(Original & Processed)
                    // so the template number not need add every time
                    if (isOriginalData % 2 == 0) {
                        mSampleEnrolled++;
                    }
                }
                isOriginalData++;
            }
        } else {
            mErrorCode = ((Integer)result).intValue();
        }
        updateUI();
    }

    private boolean saveDataFile(int index, byte[] data) {
        boolean saved = false;
        String fileName = FingerSettingsConst.getDataFileName(index);
        /*
        if (isOriginalData == 1) {
            fileName = "org_" + fileName;
        }
        */
        File folder = getDataFile(mUser, mHand, mFinger, false);
        File file = new File(folder, fileName);
        if (file.exists()) {
            file.delete();
        }
        try {
            FileOutputStream fos = new FileOutputStream(file);
            fos.write(data);
            fos.flush();
            fos.close();
            saved = true;
        } catch (FileNotFoundException e) {
        } catch (IOException e) {
        }
        mCurrentImageFilePath = file.getAbsolutePath();
        return saved;
    }

    private File getDataFile(String user, int hand, int finger, boolean init) {
        int fingers = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_NAME, FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_DEFAULT);
        String fingerFolder = FingerSettingsConst.getFingerPath(hand, fingers, finger);

        File dataDir = FingerSettingsConst.getDataSavePath(this);
        String pathPrefix = dataDir.getAbsolutePath();
        if (isOriginalData == 1) {
            pathPrefix = pathPrefix + "_org";
        }
        Log.v(FingerSettingsConst.LOG_TAG, "getDataFile pathPrefix: " + pathPrefix);
        File userDataDir = new File(pathPrefix/*dataDir*/, user);
        if (!userDataDir.exists()) {
            if (!userDataDir.mkdirs()) {
                Log.v(FingerSettingsConst.LOG_TAG, "Cannot make directory: " + userDataDir.getAbsolutePath());
                return null;
            }
        }

        File fingerDataDir = new File(userDataDir, fingerFolder);
        if (fingerDataDir.exists()) {
            if (init) {
                deleteFiles(fingerDataDir);
            }
        } else {
            if (!fingerDataDir.mkdirs()) {
                Log.v(FingerSettingsConst.LOG_TAG, "Cannot make directory: " + fingerDataDir.getAbsolutePath());
                return null;
            }
        }

        return fingerDataDir;
    }

    private void deleteFiles(File oldPath) {
        if (oldPath.isDirectory()) {
            File[] files = oldPath.listFiles();
            if (files != null && files.length > 0) {
                for (File file : files) {
                    deleteFiles(file);
                }
            }
        } else {
            oldPath.delete();
        }
    }

    private void setNextEnv() {
        if (mSampleEnrolled >= mSampleCount) {
            int fingers = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_NAME, FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_DEFAULT);

            boolean lastfinger = FingerSettingsConst.isLastFinger(fingers, mFinger);
            mUser = FingerSettingsConst.getNextUser(mUser, mHand, lastfinger);
            mHand = FingerSettingsConst.getNextHand(mHand, lastfinger);
            mFinger = FingerSettingsConst.getNextFinger(mFinger, lastfinger);

            if (FingerSettingsConst.LOG_DBG) {
                Log.v(FingerSettingsConst.LOG_TAG, "next env: mUser=" + mUser + ",mHand=" + mHand + ",mFinger=" + mFinger);
            }

            SharedPreferencesData.saveDataString(this, FingerSettingsConst.ENROLL_ENV_CONFIG,
                    FingerSettingsConst.ENROLL_ENV_NEXT_USERID_NAME, mUser);
            SharedPreferencesData.saveDataInt(this, FingerSettingsConst.ENROLL_ENV_CONFIG,
                    FingerSettingsConst.ENROLL_ENV_NEXT_HAND_NAME, mHand);
            SharedPreferencesData.saveDataInt(this, FingerSettingsConst.ENROLL_ENV_CONFIG,
                    FingerSettingsConst.ENROLL_ENV_NEXT_FINGER_NAME, mFinger);
        }
    }

    private void initIcon() {
        lp = new WindowManager.LayoutParams();
        lp.width = 190;
        lp.height = 190;
        lp.y = 183;
        lp.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
        lp.format = PixelFormat.TRANSPARENT;
        lp.type = WindowManager.LayoutParams.TYPE_VOLUME_OVERLAY;
        lp.windowAnimations = -1;
        lp.flags = lp.flags
                | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
                | WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH
                | WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED
                | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                | WindowManager.LayoutParams.FLAG_FULLSCREEN;

        mFingerpintImage = new FingerprintTestView(getApplicationContext());
    }

    private void showIcon() {
        WindowManager wm = (WindowManager)getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
        wm.addView(mFingerpintImage, lp);
    }

    private void hideIcon() {
        WindowManager wm = (WindowManager)getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
        wm.removeView(mFingerpintImage);
    }

    public class FingerprintTestView extends ImageView {
        private Context mContext;

        public FingerprintTestView(Context context) {
            this(context, null);
        }

        public FingerprintTestView(Context context, @Nullable AttributeSet attrs) {
            this(context, attrs, 0);
        }

        public FingerprintTestView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
            super(context, attrs, defStyleAttr);
            mContext = context;
            setImageDrawable(context.getDrawable(R.drawable.test));
        }

        @Override
        public boolean onTouchEvent(MotionEvent mv) {
            if (mv.getAction() == MotionEvent.ACTION_DOWN) {
                if (mGatherMode) {
                    testSendFingerDownMsg();
                } else {
                    testGatherImageFingerDown();
                }
            } else if (mv.getAction() == MotionEvent.ACTION_UP) {
                if (mGatherMode) {
                    testSendFingerUpMsg();
                } else {
                    testGatherImageFingerUp();
                }
            }
            return true;
        }
    }
}
