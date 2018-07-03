
package com.silead.optic;

import java.io.UnsupportedEncodingException;

import android.content.Context;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.silead.internal.IFingerService;
import com.silead.internal.IFingerServiceReceiver;

public class CalibrateManager {
    private static final String TAG = "CalibrateManager";
    private static final boolean DBG = true;

    static public final String FINGERPRINT_TEST_SERVICE = "com.silead.fingerprintService";
    private static final boolean IS_VERIFY_DATA = true;
    private static final byte[] VERIFY_DATA = {
            0x73, 0x6c, 0x66, 0x70
    };

    public static final int OPTIC_CMD_CALIBRATE = 11;
    public static final int OPTIC_CMD_CALIBRATE_STEP = 12;

    public static final int TEST_RESULT_IMAGE_SAVE_FAILED = -3;
    public static final int TEST_RESULT_DATA_IMCOMPLITE = -2;
    public static final int TEST_RESULT_SERVICE_FAILED = -1;
    public static final int TEST_RESULT_OK = 0;
    public static final int TEST_RESULT_BAD_PARAM = 1000;
    public static final int TEST_RESULT_NO_FINGER = 1018;
    public static final int TEST_RESULT_MOVE_TOO_FAST = 1019;
    public static final int TEST_RESULT_ENROLL_SAME_AREA = 1020;
    public static final int TEST_RESULT_ENROLL_QUALITY_FAILED = 1021;
    public static final int TEST_RESULT_ENROLL_COVERAREA_FAILED = 1022;
    public static final int TEST_RESULT_ENROLL_QUALITY_COVERAREA_FAILED = 1023;
    public static final int TEST_RESULT_ENROLL_FAKE_FINGER = 1024;
    public static final int TEST_RESULT_ENROLL_GAIN_IMPROVE_TIMEOUT = 1025;
    public static final int TEST_RESULT_CANCELED = 1030;

    private static final int MSG_TEST_ERROR = 100;
    private static final int MSG_TEST_SUCCESS = 101;
    private static final int MSG_CALIBRATE = 1001;
    private static final int MSG_CALIBRATE_STEP = 1002;

    private static CalibrateManager sInstance = null;

    private Handler mHandler;
    private IBinder mToken = new Binder();

    OpticCmdCallback mOpticCmdCallback;

    public static CalibrateManager getDefault(Context context) {
        if (sInstance == null) {
            sInstance = new CalibrateManager(context);
        }
        return sInstance;
    }

    public CalibrateManager(Context context) {
        mHandler = new MyHandler(context);
    }

    private IFingerService getIFingerService() {
        return IFingerService.Stub.asInterface(ServiceManager.getService(FINGERPRINT_TEST_SERVICE));
    }

    // ***************************************************************//
    public void calibrate(OpticCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        testCmd(OPTIC_CMD_CALIBRATE, data, callback);
    }

    public void calibrateStep(int step, OpticCmdCallback callback) {
        byte[] data = null;
        int offset = 0;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length + 1];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
            offset = VERIFY_DATA.length;
        } else {
            data = new byte[1];
        }

        data[offset++] = (byte) (step & 0xFF);
        testCmd(OPTIC_CMD_CALIBRATE_STEP, data, callback);
    }

    public void testCmd(int cmdId, byte[] param, OpticCmdCallback callback) {
        int ret = -1;
        mOpticCmdCallback = callback;
        try {
            ret = getIFingerService().testCmd(mToken, cmdId, param, mServiceReceiver);
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
        }

        if (ret < 0) {
            onOpticCmdResult(cmdId, null);
        }
    }

    // ***************************************************************//
    private int onCalibrateResult(int cmdId, byte[] result) {
        int err = TEST_RESULT_DATA_IMCOMPLITE;
        int offset = 0;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (TEST_RESULT_OK == err) { // successful
                mHandler.obtainMessage(MSG_TEST_SUCCESS).sendToTarget();
            }
        }
        if (err != TEST_RESULT_OK) {
            mHandler.obtainMessage(MSG_TEST_ERROR, err).sendToTarget();
        }

        return 1;
    }

    private int onCalibrateStepResult(int cmdId, byte[] result) {
        CalibrateStepResult object = CalibrateStepResult.parse(result);
        mHandler.obtainMessage(MSG_CALIBRATE_STEP, object).sendToTarget();
        return 0;
    }

    public int onOpticCmdResult(int cmdId, byte[] result) {
        if (DBG) {
            Log.d(TAG, "onOpticCmdResult: cmdId = " + cmdId + ",result.length=" + (result == null ? 0 : result.length));
        }

        switch (cmdId) {
            case OPTIC_CMD_CALIBRATE: {
                return onCalibrateResult(cmdId, result);
            }
            case OPTIC_CMD_CALIBRATE_STEP: {
                return onCalibrateStepResult(cmdId, result);
            }
        }
        return 0;
    }

    private IFingerServiceReceiver mServiceReceiver = new IFingerServiceReceiver.Stub() {
        @Override
        public int onTestCmd(int cmdId, byte[] result) throws RemoteException {
            return onOpticCmdResult(cmdId, result);
        }
         @Override
        public int onTestError(int cmdId, int err) throws RemoteException {
            mHandler.obtainMessage(MSG_TEST_ERROR, err).sendToTarget();
            return 0;
        }
    };

    public static abstract class OpticCmdCallback {
        public void onError(int err) {
        }
        public void onSuccess() {
        }
        public void onCalibrateStep(int step, int err) {
        }
    };

    private class MyHandler extends Handler {
        private MyHandler(Context context) {
            super(context.getMainLooper());
        }

        private MyHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(android.os.Message msg) {
            if (DBG) {
                Log.d(TAG, "handleMessage: msg.what = " + msg.what + ",mOpticCmdCallback=" + mOpticCmdCallback);
            }

            switch (msg.what) {
                case MSG_TEST_ERROR: {
                    if (mOpticCmdCallback != null) {
                        int ret = ((Integer)msg.obj).intValue();
                        mOpticCmdCallback.onError(ret);
                    }
                    break;
                }
                case MSG_TEST_SUCCESS: {
                    if (mOpticCmdCallback != null) {
                        mOpticCmdCallback.onSuccess();
                    }
                    break;
                }
                case MSG_CALIBRATE_STEP: {
                    if (mOpticCmdCallback != null) {
                        CalibrateStepResult object = (CalibrateStepResult) msg.obj;
                        mOpticCmdCallback.onCalibrateStep(object.getStep(), object.getResult());
                    }
                    break;
                }
            }
        }
    };
}