
package com.silead.manager;

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

public class FingerManager {
    private static final String TAG = "FingerManager";
    private static final boolean DBG = true;

    static public final String FINGERPRINT_TEST_SERVICE = "com.silead.fingerprintService";
    private static final boolean IS_VERIFY_DATA = true;
    private static final byte[] VERIFY_DATA = {
            0x73, 0x6c, 0x66, 0x70
    };

    public static final int TEST_CMD_SPI = 0;
    public static final int TEST_CMD_TEST_RESET_PIN = 1;
    public static final int TEST_CMD_DEAD_PIXEL = 2;
    public static final int TEST_CMD_GET_VERSION = 3;
    public static final int TEST_CMD_GET_IMAGE = 4;
    public static final int TEST_CMD_WAIT_FINGER_UP = 5;
    public static final int TEST_CMD_FRR_FAR_SEND_IMAGE = 6;
    public static final int TEST_CMD_FRR_FAR_SEND_IMAGE_NEXT_FINGER = 7;
    public static final int TEST_CMD_IMAGE_FINISH    = 8;
    public static final int TEST_CMD_TEST_SPEED      = 9;
    public static final int TEST_CMD_TEST_FINISH     = 10;
    public static final int TEST_CMD_OPTIC_CALIBRATE = 11;
    public static final int TEST_CMD_OPTIC_CALIBRATE_STEP = 12;
    // Add for send fingerDown/Up message 20180615 begin
    public static final int TEST_CMD_SEND_FINGER_DOWN = 13;
    public static final int TEST_CMD_SEND_FINGER_UP   = 14;
    public static final int TEST_CMD_GATHER_IMAGE_FINGER_DOWN = 15;
    public static final int TEST_CMD_GATHER_IMAGE_FINGER_UP   = 16;
    // Add for send fingerDown/Up message 20180615 end

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
    private static final int MSG_TEST_CMD_DEAD_PIXEL = 1002;
    private static final int MSG_TEST_CMD_GET_VERSION = 1003;
    private static final int MSG_TEST_CMD_GET_IMAGE = 1004;
    private static final int MSG_TEST_CMD_WAIT_FINGER_UP = 1005;
    private static final int MSG_TEST_CMD_FRR_FAR_SEND_IMAGE = 1007;
    private static final int MSG_TEST_CMD_FRR_FAR_SEND_IMAGE_NEXT_FINGER = 1008;
    private static final int MSG_TEST_CMD_SPEED = 1009;
    // Add for send fingerDown/Up message 20180615 begin
    private static final int MSG_TEST_CMD_GATHER_IMAGE = 1010;
    // Add for send fingerDown/Up message 20180615 end

    private Object mResultObj;
    private Object mSyncObj = new Object();

    private static FingerManager sInstance = null;

    private Handler mHandler;
    private IBinder mToken = new Binder();

    TestCmdCallback mTestCmdCallback;

    public static FingerManager getDefault(Context context) {
        if (sInstance == null) {
            sInstance = new FingerManager(context);
        }
        return sInstance;
    }

    public FingerManager(Context context) {
        mHandler = new MyHandler(context);
    }

    private IFingerService getIFingerService() {
        return IFingerService.Stub.asInterface(ServiceManager.getService(FINGERPRINT_TEST_SERVICE));
    }

    // ***************************************************************//
    public void getAllVerInfo(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        testCmd(TEST_CMD_GET_VERSION, data, callback);
    }

    public String testSpi(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }

