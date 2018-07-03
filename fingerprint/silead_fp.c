/******************************************************************************
 * @file   silead_fp.c
 * @brief  Contains CA send command functions.
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
 * David Wang  2018/5/15   0.1.1      Support get finger status
 * John Zhang  2018/5/15   0.1.2      Support get config
 * Jack Zhang  2018/5/17   0.1.3      Change test process to simplify app use
 * Rich Li     2018/5/28   0.1.4      Add get enroll number command ID.
 * Davie Wang  2018/6/1    0.1.5      Add capture image sub command ID.
 * Davie Wang  2018/6/15   0.1.6      Add optics command ID.
 * Rich Li     2018/7/2    0.1.7      Add algo set param command ID.
 *
 *****************************************************************************/

#define FILE_TAG "silead_fp"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "tz_cmd.h"
#include "silead_fp.h"
#include "silead_error.h"
#include "silead_ca.h"

// 0: send buffer 1: get buffer 2: send & get buffer 3:always get even error
#define _tz_send_modified_command(cmd, p, l)                                silfp_ca_send_modified_command(cmd, p, l, 0, 0, 0, NULL, NULL)
#define _tz_send_modified_command_1d(cmd, p, l, d)                          silfp_ca_send_modified_command(cmd, p, l, 0, d, 0, NULL, NULL)
#define _tz_send_modified_command_2r(cmd, p, l, r1, r2)                     silfp_ca_send_modified_command(cmd, p, l, 0, 0, 0, r1, r2)
#define _tz_send_modified_command_1d_1r_and_get(cmd, p, l, d1, r1)          silfp_ca_send_modified_command(cmd, p, l, 2, d1, 0, r1, NULL)
#define _tz_send_modified_command_get(cmd, p, l)                            silfp_ca_send_modified_command(cmd, p, l, 1, 0, 0, NULL, NULL)
#define _tz_send_modified_command_get_1r(cmd, p, l, r)                      silfp_ca_send_modified_command(cmd, p, l, 1, 0, 0, r, NULL)
#define _tz_send_modified_command_get_2r(cmd, p, l, r1, r2)                 silfp_ca_send_modified_command(cmd, p, l, 1, 0, 0, r1, r2)
#define _tz_send_modified_command_get_1d_2r(cmd, p, l, d, r1, r2)           silfp_ca_send_modified_command(cmd, p, l, 1, d, 0, r1, r2)
#define _tz_send_modified_command_and_get(cmd, p, l)                        silfp_ca_send_modified_command(cmd, p, l, 2, 0, 0, NULL, NULL)
#define _tz_send_modified_command_get_1d_2r_always(cmd, p, l, d, r1, r2)    silfp_ca_send_modified_command(cmd, p, l, 3, d, 0, r1, r2)
#define _tz_send_normal_command(cmd)                                    silfp_ca_send_normal_command(cmd, 0, 0, 0, 0, NULL, NULL, NULL)
#define _tz_send_normal_command_1d(cmd, d)                              silfp_ca_send_normal_command(cmd, d, 0, 0, 0, NULL, NULL, NULL)
#define _tz_send_normal_command_2d(cmd, d1, d2)                         silfp_ca_send_normal_command(cmd, d1, d2, 0, 0, NULL, NULL, NULL)
#define _tz_send_normal_command_1r(cmd, p1)                             silfp_ca_send_normal_command(cmd, 0, 0, 0, 0, p1, NULL, NULL)
#define _tz_send_normal_command_2r(cmd, p1, p2)                         silfp_ca_send_normal_command(cmd, 0, 0, 0, 0, p1, p2, NULL)
#define _tz_send_normal_command_3r(cmd, p1, p2, p3)                     silfp_ca_send_normal_command(cmd, 0, 0, 0, 0, p1, p2, p3)
#define _tz_send_normal_command_1d_2r(cmd, d1, p1, p2)                  silfp_ca_send_normal_command(cmd, d1, 0, 0, 0, p1, p2, NULL)
#define _tz_send_normal_command_2d_1r(cmd, d1, d2, p1)                  silfp_ca_send_normal_command(cmd, d1, d2, 0, 0, p1, NULL, NULL)
#define _tz_send_normal_command_4d_2r(cmd, d1, d2, d3, d4, p1, p2)      silfp_ca_send_normal_command(cmd, d1, d2, d3, d4, p1, p2, NULL)
#define _tz_send_normal_command_4d_3r(cmd, d1, d2, d3, d4, p1, p2, p3)  silfp_ca_send_normal_command(cmd, d1, d2, d3, d4, p1, p2, p3)

