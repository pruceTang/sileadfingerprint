
package com.silead.frrfar;

import java.io.File;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.content.Context;

public class FingerSettingsConst {
    public static final String LOG_TAG = "frrfarTest";
    public static final boolean LOG_DBG = true;
    
    public static final boolean FP_UI_BACK_BTN_SUPPORT = true;
    private static final boolean DATA_FILE_NAME_INDICATE_FINGER = true;
    private static final int MIN_USER_NAME_LEN = 4;
    private static final int MIN_DATA_FILE_NAME_LEN = 4;

    public static final boolean FP_SETTINGS_FRRFAR_IMG_RESULT_DEFAULT = true;
    public static final boolean FP_SETTINGS_FAR_RUN_FRR_FIRST_DEFAULT = true;
    public static final boolean FP_SETTINGS_GATHER_FINGER_DEFAULT     = true;
    public static final int FP_SETTINGS_SAMPLE_COUNT_DEFAULT = 50;
    public static final int FP_SETTINGS_ENROLL_FINGER_DEFAULT = 0x07; // 0 0111

    public static final String ENROLL_ENV_NEXT_USER_DEFAULT = "0001";
    public static final int ENROLL_ENV_NEXT_HAND_DEFAULT = 0;
    public static final int ENROLL_ENV_NEXT_FINGER_DEFAULT = 0;

    public static final String FRR_FAR_DATA_FILE_SUFFIX = ".bmp";

    public static final String ENROLL_ENV_CONFIG = "enroll_env";
    public static final String ENROLL_ENV_NEXT_HAND_NAME = "next_hand";
    public static final String ENROLL_ENV_NEXT_FINGER_NAME = "next_finger";
    public static final String ENROLL_ENV_NEXT_USERID_NAME = "next_userid";

    public static final String FP_SETTINGS_CONFIG = "fp_settings";
    public static final String FP_SETTINGS_SAMPLE_COUNT_NAME = "sample_count";
    public static final String FP_SETTINGS_ENROLL_TEMPLATE_NAME = "template_enroll";
    public static final String FP_SETTINGS_ENROLL_FINGER_NAME = "finger_enroll";
    public static final String FP_SETTINGS_FRRFAR_IMG_RESULT_NAME = "farfar_img_result";
    public static final String FP_SETTINGS_FAR_RUN_FRR_FIRST_NAME = "far_run_frr_first";
    public static final String FP_SETTINGS_GATHER_FINGER_NAME = "gather_finger";

    // should the same size with enroll_env_set_hand_values in arrays.xml
    private static final String[] HAND_STR = {
            "L", "R"
    };
    // should the same size with finger_settings_enroll_finger_values in arrays.xml
    private static final String[] FINGER_STR = {
            "1", "2", "3", "4", "5"
    };

    public static File getDataSavePath(Context context) {
        //File data = context.getFilesDir();
        String data = "/data/system/silead";
        File folder = new File(data, "image");
        if (!folder.exists()) {
            folder.mkdirs();
        }
        return folder;
    }

    public static File getTestResultPath(Context context, boolean ffr) {
        File data = context.getFilesDir();
        File result = new File(data, "result");
        if (!result.exists()) {
            result.mkdir();
        }

        File folder;
        if (ffr) {
            folder = new File(result, "frr");
        } else {
            folder = new File(result, "far");
        }
        if (!folder.exists()) {
            folder.mkdir();
        }

        return folder;
    }

    public static String getFingerPath(int hand, int fingers, int finger) {
        String path = "";
        int i;
        int count = 0;

        int index = hand;
        if (index < 0 || index >= HAND_STR.length) {
            index = 0;
        }
        path = HAND_STR[index];

        index = finger;
        if (index < 0 || index >= FINGER_STR.length) {
            index = 0;
        }
        if (DATA_FILE_NAME_INDICATE_FINGER) {
            for (i = 0; i < FINGER_STR.length; i++) {
                if ((fingers & (1 << i)) != 0) {
                    if (count == finger) {
                        break;
                    }
                    count++;
                }
            }

            if (i >= FINGER_STR.length) {
                for (i = 0; i < FINGER_STR.length; i++) {
                    if ((fingers & (1 << i)) != 0) {
                        break;
                    }
                }
            }
            path += FINGER_STR[i];
        } else {
            path += FINGER_STR[index];
        }

        return path;
    }

    public static String getDataFileName(int index) {
        StringBuffer name = new StringBuffer();
        String num = String.valueOf(index);
        for (int i = 0; i < MIN_DATA_FILE_NAME_LEN - num.length(); i++) {
            name.append("0");
        }
        name.append(num);
        name.append(FRR_FAR_DATA_FILE_SUFFIX);
        return name.toString();
    }

    public static boolean isLastFinger(int fingers, int finger) {
        if (fingers == 0) {
            fingers = FP_SETTINGS_ENROLL_FINGER_DEFAULT;
        }

        int count = numberInBinary(fingers);
        return (finger >= count - 1);
    }

    public static int getNextFinger(int finger, boolean lastfinger) {
        int next = finger + 1;
        if (lastfinger) {
            next = 0;
        }
        return next;
    }

    public static int getNextHand(int hand, boolean lastfinger) {
        int next = hand;
        if (lastfinger) {
            next += 1;
        }
        if (next >= HAND_STR.length) {
            next = 0;
        }
        return next;
    }

    public static String getNextUser(String user, int hand, boolean lastfinger) {
        String nextUser = user;
        if (hand >= HAND_STR.length - 1 && lastfinger) {
            if (isNumeric(user)) {
                int value = Integer.valueOf(user) + 1;
                String num = String.valueOf(value);
                StringBuffer sb = new StringBuffer();
                for (int i = 0; i < MIN_USER_NAME_LEN - num.length(); i++) {
                    sb.append("0");
                }
                sb.append(num);
                nextUser = sb.toString();
            }
        }
        return nextUser;
    }

    public static boolean isNumeric(String str) {
        Pattern pattern = Pattern.compile("[0-9]*");
        Matcher isNum = pattern.matcher(str);
        if (!isNum.matches()) {
            return false;
        }
        return true;
    }

    public static int numberInBinary(int n) {
        int count = 0;
        int value = n;
        while (value != 0) {
            count++;
            value = value & (value - 1);
        }
        return count;
    }

    public static void sortFiles(File[] files) {
        List<File> fileList = Arrays.asList(files);
        Collections.sort(fileList, new Comparator<File>() {
            @Override
            public int compare(File o1, File o2) {
                if (o1.isDirectory() && o2.isFile())
                    return -1;
                if (o1.isFile() && o2.isDirectory())
                    return 1;
                return o1.getName().compareTo(o2.getName());
            }
        });
    }
}