        mResultObj = null;
        testCmd(TEST_CMD_SPI, data, callback);
        synchronized (mSyncObj) {
            while (mResultObj == null) {
                try {
                    mSyncObj.wait();
                } catch (InterruptedException e) {
                }
            }
        }
        return (String)mResultObj;
    }

    public void testResetPin(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        testCmd(TEST_CMD_TEST_RESET_PIN, data, callback);
    }

    public void testGetImage(int index, TestCmdCallback callback) {
        byte[] data = null;
        int offset = 0;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length + 1];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
            offset = VERIFY_DATA.length;
        } else {
            data = new byte[1];
        }

        data[offset++] = (byte) (index & 0xFF);
        testCmd(TEST_CMD_GET_IMAGE, data, callback);
    }

    public void testWaitFingerUp(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        testCmd(TEST_CMD_WAIT_FINGER_UP, data, callback);
    }

    public void testSendImage(int index, int frr, byte[] buffer, TestCmdCallback callback) {
        byte[] data = null;
        int offset = 0;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length + buffer.length + 5];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
            offset = VERIFY_DATA.length;
        } else {
            data = new byte[buffer.length + 5];
        }

        data[offset++] = (byte) ((index >> 24) & 0xFF);
        data[offset++] = (byte) ((index >> 16) & 0xFF);
        data[offset++] = (byte) ((index >> 8) & 0xFF);
        data[offset++] = (byte) (index & 0xFF);
        data[offset++] = (byte) (frr & 0xFF);

        System.arraycopy(buffer, 0, data, offset, buffer.length);
        testCmd(TEST_CMD_FRR_FAR_SEND_IMAGE, data, callback);
    }

    public void testSendImageNextFinger(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        testCmd(TEST_CMD_FRR_FAR_SEND_IMAGE_NEXT_FINGER, data, callback);
    }

    public void testImageFinish(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        testCmd(TEST_CMD_IMAGE_FINISH, data, callback);
    }

    public void testDeadPixel(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        testCmd(TEST_CMD_DEAD_PIXEL, data, callback);
    }

    public void testSpeed(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        testCmd(TEST_CMD_TEST_SPEED, data, callback);
    }

    public void testFinish(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        testCmd(TEST_CMD_TEST_FINISH, data, callback);
    }

    // Add for send fingerDown/Up message 20180615 begin
    public void testSendFingerDownMsg(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        testCmd(TEST_CMD_SEND_FINGER_DOWN, data, callback);
    }

    public void testSendFingerUpMsg(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        testCmd(TEST_CMD_SEND_FINGER_UP, data, callback);
    }

    public void testGatherImageFingerDown(int index, TestCmdCallback callback) {
        byte[] data = null;
        int offset = 0;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length + 1];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
            offset = VERIFY_DATA.length;
        } else {
            data = new byte[1];
        }

        data[offset++] = (byte) (index & 0xFF);
        testCmd(TEST_CMD_GATHER_IMAGE_FINGER_DOWN, data, callback);
    }

    public void testGatherImageFingerUp(int index, TestCmdCallback callback) {
        byte[] data = null;
        int offset = 0;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length + 1];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
            offset = VERIFY_DATA.length;
        } else {
            data = new byte[1];
        }

        data[offset++] = (byte) (index & 0xFF);
        testCmd(TEST_CMD_GATHER_IMAGE_FINGER_UP, data, callback);
    }
    // Add for send fingerDown/Up message 20180615 end

    public void testCmd(int cmdId, byte[] param, TestCmdCallback callback) {
        int ret = -1;
        mTestCmdCallback = callback;
        try {
            ret = getIFingerService().testCmd(mToken, cmdId, param, mServiceReceiver);
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
        }

        if (ret < 0) {
            onTestCmdResult(-1, cmdId, null);
        }
    }

    // ***************************************************************//
    private String getDefaultVersion() {
        return "unknow";
    }

    private String getVersionValue(byte[] result, int offset) {
        if (result != null && result.length > offset && result.length >= (0xFF & result[offset]) + offset + 1) {
            try {
                return new String(result, offset + 1, (0xFF & result[offset]), "UTF-8");
            } catch (UnsupportedEncodingException e) {
            }
        }
        return null;
    }

    private int onVersionResult(long devId, int cmdId, byte[] result) {
        String[] strVersion = {
                null, null, null, null
        };
        int count = 4;
        int i = 0;
        int err = 0;
        int offset = 0;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (TEST_RESULT_OK == err) { // successful
                for (i = 0; i < count; i++) {
                    strVersion[i] = getVersionValue(result, offset);
                    if (strVersion[i] == null) {
                        break;
                    }
                    offset += (0xFF & result[offset]) + 1;
                }
            }
        }

        for (; i < count; i++) {
            if (strVersion[i] == null) {
                strVersion[i] = getDefaultVersion();
            }
        }

        mHandler.obtainMessage(MSG_TEST_CMD_GET_VERSION, new FingerVersion(strVersion[0], strVersion[1],
                strVersion[2], strVersion[3])).sendToTarget();
        return 1;
    }

    private String getDefaultChipId() {
        return "unknow";
    }

    private int onSpiTestResult(long devId, int cmdId, byte[] result) {
        String strChipId = getDefaultChipId();
        int err = 0;
        int offset = 0;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (TEST_RESULT_OK == err) { // successful
                if ((0xFF & result[offset]) + 2 <= result.length) {
                    try {
                        strChipId = new String(result, offset + 1, (0xFF & result[offset]), "UTF-8");
                    } catch (UnsupportedEncodingException e) {
                    }
                }
            }
        }
        mResultObj = strChipId;
        synchronized (mSyncObj) {
            mSyncObj.notifyAll();
        }
        return 1;
    }

    private int onResetPinTestResult(long devId, int cmdId, byte[] result) {
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

    private int onGetImageResult(long devId, int cmdId, byte[] result) {
        int err = TEST_RESULT_DATA_IMCOMPLITE;
        int offset = 0;
        int imageQuality;
        int effectiveArea;
        int index;
        int istpl = 1;
        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (result.length >= offset + 4) {
                int dataLen = (0xFF & result[offset++]) << 24;
                dataLen |= (0xFF & result[offset++]) << 16;
                dataLen |= (0xFF & result[offset++]) << 8;
                dataLen |= (0xFF & result[offset++]);

                if (dataLen > 4 && result.length >= offset + dataLen) {
                    imageQuality = (0xFF & result[offset++]);
                    effectiveArea = (0xFF & result[offset++]);
                    index = (0xFF & result[offset++]);
                    istpl = (0xFF & result[offset++]);
                    mHandler.obtainMessage(MSG_TEST_CMD_GET_IMAGE, new FingerFrrFarEnroll(err, imageQuality,
                            effectiveArea, index, istpl, result, offset, dataLen - 4)).sendToTarget();

                    return 0;
                }
            }
            if (err == TEST_RESULT_OK) {
                err = TEST_RESULT_DATA_IMCOMPLITE;
            }
        }

        if (err != TEST_RESULT_OK) {
            mHandler.obtainMessage(MSG_TEST_ERROR, err).sendToTarget();
        }
        return 0;
    }

    // Add for send fingerDown/Up message 20180615 begin
    private int onGatherImageResult(long devId, int cmdId, byte[] result) {
        int err = TEST_RESULT_DATA_IMCOMPLITE;
        int offset = 0;
        int imageQuality;
        int effectiveArea;
        int index;
        int istpl = 1;
        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (result.length >= offset + 4) {
                int dataLen = (0xFF & result[offset++]) << 24;
                dataLen |= (0xFF & result[offset++]) << 16;
                dataLen |= (0xFF & result[offset++]) << 8;
                dataLen |= (0xFF & result[offset++]);

                if (dataLen > 4 && result.length >= offset + dataLen) {
                    imageQuality = (0xFF & result[offset++]);
                    effectiveArea = (0xFF & result[offset++]);
                    index = (0xFF & result[offset++]);
                    istpl = (0xFF & result[offset++]);
                    mHandler.obtainMessage(MSG_TEST_CMD_GATHER_IMAGE, new FingerFrrFarEnroll(err, imageQuality,
                            effectiveArea, index, istpl, result, offset, dataLen - 4)).sendToTarget();

                    return 0;
                }
            }
            if (err == TEST_RESULT_OK) {
                err = TEST_RESULT_DATA_IMCOMPLITE;
            }
        }

        if (err != TEST_RESULT_OK) {
            mHandler.obtainMessage(MSG_TEST_ERROR, err).sendToTarget();
        }
        return 0;
    }
    // Add for send fingerDown/Up message 20180615 end

    private int onWaitFingerUpResult(long devId, int cmdId, byte[] result) {
        int err = 0;
        int offset = 0;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);
        }
        mHandler.obtainMessage(MSG_TEST_CMD_WAIT_FINGER_UP, err).sendToTarget();
        return 0;
    }

    private int onSendImageResult(long devId, int cmdId, byte[] result) {
        int err = TEST_RESULT_DATA_IMCOMPLITE;
        int offset = 0;
        int isenroll;
        int value;
        int index;
        int time1 = 0;
        int time2 = 0;
        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (err == TEST_RESULT_OK) {
                if (result.length >= offset + 6) {
                    index = (0xFF & result[offset++]) << 24;
                    index |= (0xFF & result[offset++]) << 16;
                    index |= (0xFF & result[offset++]) << 8;
                    index |= (0xFF & result[offset++]);
                    isenroll = (0xFF & result[offset++]);
                    value = (0xFF & result[offset++]);
                    if (result.length >= offset + 4) {
                        time1 = (0xFF & result[offset++]) << 24;
                        time1 |= (0xFF & result[offset++]) << 16;
                        time1 |= (0xFF & result[offset++]) << 8;
                        time1 |= (0xFF & result[offset++]);
                    }
                    if (result.length >= offset + 4) {
                        time2 = (0xFF & result[offset++]) << 24;
                        time2 |= (0xFF & result[offset++]) << 16;
                        time2 |= (0xFF & result[offset++]) << 8;
                        time2 |= (0xFF & result[offset++]);
                    }

                    mHandler.obtainMessage(MSG_TEST_CMD_FRR_FAR_SEND_IMAGE, new FingerFrrFarImageResult(isenroll,
                            value, index, time1, time2)).sendToTarget();
                } else {
                    err = TEST_RESULT_DATA_IMCOMPLITE;
                }
            }
        }

        if (err != TEST_RESULT_OK) {
            mHandler.obtainMessage(MSG_TEST_ERROR, err).sendToTarget();
        }
        return 0;
    }

    private int onSendImageNextFingerResult(long devId, int cmdId, byte[] result) {
        int err = 0;
        int offset = 0;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);
        }
        mHandler.obtainMessage(MSG_TEST_CMD_FRR_FAR_SEND_IMAGE_NEXT_FINGER, err).sendToTarget();
        return 0;
    }

    private int onSendCmdSuccess(long devId, int cmdId, byte[] result) {
        int err = 0;
        int offset = 0;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);
        }
        mHandler.obtainMessage(MSG_TEST_SUCCESS, err).sendToTarget();
        return 0;
    }

    private int onDeadPixelResult(long devId, int cmdId, byte[] result) {
        int err = TEST_RESULT_DATA_IMCOMPLITE;
        int offset = 0;
        int rlt;
        int deadpixelnum;
        int badlinenum;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (err == TEST_RESULT_OK) {
                if (result.length >= offset + 9) {
                    rlt = (0xFF & result[offset++]);
                    deadpixelnum = (0xFF & result[offset++]) << 24;
                    deadpixelnum |= (0xFF & result[offset++]) << 16;
                    deadpixelnum |= (0xFF & result[offset++]) << 8;
                    deadpixelnum |= (0xFF & result[offset++]);
                    badlinenum = (0xFF & result[offset++]) << 24;
                    badlinenum |= (0xFF & result[offset++]) << 16;
                    badlinenum |= (0xFF & result[offset++]) << 8;
                    badlinenum |= (0xFF & result[offset++]);
                    mHandler.obtainMessage(MSG_TEST_CMD_DEAD_PIXEL, new FingerDeadPixelResult(rlt,
                            deadpixelnum, badlinenum)).sendToTarget();
                } else {
                    err = TEST_RESULT_DATA_IMCOMPLITE;
                }
            }
        }

        if (err != TEST_RESULT_OK) {
            mHandler.obtainMessage(MSG_TEST_ERROR, err).sendToTarget();
        }
        return 1;
    }

    private int onTestSpeedResult(long devId, int cmdId, byte[] result) {
        int err = TEST_RESULT_DATA_IMCOMPLITE;
        int offset = 0;
        int rlt;
        int qualityscore = 0;
        int converarea = 0;
        int capture_time = 0;
        int reduce_noise_time = 0;
        int auth_time = 0;
        int tpl_upd_time = 0;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (err == TEST_RESULT_OK) {
                if (result.length >= offset + 19) {
                    rlt = (0xFF & result[offset++]);
                    qualityscore = (0xFF & result[offset++]);
                    converarea = (0xFF & result[offset++]);
                    capture_time = (0xFF & result[offset++]) << 24;
                    capture_time |= (0xFF & result[offset++]) << 16;
                    capture_time |= (0xFF & result[offset++]) << 8;
                    capture_time |= (0xFF & result[offset++]);
                    reduce_noise_time = (0xFF & result[offset++]) << 24;
                    reduce_noise_time |= (0xFF & result[offset++]) << 16;
                    reduce_noise_time |= (0xFF & result[offset++]) << 8;
                    reduce_noise_time |= (0xFF & result[offset++]);
                    auth_time = (0xFF & result[offset++]) << 24;
                    auth_time |= (0xFF & result[offset++]) << 16;
                    auth_time |= (0xFF & result[offset++]) << 8;
                    auth_time |= (0xFF & result[offset++]);
                    tpl_upd_time = (0xFF & result[offset++]) << 24;
                    tpl_upd_time |= (0xFF & result[offset++]) << 16;
                    tpl_upd_time |= (0xFF & result[offset++]) << 8;
                    tpl_upd_time |= (0xFF & result[offset++]);
                    mHandler.obtainMessage(MSG_TEST_CMD_SPEED, new FingerSpeedResult(rlt, qualityscore, converarea,
                            capture_time, reduce_noise_time, auth_time, tpl_upd_time)).sendToTarget();
                } else {
                    err = TEST_RESULT_DATA_IMCOMPLITE;
                }
            }
        }

        if (err != TEST_RESULT_OK) {
            mHandler.obtainMessage(MSG_TEST_ERROR, err).sendToTarget();
        }
        return 0;
    }

    public int onTestCmdResult(long devId, int cmdId, byte[] result) {
        if (DBG) {
            Log.d(TAG, "onTestCmdResult: cmdId = " + cmdId + ",result.length=" + (result == null ? 0 : result.length));
        }

        switch (cmdId) {
            case TEST_CMD_GET_VERSION: {
                return onVersionResult(devId, cmdId, result);
            }
            case TEST_CMD_SPI: {
                return onSpiTestResult(devId, cmdId, result);
            }
            case TEST_CMD_TEST_RESET_PIN: {
                return onResetPinTestResult(devId, cmdId, result);
            }
            case TEST_CMD_GET_IMAGE: {
                return onGetImageResult(devId, cmdId, result);
            }
            case TEST_CMD_WAIT_FINGER_UP: {
                return onWaitFingerUpResult(devId, cmdId, result);
            }
            case TEST_CMD_FRR_FAR_SEND_IMAGE: {
                return onSendImageResult(devId, cmdId, result);
            }
            case TEST_CMD_FRR_FAR_SEND_IMAGE_NEXT_FINGER: {
                return onSendImageNextFingerResult(devId, cmdId, result);
            }
            case TEST_CMD_IMAGE_FINISH: {
                return onSendCmdSuccess(devId, cmdId, result);
            }
            case TEST_CMD_DEAD_PIXEL: {
                return onDeadPixelResult(devId, cmdId, result);
            }
            case TEST_CMD_TEST_SPEED: {
                return onTestSpeedResult(devId, cmdId, result);
            }
            case TEST_CMD_TEST_FINISH: {
                return onSendCmdSuccess(devId, cmdId, result);
            }
            // Add for send fingerDown/Up message 20180615 begin
            case TEST_CMD_GATHER_IMAGE_FINGER_DOWN:
            case TEST_CMD_SEND_FINGER_DOWN: {
                return onGatherImageResult(devId, cmdId, result);
            }
            // Add for send fingerDown/Up message 20180615 end
        }
        return 0;
    }

    private IFingerServiceReceiver mServiceReceiver = new IFingerServiceReceiver.Stub() {
        @Override
        public int onTestCmd(int cmdId, byte[] result) throws RemoteException {
            return onTestCmdResult(0, cmdId, result);
        }
         @Override
        public int onTestError(int cmdId, int err) throws RemoteException {
            if (cmdId == TEST_CMD_SPI) {
                onSpiTestResult(0, cmdId, null);
            } else {
                mHandler.obtainMessage(MSG_TEST_ERROR, err).sendToTarget();
            }
            return 0;
        }
    };

    public static abstract class TestCmdCallback {
        public void onError(int err) {
        }

        public void onSuccess() {
        }

        public void onTestVersion(FingerVersion version) {
        }

        public void onGetImageSuccess(FingerFrrFarEnroll result) {
        }

        public void onFingerUp() {
        }

        public void onSendImageSuccess(FingerFrrFarImageResult result) {
        }

        public void onSendImageNextFinger() {
        }

        public void onDeadpixelTest(FingerDeadPixelResult result) {
        }

        public void onSpeedTest(FingerSpeedResult result) {
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
                Log.d(TAG, "handleMessage: msg.what = " + msg.what + ",mTestCmdCallback=" + mTestCmdCallback);
            }

            switch (msg.what) {
                case MSG_TEST_ERROR: {
                    if (mTestCmdCallback != null) {
                        int ret = ((Integer)msg.obj).intValue();
                        mTestCmdCallback.onError(ret);
                    }
                    break;
                }
                case MSG_TEST_SUCCESS: {
                    if (mTestCmdCallback != null) {
                        mTestCmdCallback.onSuccess();
                    }
                    break;
                }
                case MSG_TEST_CMD_GET_VERSION: {
                    if (mTestCmdCallback != null) {
                        FingerVersion version = (FingerVersion) msg.obj;
                        mTestCmdCallback.onTestVersion(version);
                    }
                    break;
                }
                case MSG_TEST_CMD_GET_IMAGE: {
                    if (mTestCmdCallback != null) {
                        FingerFrrFarEnroll ret = (FingerFrrFarEnroll) msg.obj;
                        mTestCmdCallback.onGetImageSuccess(ret);
                    }
                    break;
                }
                case MSG_TEST_CMD_WAIT_FINGER_UP: {
                    if (mTestCmdCallback != null) {
                        mTestCmdCallback.onFingerUp();
                    }
                    break;
                }
                case MSG_TEST_CMD_FRR_FAR_SEND_IMAGE: {
                    if (mTestCmdCallback != null) {
                        FingerFrrFarImageResult ret = (FingerFrrFarImageResult) msg.obj;
                        mTestCmdCallback.onSendImageSuccess(ret);
                    }
                    break;
                }
                case MSG_TEST_CMD_FRR_FAR_SEND_IMAGE_NEXT_FINGER: {
                    if (mTestCmdCallback != null) {
                        mTestCmdCallback.onSendImageNextFinger();
                    }
                    break;
                }
                case MSG_TEST_CMD_DEAD_PIXEL: {
                    if (mTestCmdCallback != null) {
                        FingerDeadPixelResult ret = (FingerDeadPixelResult) msg.obj;
                        mTestCmdCallback.onDeadpixelTest(ret);
                    }
                    break;
                }
                case MSG_TEST_CMD_SPEED: {
                    if (mTestCmdCallback != null) {
                        FingerSpeedResult ret = (FingerSpeedResult) msg.obj;
                        mTestCmdCallback.onSpeedTest(ret);
                    }
                    break;
                }
                // Add for send fingerDown/Up message 20180615 begin
                case MSG_TEST_CMD_GATHER_IMAGE: {
                    if (mTestCmdCallback != null) {
                        FingerFrrFarEnroll ret = (FingerFrrFarEnroll) msg.obj;
                        mTestCmdCallback.onGetImageSuccess(ret);
                    }
                    break;
                }
                // Add for send fingerDown/Up message 20180615 end
            }
        }
    };
}
