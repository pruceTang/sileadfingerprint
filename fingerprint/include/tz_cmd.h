/******************************************************************************
 * @file   tz_cmd.h
 * @brief  Contains CA/TA communication command IDs.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * David Wang  2018/4/2    0.1.0      Init version
 * Joe Li      2018/5/8    0.1.1      Add set nav mode command ID.
 * David Wang  2018/5/15   0.1.2      Add get finger status command ID.
 * John Zhang  2018/5/16   0.1.3      Add get config command ID.
 * Rich Li     2018/5/28   0.1.4      Add get enroll number command ID.
 * Davie Wang  2018/6/1    0.1.5      Add capture image sub command ID.
 * Davie Wang  2018/6/15   0.1.6      Add optics command ID.
 * Rich Li     2018/7/2    0.1.7      Add algo set param command ID.
 *
 *****************************************************************************/

/* This file must be the same between CA and TA APP */

#ifndef __SL_TZ_CMD_H__
#define __SL_TZ_CMD_H__

enum fp_tz_cmd {
    TZ_FP_CMD_MODE_DOWNLOAD           = 0x00000001,
    TZ_FP_CMD_CAPTURE_IMG             = 0x00000002,
    TZ_FP_CMD_NAV_CAPTURE_IMG         = 0x00000003,
    TZ_FP_CMD_AUTH_START              = 0x00000004,
    TZ_FP_CMD_AUTH_STEP               = 0x00000005,
    TZ_FP_CMD_AUTH_END                = 0x00000006,
    TZ_FP_CMD_ENROLL_START            = 0x00000007,
    TZ_FP_CMD_ENROLL_STEP             = 0x00000008,
    TZ_FP_CMD_ENROLL_END              = 0x00000009,
    TZ_FP_CMD_NAV_START               = 0x0000000A,
    TZ_FP_CMD_NAV_STEP                = 0x0000000B,
    TZ_FP_CMD_NAV_END                 = 0x0000000C,
    TZ_FP_CMD_NAV_SUPPORT             = 0x0000000D,
    TZ_FP_CMD_INIT                    = 0x0000000E,
    TZ_FP_CMD_DEINIT                  = 0x0000000F,
    TZ_FP_CMD_SET_GID                 = 0x00000010,
    TZ_FP_CMD_LOAD_USER_DB            = 0x00000011,
    TZ_FP_CMD_FP_REMOVE               = 0x00000012,
    TZ_FP_CMD_GET_DB_COUNT            = 0x00000013,
    TZ_FP_CMD_GET_FINGERPRINTS        = 0x00000014,
    TZ_FP_CMD_LOAD_ENROLL_CHALLENGE   = 0x00000015,
    TZ_FP_CMD_SET_ENROLL_CHALLENGE    = 0x00000016,
    TZ_FP_CMD_VERIFY_ENROLL_CHALLENGE = 0x00000017,
    TZ_FP_CMD_LOAD_AUTH_ID            = 0x00000018,
    TZ_FP_CMD_GET_AUTH_OBJ            = 0x00000019,
    TZ_FP_CMD_UPDATE_CFG              = 0x0000001A,
    TZ_FP_CMD_INIT2                   = 0x0000001B,
    TZ_FP_CMD_LOAD_TEMPLATE           = 0x0000001C,
    TZ_FP_CMD_SAVE_TEMPLATE           = 0x0000001D,
    TZ_FP_CMD_UPDATE_TEMPLATE         = 0x0000001E,
    TZ_FP_CMD_SET_LOG_MODE            = 0x0000001F,
    TZ_FP_CMD_SET_SPI_DEV             = 0x00000020,
    TZ_FP_CMD_SET_NAV_TYPE            = 0x00000021,
    TZ_FP_CMD_GET_FINGER_STATUS       = 0x00000022,
    TZ_FP_CMD_GET_CONFIG              = 0x00000023,
    TZ_FP_CMD_GET_ENROLL_NUM          = 0x00000024,
    TZ_FP_CMD_CI_CHK_FINGER           = 0x00000025,
    TZ_FP_CMD_CI_ADJ_GAIN             = 0x00000026,
    TZ_FP_CMD_CI_SHOT                 = 0x00000027,
    TZ_FP_CMD_SET_ALG_PARAM           = 0x00000028,