static int32_t fp_tz_open(void)
{
    return silfp_ca_open();
}

static int32_t fp_tz_close(void)
{
    return silfp_ca_close();
}

static int64_t fp_tz_get_int64_command(uint32_t cmd, uint32_t v)
{
    int64_t value = 0;
    uint32_t low;
    uint32_t high;

    int32_t ret = _tz_send_normal_command_1d_2r(cmd, v, &high, &low);
    if (ret < 0) {
        value = ret;
    } else {
        value = high;
        value <<= 32;
        value |= low;
    }

    return value;
}

static int32_t fp_tz_wait_finger_status(uint32_t status)
{
    return _tz_send_normal_command_1d(TZ_FP_CMD_MODE_DOWNLOAD, status);
}

static int32_t fp_tz_capture_image(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_CAPTURE_IMG);
}

static int32_t fp_tz_nav_capture_image(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_NAV_CAPTURE_IMG);
}

static int32_t fp_tz_auth_start(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_AUTH_START);
}

static int32_t fp_tz_auth_step(uint64_t op_id, uint32_t *fid)
{
    return _tz_send_normal_command_2d_1r(TZ_FP_CMD_AUTH_STEP, (uint32_t)(op_id >> 32), (uint32_t)op_id, fid);
}

static int32_t fp_tz_auth_end(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_AUTH_END);
}

static int32_t fp_tz_enroll_start(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_ENROLL_START);
}

static int32_t fp_tz_enroll_step(uint32_t *remaining)
{
    return _tz_send_normal_command_1r(TZ_FP_CMD_ENROLL_STEP, remaining);
}

static int32_t fp_tz_enroll_end(int32_t status, uint32_t *fid)
{
    if (fid != NULL) {
        return _tz_send_normal_command_2d_1r(TZ_FP_CMD_ENROLL_END, status, *fid, fid);
    } else {
        return _tz_send_normal_command_1d(TZ_FP_CMD_ENROLL_END, status);
    }
}

static int32_t fp_tz_nav_support(uint32_t *type)
{
    return _tz_send_normal_command_1r(TZ_FP_CMD_NAV_SUPPORT, type);
}

static int32_t fp_tz_nav_start(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_NAV_START);
}

static int32_t fp_tz_nav_step(uint32_t *pkey)
{
    return _tz_send_normal_command_1r(TZ_FP_CMD_NAV_STEP, pkey);
}

static int32_t fp_tz_nav_end(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_NAV_END);
}

static int32_t fp_tz_download_normal(void)
{
    return _tz_send_normal_command_1d(TZ_FP_CMD_MODE_DOWNLOAD, NORMAL);
}

static int32_t fp_tz_init(FP_DEV_CONF *dev_conf, uint32_t *chipid, uint32_t *subid, uint32_t *vid, uint32_t *update_cfg)
{
    int32_t ret;
    uint32_t value1 = 0;
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
    uint32_t data4;

    if (dev_conf == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }

    data1 = (dev_conf->mode & 0x000000FF);
    data1 <<= 8;
    data1 |= (dev_conf->bits & 0x000000FF);
    data1 <<= 16;
    data1 |= (dev_conf->delay & 0x0000FFFF);

    data2 = dev_conf->speed;

    data3 = (dev_conf->dev_id & 0x000000FF);
    data3 <<= 16;
    data3 |= (dev_conf->reserve & 0x0000FFFF);

    data4 = dev_conf->reg;

    if (dev_conf->dev[0] != 0 && strlen(dev_conf->dev) > 0) {
        _tz_send_modified_command(TZ_FP_CMD_SET_SPI_DEV, dev_conf->dev, strlen(dev_conf->dev));
    }

    ret = _tz_send_normal_command_4d_3r(TZ_FP_CMD_INIT, data1, data2, data3, data4, chipid, subid, &value1);
    if (ret >= 0) {
        if (vid != NULL) {
            *vid = (value1 & 0x7FFFFFFF);
        }
        if (update_cfg != NULL) {
            *update_cfg = (value1 & 0x80000000) ? 1 : 0;
        }
    }
    return ret;
}

static int32_t fp_tz_deinit(void)
{
    _tz_send_normal_command(TZ_FP_CMD_DEINIT);
    return fp_tz_close();
}

static int32_t fp_tz_set_gid(uint32_t gid, uint32_t serial)
{
    return _tz_send_normal_command_2d(TZ_FP_CMD_SET_GID, gid, serial);
}

