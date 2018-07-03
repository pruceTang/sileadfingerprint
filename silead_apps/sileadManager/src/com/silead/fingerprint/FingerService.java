
package com.silead.fingerprint;

import java.util.NoSuchElementException;

import android.content.Context;
import android.os.IBinder;
import android.os.IBinder.DeathRecipient;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.silead.manager.FingerManager;
import com.silead.internal.IFingerService;
import com.silead.internal.IFingerServiceReceiver;

public class FingerService extends IFingerService.Stub {
    private static final String LOG_TAG = "fingerprintservice";
    private static final boolean DBG = false;

    private static final String USE_FINGERPRINT="android.permission.USE_FINGERPRINT";

    private Context mContext;
    private ClientMonitor mTestClient;
    private FingerCommand mFingerCommand;

    public FingerService(Context context, FingerCommand fcmd) {
        mContext = context;
        mFingerCommand = fcmd;
        
        mFingerCommand.setFpCallback(mFingerprintCallback);

        publish();
    }

    private void publish() {
        if (DBG) {
            Log.d(LOG_TAG, "publish: " + FingerManager.FINGERPRINT_TEST_SERVICE);
        }
        ServiceManager.addService(FingerManager.FINGERPRINT_TEST_SERVICE, this);
    }

    FingerFpCallback mFingerprintCallback =  new FingerFpCallback() {
        @Override
        public void onTestCmd(int cmdId, byte[] result) {
            if (DBG) {
                Log.w(LOG_TAG, "cmdId = " + cmdId + ", result.length = " + (result == null ? 0 : result.length));
            }

            if (mTestClient != null && mTestClient.sendTestResult(cmdId, result)) {
                removeClient(mTestClient);
            }
        }

        @Override
        public void onError(int cmdId, int err) {
            if (DBG) {
                Log.w(LOG_TAG, "cmdId = " + cmdId + ", err = " + err);
            }

            if (mTestClient != null && mTestClient.onTestError(cmdId, err)) {
                removeClient(mTestClient);
            }
        }
    };

    @Override
    public int testCmd(final IBinder token, int cmdId, byte[] param, IFingerServiceReceiver receiver) {
        checkPermission(USE_FINGERPRINT);

        removeClient(mTestClient);
        mTestClient = new ClientMonitor(token, receiver);
        return mFingerCommand.testCmd(cmdId, param);
    }

    void checkPermission(String permission) {
        mContext.enforceCallingOrSelfPermission(permission,
                "Must have " + permission + " permission.");
    }

    private void removeClient(ClientMonitor client) {
        if (client == null) {
            return;
        }
        client.destroy();
        if (client == mTestClient) {
            mTestClient = null;
        }
    }

    private class ClientMonitor implements IBinder.DeathRecipient {
        IBinder token;
        IFingerServiceReceiver receiver;

        public ClientMonitor(IBinder token, IFingerServiceReceiver receiver) {
            this.token = token;
            this.receiver = receiver;
            try {
                token.linkToDeath(this, 0);
            } catch (RemoteException e) {
                Log.w(LOG_TAG, "caught remote exception in linkToDeath: ", e);
            }
        }

        public void destroy() {
            if (token != null) {
                try {
                    token.unlinkToDeath(this, 0);
                } catch (NoSuchElementException e) {
                    Log.e(LOG_TAG, "destroy(): " + this + ":", new Exception("here"));
                }
                token = null;
            }
            receiver = null;
        }

        @Override
        public void binderDied() {
            token = null;
            removeClient(this);
            receiver = null;
        }

        @Override
        protected void finalize() throws Throwable {
            try {
                if (token != null) {
                    if (DBG) {
                        Log.w(LOG_TAG, "removing leaked reference: " + token);
                    }
                    removeClient(this);
                }
            } finally {
                super.finalize();
            }
        }

        private boolean sendTestResult(int cmdId, byte[] result) {
            if (receiver == null) {
                return true; // client not listening
            }

            try {
                int ret = receiver.onTestCmd(cmdId, result);
                if (DBG) {
                    Log.d(LOG_TAG, "sendTestResult ret=" + ret);
                }
                return ret > 0 ? true : false;
            } catch (RemoteException e) {
                Log.w(LOG_TAG, "Failed to notify EnrollResult:", e);
                return true;
            }
        }
        
        private boolean onTestError(int cmdId, int err) {
            if (receiver == null) {
                return true; // client not listening
            }

            try {
                int ret = receiver.onTestError(cmdId, err);
                if (DBG) {
                    Log.d(LOG_TAG, "sendTestResult ret=" + ret);
                }
                return true;
            } catch (RemoteException e) {
                Log.w(LOG_TAG, "Failed to notify EnrollResult:", e);
                return true;
            }
        }
    }
}
