
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public final class FingerSpeedResult implements Parcelable {
    private int mResult;
    private int mImageQuality;
    private int mEffectiveArea;
    private int mCaptureTime;
    private int mReduceBgNoiseTime;
    private int mAuthTime;
    private int mTplUpdTime;

    public FingerSpeedResult(int result, int quality, int area, int capturetime, int reducenoisetime, int authtime, int tplupdtime) {
        mResult = result;
        mImageQuality = quality;
        mEffectiveArea = area;
        mCaptureTime = capturetime;
        mReduceBgNoiseTime = reducenoisetime;
        mAuthTime = authtime;
        mTplUpdTime = tplupdtime;
    }

    private FingerSpeedResult(Parcel in) {
        mResult = in.readInt();
        mImageQuality = in.readInt();
        mEffectiveArea = in.readInt();
        mCaptureTime = in.readInt();
        mReduceBgNoiseTime = in.readInt();
        mAuthTime = in.readInt();
        mTplUpdTime = in.readInt();
    }

    public int getResult() {
        return mResult;
    }

    public int getImageQuality() {
        return mImageQuality;
    }

    public int getEffectiveArea() {
        return mEffectiveArea;
    }
    
    public int getCaptureTime() {
        return mCaptureTime;
    }

    public int getReduceBgNoiseTime() {
        return mReduceBgNoiseTime;
    }
    
    public int getAuthTime() {
        return mAuthTime;
    }

    public int getTplUpdTime() {
        return mTplUpdTime;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mResult);
        out.writeInt(mImageQuality);
        out.writeInt(mEffectiveArea);
        out.writeInt(mCaptureTime);
        out.writeInt(mReduceBgNoiseTime);
        out.writeInt(mAuthTime);
        out.writeInt(mTplUpdTime);
    }

    public static final Parcelable.Creator<FingerSpeedResult> CREATOR = new Parcelable.Creator<FingerSpeedResult>() {
        public FingerSpeedResult createFromParcel(Parcel in) {
            return new FingerSpeedResult(in);
        }

        public FingerSpeedResult[] newArray(int size) {
            return new FingerSpeedResult[size];
        }
    };
};
