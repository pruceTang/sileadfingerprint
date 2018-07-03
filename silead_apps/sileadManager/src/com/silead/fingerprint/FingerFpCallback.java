
package com.silead.fingerprint;

public interface FingerFpCallback {
    public void onTestCmd(int cmdId, byte[] result);
    public void onError(int cmdId, int err);
}