    TZ_FP_CMD_SET_KEY_DATA            = 0x00000030,
    TZ_FP_CMD_INIT_UNK_0              = 0x00000031,
    TZ_FP_CMD_INIT_UNK_1              = 0x00000032,
    TZ_FP_CMD_DEINIT_UNK_1            = 0x00000033,
    TZ_FP_CMD_INIT_UNK_2              = 0x00000034,
    TZ_FP_CMD_DEINIT_UNK_2            = 0x00000035,
    TZ_FP_CMD_CALIBRATE               = 0x00000036,
    TZ_FP_CMD_CALIBRATE2              = 0x00000037,
    TZ_FP_CMD_CHECK_ESD               = 0x00000038,
    TZ_FP_CMD_GET_OTP                 = 0x00000039,

    TZ_FP_CMD_CALIBRATE_OPTIC         = 0x00000040,

    TZ_FP_CMD_GET_VERSIONS            = 0x00000050,
    TZ_FP_CMD_GET_CHIPID              = 0x00000051,
    TZ_FP_CMD_TEST_IMAGE_CAPTURE      = 0x00000052,
    TZ_FP_CMD_TEST_SEND_GP_IMG        = 0x00000053,
    TZ_FP_CMD_TEST_IMAGE_FINISH       = 0x00000054,
    TZ_FP_CMD_TEST_DEADPX             = 0x00000055,
    TZ_FP_CMD_TEST_GET_IMG_INFO       = 0x00000056,
    TZ_FP_CMD_TEST_DUMP_DATA          = 0x00000057,
    TZ_FP_CMD_TEST_SPEED              = 0x00000058,
    TZ_FP_CMD_TEST_CMD                = 0x00000059,
};

