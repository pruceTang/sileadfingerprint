
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public final class FingerDeadPixelResult implements Parcelable {
    private int mResult;
    private int mDeadPixelNum;
    private int mBadlineNum;

    public FingerDeadPixelResult(int result, int deadPixelnum, int badlinenum) {
        mResult = result;
        mDeadPixelNum = deadPixelnum;
        mBadlineNum = badlinenum;
    }

    private FingerDeadPixelResult(Parcel in) {
        mResult = in.readInt();
        mDeadPixelNum = in.readInt();
        mBadlineNum = in.readInt();
    }

    public int getResult() {
        return mResult;
    }

    public int getDeadPixelNum() {
        return mDeadPixelNum;
    }

    public int getBadlineNum() {
        return mBadlineNum;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mResult);
        out.writeInt(mDeadPixelNum);
        out.writeInt(mBadlineNum);
    }

    public static final Parcelable.Creator<FingerDeadPixelResult> CREATOR = new Parcelable.Creator<FingerDeadPixelResult>() {
        public FingerDeadPixelResult createFromParcel(Parcel in) {
            return new FingerDeadPixelResult(in);
        }

        public FingerDeadPixelResult[] newArray(int size) {
            return new FingerDeadPixelResult[size];
        }
    };
};
