
package com.silead.fingerprint;

import android.app.Application;

public class FingerManagerApp extends Application {
    private FingerService mService;

    public FingerManagerApp() {
    }

    @Override
    public void onCreate() {
	
        FingerCommand fcmd = new FingerCommand();
        mService = new FingerService(this, fcmd);
    }
}