static int32_t fp_tz_load_user_db(const char* db_path)
{
    if (db_path == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }
    return _tz_send_modified_command(TZ_FP_CMD_LOAD_USER_DB, (void *)db_path, strlen(db_path));
}

static int32_t fp_tz_remove_finger(uint32_t fid)
{
    return _tz_send_normal_command_1d(TZ_FP_CMD_FP_REMOVE, fid);
}

static int32_t fp_tz_get_db_count(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_GET_DB_COUNT);
}

static int32_t fp_tz_get_finger_prints(uint32_t *ids, uint32_t size)
{
    int32_t ret = 0;
    uint32_t count;

    if (ids == NULL || size == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _tz_send_modified_command_get_1r(TZ_FP_CMD_GET_FINGERPRINTS, (void *)ids, size*sizeof(uint32_t), &count);
    if (ret >= 0) {
        ret = count;
    }

    return ret;
}

static int64_t fp_tz_load_enroll_challenge(void)
{
    return fp_tz_get_int64_command(TZ_FP_CMD_LOAD_ENROLL_CHALLENGE, 0);
}

static int32_t fp_tz_set_enroll_challenge(uint64_t __unused challenge)
{
    return _tz_send_normal_command_2d(TZ_FP_CMD_SET_ENROLL_CHALLENGE, 0, 0);
}

static int32_t fp_tz_verify_enroll_challenge(const void* hat, uint32_t size)
{
    int32_t ret;
    void *buffer = NULL;

    if (hat == NULL || size == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    buffer = malloc(size);
    if (buffer == NULL) {
        LOG_MSG_ERROR("allocation failed");
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    memcpy(buffer, hat, size);
    ret = _tz_send_modified_command(TZ_FP_CMD_VERIFY_ENROLL_CHALLENGE, (void *)buffer, size);

    free(buffer);
    return ret;
}

static int64_t fp_tz_load_auth_id(void)
{
    return fp_tz_get_int64_command(TZ_FP_CMD_LOAD_AUTH_ID, 0);
}

static int32_t fp_tz_get_hw_auth_obj(void *buffer, uint32_t length)
{
    if (buffer == NULL || length == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }
    return _tz_send_modified_command_and_get(TZ_FP_CMD_GET_AUTH_OBJ, buffer, length);
}

static int32_t fp_tz_update_cfg(const void* buffer, uint32_t len)
{
    return _tz_send_modified_command(TZ_FP_CMD_UPDATE_CFG, (void *)buffer, len);
}

static int32_t fp_tz_init2(const void *buffer, const uint32_t len, uint32_t *feature, uint32_t *tpl_size)
{
    int32_t ret;
    uint32_t value1 = 0;
    void *ptoken = NULL;
    int32_t token_len = 0;

    if (buffer == NULL || len == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _tz_send_modified_command_2r(TZ_FP_CMD_INIT2, (void *)buffer, len, &value1, tpl_size);
    if (ret >= 0) {
        if (!(value1 & 0x80000000)) {
            token_len = silfp_ca_keymaster_get(&ptoken);
            if (len > 0) {
                _tz_send_modified_command(TZ_FP_CMD_SET_KEY_DATA, ptoken, token_len);
                free(ptoken);
            }
        }

        if (feature != NULL) {
            *feature = (value1 & 0x7FFFFFFF);
        }
    }

    return ret;
}

static int32_t fp_tz_load_template(uint32_t fid, const void *buffer, uint32_t len)
{
    if (buffer == NULL || len == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }
    return _tz_send_modified_command_1d(TZ_FP_CMD_LOAD_TEMPLATE, (void *)buffer, len, fid);
}

static int32_t fp_tz_save_template(void *buffer, uint32_t *plen)
{
    if (buffer == NULL || plen == NULL || *plen <= 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tz_send_modified_command_get_1r(TZ_FP_CMD_SAVE_TEMPLATE, buffer, *plen, plen);
}

static int32_t fp_tz_update_template(void *buffer, uint32_t *plen, uint32_t *fid)
{
    if (buffer == NULL || plen == NULL || fid == NULL || *plen <= 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tz_send_modified_command_get_2r(TZ_FP_CMD_UPDATE_TEMPLATE, buffer, *plen, plen, fid);
}

static int32_t fp_tz_set_log_mode(uint8_t tam, uint8_t agm)
{
    return _tz_send_normal_command_2d(TZ_FP_CMD_SET_LOG_MODE, tam, agm);
}

static int32_t fp_tz_set_nav_type(uint32_t mode)
{
    return _tz_send_normal_command_1d(TZ_FP_CMD_SET_NAV_TYPE, mode);
}

static int32_t fp_tz_get_finger_status(uint32_t *status, uint32_t *addition)
{
    return _tz_send_normal_command_2r(TZ_FP_CMD_GET_FINGER_STATUS, status, addition);
}

static int32_t fp_tz_get_config(void *buffer, uint32_t *plen)
{
    if (buffer == NULL || plen == NULL || *plen <= 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tz_send_modified_command_get_1r(TZ_FP_CMD_GET_CONFIG, buffer, *plen, plen);
}

static int32_t fp_tz_get_enroll_num(uint32_t *num)
{
    return _tz_send_normal_command_1r(TZ_FP_CMD_GET_ENROLL_NUM, num);
}

static int32_t fp_tz_calibrate(void *buffer, uint32_t len)
{
    return _tz_send_modified_command(TZ_FP_CMD_CALIBRATE, (void *)buffer, len);
}

static int32_t fp_tz_calibrate2(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_CALIBRATE2);
}

static int32_t fp_tz_ci_chk_finger(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_CI_CHK_FINGER);
}

static int32_t fp_tz_ci_adj_gain(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_CI_ADJ_GAIN);
}

static int32_t fp_tz_ci_shot(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_CI_SHOT);
}

int32_t fp_tz_alg_set_param(uint32_t cmd, void *buffer, uint32_t *plen, uint32_t *result)
{
    if (buffer == NULL || plen == NULL || *plen <= 0 || result == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tz_send_modified_command_get_1d_2r(TZ_FP_CMD_SET_ALG_PARAM, buffer, *plen, cmd, plen, result);
}

static int32_t fp_tz_chk_esd(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_CHECK_ESD);
}

static int32_t fp_tz_get_otp(uint32_t *otp1, uint32_t *otp2, uint32_t *otp3)
{
    return _tz_send_normal_command_3r(TZ_FP_CMD_GET_OTP, otp1, otp2, otp3);
}

static int32_t fp_tz_calibrate_optic(uint32_t step, uint8_t *buffer, uint32_t *plen)
{
    if (buffer == NULL || plen == NULL || *plen <= 0) {
        return -SL_ERROR_BAD_PARAMS;
    }
    return _tz_send_modified_command_1d_1r_and_get(TZ_FP_CMD_CALIBRATE_OPTIC, (void *)buffer, *plen, step, plen);
}

static int32_t fp_tz_get_version(uint32_t *algo, uint32_t *ta)
{
    return _tz_send_normal_command_2r(TZ_FP_CMD_GET_VERSIONS, algo, ta);
}

static int32_t fp_tz_get_chip_id(uint32_t *chipid, uint32_t *subid)
{
    return _tz_send_normal_command_2r(TZ_FP_CMD_GET_CHIPID, chipid, subid);
}

static int32_t fp_tz_test_get_image_info(uint32_t *w, uint32_t *h, uint32_t *size, uint32_t *w_ori, uint32_t *h_ori)
{
    int32_t ret;
    uint32_t data1 = 0;
    uint32_t data2 = 0;

    ret = _tz_send_normal_command_3r(TZ_FP_CMD_TEST_GET_IMG_INFO, &data1, &data2, size);
    if (ret >= 0) {
        if (h != NULL) {
            *h = (data1 & 0x0000FFFF);
        }
        if (w != NULL) {
            *w = (data1 >> 16 & 0x0000FFFF);
        }
        if (h_ori != NULL) {
            *h_ori = (data2 & 0x0000FFFF);
        }
        if (w_ori != NULL) {
            *w_ori = (data2 >> 16 & 0x0000FFFF);
        }
    }
    return ret;
}

static int32_t fp_tz_test_dump_data(uint32_t type, void *buffer, uint32_t len, uint32_t *size, uint32_t *w, uint32_t *h)
{
    int32_t ret;
    uint32_t data2 = 0;

    if (buffer == NULL || len == 0 || size == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _tz_send_modified_command_get_1d_2r(TZ_FP_CMD_TEST_DUMP_DATA, buffer, len, type, size, &data2);
    if (ret >= 0 && *size != 0) {
        if (w != NULL) {
            *w = (data2 & 0x0000FFFF);
        }
        if (h != NULL) {
            *h = (data2 >> 16 & 0x0000FFFF);
        }
    }

    return ret;
}

static int32_t fp_tz_test_image_capture(uint32_t gentpl, void *buffer, uint32_t *len, uint8_t *quality, uint8_t *area, uint8_t *istpl)
{
    int32_t ret;
    uint32_t data2 = 0;

    if (buffer == NULL || len == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _tz_send_modified_command_get_1d_2r_always(TZ_FP_CMD_TEST_IMAGE_CAPTURE, buffer, *len, gentpl, len, &data2);
    if (quality != NULL) {
        *quality = (data2 & 0x000000FF);
    }
    if (area != NULL) {
        *area = (data2 >> 8 & 0x000000FF);
    }
    if (istpl != NULL) {
        *istpl = (data2 >> 16) & 0x000000FF;
    }

    return ret;
}

static int32_t fp_tz_test_send_group_image(const uint32_t frr, void *buffer, uint32_t *plen)
{
    if (buffer == NULL || plen == NULL || *plen <= 0) {
        return -SL_ERROR_BAD_PARAMS;
    }
    return _tz_send_modified_command_1d_1r_and_get(TZ_FP_CMD_TEST_SEND_GP_IMG, (void *)buffer, *plen, frr, plen);
}

static int32_t fp_tz_test_image_finish(void)
{
    return _tz_send_normal_command(TZ_FP_CMD_TEST_IMAGE_FINISH);
}

static int32_t fp_tz_test_deadpx(uint32_t *result, uint32_t *deadpx, uint32_t *badline)
{
    if (result== NULL || deadpx == NULL || badline == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tz_send_normal_command_3r(TZ_FP_CMD_TEST_DEADPX, result, deadpx, badline);
}

int32_t fp_tz_test_speed(void *buffer, uint32_t *plen)
{
    if (buffer == NULL || plen == NULL || *plen <= 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tz_send_modified_command_get_1r(TZ_FP_CMD_TEST_SPEED, buffer, *plen, plen);
}

int32_t fp_tz_test_cmd(uint32_t cmd, void *buffer, uint32_t *plen, uint32_t *result)
{
    if (buffer == NULL || plen == NULL || *plen <= 0 || result == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tz_send_modified_command_get_1d_2r(TZ_FP_CMD_TEST_CMD, buffer, *plen, cmd, plen, result);
}

/*********************************************************************************/
static const silead_fp_handle_t s_callbacks = {
    fp_tz_wait_finger_status,
    fp_tz_capture_image,
    fp_tz_nav_capture_image,

    fp_tz_auth_start,
    fp_tz_auth_step,
    fp_tz_auth_end,

    fp_tz_enroll_start,
    fp_tz_enroll_step,
    fp_tz_enroll_end,

    fp_tz_nav_support,
    fp_tz_nav_start,
    fp_tz_nav_step,
    fp_tz_nav_end,

    fp_tz_download_normal,
    fp_tz_init,
    fp_tz_deinit,

    fp_tz_set_gid,
    fp_tz_load_user_db,
    fp_tz_remove_finger,
    fp_tz_get_db_count,
    fp_tz_get_finger_prints,

    fp_tz_load_enroll_challenge,
    fp_tz_set_enroll_challenge,
    fp_tz_verify_enroll_challenge,
    fp_tz_load_auth_id,
    fp_tz_get_hw_auth_obj,

    fp_tz_update_cfg,
    fp_tz_init2,

    fp_tz_load_template,
    fp_tz_save_template,
    fp_tz_update_template,

    fp_tz_set_log_mode,
    fp_tz_set_nav_type,
    fp_tz_get_finger_status,
    fp_tz_get_config,
    fp_tz_get_enroll_num,
    fp_tz_calibrate,
    fp_tz_calibrate2,
    fp_tz_ci_chk_finger,
    fp_tz_ci_adj_gain,
    fp_tz_ci_shot,
    fp_tz_alg_set_param,
    fp_tz_chk_esd,
    fp_tz_get_otp,
    fp_tz_calibrate_optic,

    fp_tz_get_version,
    fp_tz_get_chip_id,

    fp_tz_test_get_image_info,
    fp_tz_test_dump_data,

    fp_tz_test_image_capture,
    fp_tz_test_send_group_image,
    fp_tz_test_image_finish,

    fp_tz_test_deadpx,
    fp_tz_test_speed,
    fp_tz_test_cmd,
};

const silead_fp_handle_t * silfp_get_impl_handler(void)
{
    if (fp_tz_open() < 0) {
        fp_tz_deinit();
        return NULL;
    }

    return &s_callbacks;
}
