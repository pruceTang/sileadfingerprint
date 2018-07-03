
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public final class FingerFrrFarImageResult implements Parcelable {
    private int mIsEnroll;
    private int mResult;
    private int mIndex;
    private int mTime;
    private int mTplUpdTime;

    public FingerFrrFarImageResult(int isenroll, int result, int index, int time1, int time2) {
        mIsEnroll = isenroll;
        mResult = result;
        mIndex = index;
        mTime = time1;
        mTplUpdTime = time2;
    }

    private FingerFrrFarImageResult(Parcel in) {
        mIsEnroll = in.readInt();
        mResult = in.readInt();
        mIndex = in.readInt();
        mTime = in.readInt();
        mTplUpdTime = in.readInt();
    }

    public int isEnrolled() {
        return mIsEnroll;
    }

    public int getResult() {
        return mResult;
    }

    public int getIndex() {
        return mIndex;
    }

    public int getTime() {
        return mTime;
    }

    public int getTplUpdTime() {
        return mTplUpdTime;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mIsEnroll);
        out.writeInt(mResult);
        out.writeInt(mTime);
        out.writeInt(mTplUpdTime);
    }

    public static final Parcelable.Creator<FingerFrrFarImageResult> CREATOR = new Parcelable.Creator<FingerFrrFarImageResult>() {
        public FingerFrrFarImageResult createFromParcel(Parcel in) {
            return new FingerFrrFarImageResult(in);
        }

        public FingerFrrFarImageResult[] newArray(int size) {
            return new FingerFrrFarImageResult[size];
        }
    };
};
