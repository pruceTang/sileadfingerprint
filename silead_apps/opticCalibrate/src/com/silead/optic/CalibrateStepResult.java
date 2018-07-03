
package com.silead.optic;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public final class CalibrateStepResult implements Parcelable {
    private int mErrorCode;
    private int mStep;

    public CalibrateStepResult(int err, int step) {
        mErrorCode = err;
        mStep = step;
    }

    private CalibrateStepResult(Parcel in) {
        mErrorCode = in.readInt();
        mStep = in.readInt();
    }

    public int getResult() {
        return mErrorCode;
    }

    public int getStep() {
        return mStep;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mErrorCode);
        out.writeInt(mStep);
    }

    public static final Parcelable.Creator<CalibrateStepResult> CREATOR = new Parcelable.Creator<CalibrateStepResult>() {
        public CalibrateStepResult createFromParcel(Parcel in) {
            return new CalibrateStepResult(in);
        }

        public CalibrateStepResult[] newArray(int size) {
            return new CalibrateStepResult[size];
        }
    };

    public static CalibrateStepResult parse(byte[] result) {
        int err = CalibrateManager.TEST_RESULT_DATA_IMCOMPLITE;
        int offset = 0;
        int step = -1;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (result.length >= offset + 1) {
                step = (0xFF & result[offset++]);
            }
        }
        return new CalibrateStepResult(err, step);
    }
};
