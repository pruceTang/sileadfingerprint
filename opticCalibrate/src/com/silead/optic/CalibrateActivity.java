
package com.silead.optic;

import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.app.Activity;
import android.os.Bundle;

import android.content.Context;
import android.view.Gravity;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.MotionEvent;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.WindowManager;
import android.widget.ImageView;

public class CalibrateActivity extends Activity implements View.OnClickListener {
    public static final String LOG_TAG = "CalibrateActivity";
    private CalibrateManager mCalibrateManager;

    private static final boolean FP_UI_BACK_BTN_SUPPORT = true;

    private Button mBackBtn;
    private Button mCalibrateBtn;
    private Button mCalibrateStep1Btn;
    private Button mCalibrateStep2Btn;
    private Button mCalibrateStep3Btn;
    private Button mCalibrateStep4Btn;
    private Button mCalibrateStep5Btn;
    private TextView mResultTextview;
    private TextView mDesTextview;
    private TextView mTopDesTextview;
    private WindowManager.LayoutParams lp;
    private ImageView mFingerpintImage;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.calibrate_main);

        mResultTextview = (TextView) findViewById(R.id.calibrate_result);

        mDesTextview = (TextView) findViewById(R.id.ui_bottom_des);
        if (mDesTextview != null) {
            //mDesTextview.setVisibility(View.VISIBLE);
            mDesTextview.setText(R.string.calibrate_desc);
        }
        mTopDesTextview = (TextView) findViewById(R.id.ui_top_des);
        if (mTopDesTextview != null) {
            mTopDesTextview.setVisibility(View.VISIBLE);
            mTopDesTextview.setText(R.string.calibrate_desc);
        }

        mBackBtn = (Button) findViewById(R.id.ui_bottom_btn);
        if (mBackBtn != null) {
            mBackBtn.setOnClickListener(this);
            if (!FP_UI_BACK_BTN_SUPPORT) {
                mBackBtn.setVisibility(View.GONE);
            }
        }

        mCalibrateStep1Btn = (Button) findViewById(R.id.calibrate_step1_btn);
        if (mCalibrateStep1Btn != null) {
            mCalibrateStep1Btn.setOnClickListener(this);
        }
        mCalibrateStep2Btn = (Button) findViewById(R.id.calibrate_step2_btn);
        if (mCalibrateStep2Btn != null) {
            mCalibrateStep2Btn.setOnClickListener(this);
        }
        mCalibrateStep3Btn = (Button) findViewById(R.id.calibrate_step3_btn);
        if (mCalibrateStep3Btn != null) {
            mCalibrateStep3Btn.setOnClickListener(this);
        }
        mCalibrateStep4Btn = (Button) findViewById(R.id.calibrate_step4_btn);
        if (mCalibrateStep4Btn != null) {
            mCalibrateStep4Btn.setOnClickListener(this);
        }
        mCalibrateStep5Btn = (Button) findViewById(R.id.calibrate_step5_btn);
        if (mCalibrateStep5Btn != null) {
            mCalibrateStep5Btn.setOnClickListener(this);
        }
        mCalibrateBtn = (Button) findViewById(R.id.calibrate_btn);
        if (mCalibrateBtn != null) {
            mCalibrateBtn.setOnClickListener(this);
            mCalibrateBtn.setVisibility(View.GONE);
        }

        mCalibrateManager = mCalibrateManager.getDefault(this);
    }

    @Override
    public void onResume() {
        super.onResume();
        initIcon();
        showIcon();
    }

    @Override
    public void onPause() {
        super.onPause();
        hideIcon();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    private CalibrateManager.OpticCmdCallback mOpticCmdCallback = new CalibrateManager.OpticCmdCallback() {
        @Override
        public void onError(int err) {
            Log.e(LOG_TAG, "test err: " + err);
            mResultTextview.setText(R.string.calibrate_result_fail);
            mResultTextview.setTextAppearance(CalibrateActivity.this, R.style.TestResultFailStyle);
            updataCalibrateBtn(true);
        }

        @Override
        public void onSuccess() {
            mResultTextview.setText(R.string.calibrate_result_success);
            mResultTextview.setTextAppearance(CalibrateActivity.this, R.style.TestResultPassStyle);
            updataCalibrateBtn(true);
        }

        @Override
        public void onCalibrateStep(int step, int err) {
            Log.e(LOG_TAG, "onCalibrateStep step: " + step + ", err:" + err);
            if (err == CalibrateManager.TEST_RESULT_OK) {
                mResultTextview.setText(R.string.calibrate_result_success);
                mResultTextview.setTextAppearance(CalibrateActivity.this, R.style.TestResultPassStyle);
            } else {
                mResultTextview.setText(R.string.calibrate_result_fail);
                mResultTextview.setTextAppearance(CalibrateActivity.this, R.style.TestResultFailStyle);
            }
            updataCalibrateBtn(true);
        }
    };

    private void updataCalibrateBtn(boolean enable) {
        mCalibrateBtn.setEnabled(enable);
        mCalibrateStep1Btn.setEnabled(enable);
        mCalibrateStep2Btn.setEnabled(enable);
        mCalibrateStep3Btn.setEnabled(enable);
        mCalibrateStep4Btn.setEnabled(enable);
        mCalibrateStep5Btn.setEnabled(enable);
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
        Log.e(LOG_TAG, "initIcon");
        mFingerpintImage = new FingerprintTestView(getApplicationContext());
    }

    private void showIcon() {
        Log.e(LOG_TAG, "showIcon");
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

        public FingerprintTestView(Context context, AttributeSet attrs) {
            this(context, attrs, 0);
        }

        public FingerprintTestView(Context context, AttributeSet attrs, int defStyleAttr) {
            super(context, attrs, defStyleAttr);
            mContext = context;
            setImageDrawable(context.getDrawable(R.drawable.test));
        }

        @Override
        public boolean onTouchEvent(MotionEvent mv) {
            return true;
        }
    }

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            finish();
        } else if (v == mCalibrateBtn) {
            mCalibrateManager.calibrate(mOpticCmdCallback);
            updataCalibrateBtn(false);
        } else if (v == mCalibrateStep1Btn) {
            mCalibrateManager.calibrateStep(1, mOpticCmdCallback);
            updataCalibrateBtn(false);
        } else if (v == mCalibrateStep2Btn) {
            mCalibrateManager.calibrateStep(2, mOpticCmdCallback);
            updataCalibrateBtn(false);
        } else if (v == mCalibrateStep3Btn) {
            mCalibrateManager.calibrateStep(3, mOpticCmdCallback);
            updataCalibrateBtn(false);
        } else if (v == mCalibrateStep4Btn) {
            mCalibrateManager.calibrateStep(4, mOpticCmdCallback);
            updataCalibrateBtn(false);
        } else if (v == mCalibrateStep5Btn) {
            mCalibrateManager.calibrateStep(5, mOpticCmdCallback);
            updataCalibrateBtn(false);
        }
    }
}
