
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public final class FingerVersion implements Parcelable {
    private CharSequence mHalVersion;
    private CharSequence mDevVersion;
    private CharSequence mAlgoVersion;
    private CharSequence mTaVersion;

    public FingerVersion(CharSequence hal, CharSequence dev, CharSequence algo, CharSequence ta) {
        mHalVersion = hal;
        mDevVersion = dev;
        mAlgoVersion = algo;
        mTaVersion = ta;
    }

    private FingerVersion(Parcel in) {
        mHalVersion = in.readString();
        mDevVersion = in.readString();
        mAlgoVersion = in.readString();
        mTaVersion = in.readString();
    }

    public CharSequence getHalVersion() {
        return mHalVersion;
    }

    public CharSequence getAlgoVersion() {
        return mAlgoVersion;
    }

    public CharSequence getDevVersion() {
        return mDevVersion;
    }

    public CharSequence getTaVersion() {
        return mTaVersion;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mHalVersion.toString());
        out.writeString(mDevVersion.toString());
        out.writeString(mAlgoVersion.toString());
        out.writeString(mTaVersion.toString());
    }

    public static final Parcelable.Creator<FingerVersion> CREATOR = new Parcelable.Creator<FingerVersion>() {
        public FingerVersion createFromParcel(Parcel in) {
            return new FingerVersion(in);
        }

        public FingerVersion[] newArray(int size) {
            return new FingerVersion[size];
        }
    };
};
