
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public final class FingerFrrFarEnroll implements Parcelable {
    private int mErrCode;
    private int mImageQuality;
    private int mEffectiveArea;
    private int mIndex;
    private int mIsTpl;
    private byte[] mData;

    public FingerFrrFarEnroll(int errcode, int quality, int area, int index, int istpl, byte[] data, int offset, int dataLen) {
        mErrCode = errcode;
        mImageQuality = quality;
        mEffectiveArea = area;
        mIndex = index;
        mIsTpl = istpl;
        if (data != null && data.length >= offset + dataLen) {
            mData = new byte[dataLen];
            System.arraycopy(data, offset, mData, 0, dataLen);
        } else {
            mData = new byte[0];
        }
    }

    private FingerFrrFarEnroll(Parcel in) {
        mErrCode = in.readInt();
        mImageQuality = in.readInt();
        mEffectiveArea = in.readInt();
        mIndex = in.readInt();
        mIsTpl = in.readInt();
        mData = in.createByteArray();
    }

    public int getErrCode() {
        return mErrCode;
    }

    public int getImageQuality() {
        return mImageQuality;
    }

    public int getEffectiveArea() {
        return mEffectiveArea;
    }

    public int getIndex() {
        return mIndex;
    }

    public byte[] getData() {
        return mData;
    }

    public boolean isTplImage() {
        return (mIsTpl != 0);
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mErrCode);
        out.writeInt(mImageQuality);
        out.writeInt(mEffectiveArea);
        out.writeInt(mIndex);
        out.writeInt(mIsTpl);
        out.writeByteArray(mData);
    }

    public static final Parcelable.Creator<FingerFrrFarEnroll> CREATOR = new Parcelable.Creator<FingerFrrFarEnroll>() {
        public FingerFrrFarEnroll createFromParcel(Parcel in) {
            return new FingerFrrFarEnroll(in);
        }

        public FingerFrrFarEnroll[] newArray(int size) {
            return new FingerFrrFarEnroll[size];
        }
    };
};
