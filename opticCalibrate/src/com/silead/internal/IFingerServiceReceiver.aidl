
package com.silead.internal;

import android.os.IBinder;

/**
 * @hide
 */
interface IFingerServiceReceiver {
    int onTestCmd(int cmdId, in byte[] result);
    int onTestError(int cmdId, int err);
}