static inline const char *cmd2str(int32_t cmd)
{
    switch(cmd) {
    case TZ_FP_CMD_MODE_DOWNLOAD:
        return "MODE_DOWNLOAD";
    case TZ_FP_CMD_CAPTURE_IMG:
        return "CAPTURE_IMG";
    case TZ_FP_CMD_NAV_CAPTURE_IMG:
        return "NAV_CAPTURE_IMG";
    case TZ_FP_CMD_AUTH_START:
        return "AUTH_START";
    case TZ_FP_CMD_AUTH_STEP:
        return "AUTH_STEP";
    case TZ_FP_CMD_AUTH_END:
        return "AUTH_END";
    case TZ_FP_CMD_ENROLL_START:
        return "ENROLL_START";
    case TZ_FP_CMD_ENROLL_STEP:
        return "ENROLL_STEP";
    case TZ_FP_CMD_ENROLL_END:
        return "ENROLL_END";
    case TZ_FP_CMD_NAV_START:
        return "NAV_START";
    case TZ_FP_CMD_NAV_STEP:
        return "NAV_STEP";
    case TZ_FP_CMD_NAV_END:
        return "NAV_END";
    case TZ_FP_CMD_NAV_SUPPORT:
        return "NAV_SUPPORT";
    case TZ_FP_CMD_INIT:
        return "INIT";
    case TZ_FP_CMD_DEINIT:
        return "DEINIT";
    case TZ_FP_CMD_SET_GID:
        return "SET_GID";
    case TZ_FP_CMD_LOAD_USER_DB:
        return "LOAD_USER_DB";
    case TZ_FP_CMD_FP_REMOVE:
        return "FP_REMOVE";
    case TZ_FP_CMD_GET_DB_COUNT:
        return "GET_DB_COUNT";
    case TZ_FP_CMD_GET_FINGERPRINTS:
        return "GET_FINGERPRINTS";
    case TZ_FP_CMD_LOAD_ENROLL_CHALLENGE:
        return "LOAD_ENROLL_CHALLENGE";
    case TZ_FP_CMD_SET_ENROLL_CHALLENGE:
        return "SET_ENROLL_CHALLENGE";
    case TZ_FP_CMD_VERIFY_ENROLL_CHALLENGE:
        return "VERIFY_ENROLL_CHALLENGE";
    case TZ_FP_CMD_LOAD_AUTH_ID:
        return "LOAD_AUTH_ID";
    case TZ_FP_CMD_GET_AUTH_OBJ:
        return "GET_AUTH_OBJ";
    case TZ_FP_CMD_UPDATE_CFG:
        return "UPDATE_CFG";
    case TZ_FP_CMD_INIT2:
        return "INIT2";
    case TZ_FP_CMD_LOAD_TEMPLATE:
        return "LOAD_TEMPLATE";
    case TZ_FP_CMD_SAVE_TEMPLATE:
        return "SAVE_TEMPLATE";
    case TZ_FP_CMD_UPDATE_TEMPLATE:
        return "UPDATE_TEMPLATE";
    case TZ_FP_CMD_SET_LOG_MODE:
        return "SET_LOG_MODE";
    case TZ_FP_CMD_SET_SPI_DEV:
        return "SET_SPI_DEV";
    case TZ_FP_CMD_SET_NAV_TYPE:
        return "SET_NAV_TYPE";
    case TZ_FP_CMD_GET_FINGER_STATUS:
        return "GET_FINGER_STATUS";
    case TZ_FP_CMD_GET_CONFIG:
        return "GET_CONFIG";
    case TZ_FP_CMD_GET_ENROLL_NUM:
        return "GET_ENROLL_NUM";
    case TZ_FP_CMD_CI_CHK_FINGER:
        return "CHK_FINGER";
    case TZ_FP_CMD_CI_ADJ_GAIN:
        return "ADJ_GAIN";
    case TZ_FP_CMD_CI_SHOT:
        return "SHOT";
    case TZ_FP_CMD_SET_ALG_PARAM:
        return "SET_ALG_PARAM";
    case TZ_FP_CMD_CHECK_ESD:
        return "CHK_ESD";
    case TZ_FP_CMD_GET_OTP:
        return "GET_OTP";

    case TZ_FP_CMD_CALIBRATE_OPTIC:
        return "CALIBRATE_OPTIC";

    case TZ_FP_CMD_SET_KEY_DATA:
        return "SET_KEY_DATA";
    case TZ_FP_CMD_INIT_UNK_0:
        return "INIT_UNK_0";
    case TZ_FP_CMD_INIT_UNK_1:
        return "INIT_UNK_1";
    case TZ_FP_CMD_DEINIT_UNK_1:
        return "DEINIT_UNK_1";
    case TZ_FP_CMD_INIT_UNK_2:
        return "INIT_UNK_2";
    case TZ_FP_CMD_DEINIT_UNK_2:
        return "DEINIT_UNK_2";
    case TZ_FP_CMD_CALIBRATE:
        return "CALIBRATE";
    case TZ_FP_CMD_CALIBRATE2:
        return "CALIBRATE2";

    case TZ_FP_CMD_GET_VERSIONS:
        return "GET_VERSIONS";
    case TZ_FP_CMD_GET_CHIPID:
        return "GET_CHIPID";
    case TZ_FP_CMD_TEST_IMAGE_CAPTURE:
        return "TEST_IMAGE_CAPTURE";
    case TZ_FP_CMD_TEST_SEND_GP_IMG:
        return "TEST_SEND_GP_IMG";
    case TZ_FP_CMD_TEST_IMAGE_FINISH:
        return "TEST_IMAGE_FINISH";
    case TZ_FP_CMD_TEST_DEADPX:
        return "TEST_DEADPX";
    case TZ_FP_CMD_TEST_GET_IMG_INFO:
        return "TEST_GET_IMG_INFO";
    case TZ_FP_CMD_TEST_DUMP_DATA:
        return "TEST_DUMP_DATA";
    case TZ_FP_CMD_TEST_SPEED:
        return "TEST_SPEED";
    case TZ_FP_CMD_TEST_CMD:
        return "TEST_CMD";
    default:
        return "Unknown";
    }
}

#endif /* __SL_TZ_CMD_H__ */

