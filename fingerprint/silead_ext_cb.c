/******************************************************************************
 * @file   silead_ext_cb.c
 * @brief  Contains fingerprint extension operate functions.
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
 * Jack Zhang  2018/5/18    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_ext_cb"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "silead_error.h"
#include "silead_impl.h"
#include "silead_finger_internal.h"

#define MODULE_SILEAD "silead"

#define IMG_TEST_FEATURE_IMG_QUALITY_MASK 0x00000004

typedef enum {
    TEST_CMD_SPI = 0,
    TEST_IMAGE_QUALITY_GET,
    TEST_IMAGE_CAPTURE_LOOP,
    TEST_CALIBRATE_STEP,
    TEST_OPTIC_TEST_FACTORY_QUALITY,
} e_ext_cb_id_t;

static e_ext_cb_id_t m_cmd_id;
static int32_t m_calibrate_cmd_id = 0;

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
void silfp_send_engineering_image_quality_notice(int32_t successed, int32_t image_quality);

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
void silfp_send_fingerprint_cmd_notice(int32_t cmd_id, int32_t ret);
void silfp_send_fingerprint_cmd_3v_notice(int32_t cmd_id, int32_t ret, int32_t v, int32_t v2, int32_t v3);
#endif
#endif

static pthread_mutex_t m_lock;
static pthread_cond_t m_worker_cond;
static int32_t m_spi_test_result = 0;

int32_t silfp_ext_cb_init()
{
    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_worker_cond, NULL);
    return 0;
}

int32_t silfp_ext_cb_deinit()
{
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_worker_cond);
    return 0;
}

int32_t silfp_ext_cb_get_module(char *buffer, uint32_t size)
{
    int32_t len = 0;
    if (buffer != NULL && size > 0) {
        if (size > strlen(MODULE_SILEAD)) {
            len = strlen(MODULE_SILEAD);
        } else {
            len = size - 1;
        }
        memcpy(buffer, MODULE_SILEAD, len);
        buffer[len] = '\0';
    }
    return len;
}

int32_t silfp_ext_cb_spi_test()
{
    int32_t ret;
    struct timeval now;
    struct timespec outtime;

    LOG_MSG_DEBUG("spi test");

    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + 1;
    outtime.tv_nsec = now.tv_usec * 1000;

    pthread_mutex_lock(&m_lock);

    m_spi_test_result = 0;
    m_cmd_id = TEST_CMD_SPI;
    silfp_finger_set_work_state(STATE_EXT_CB);
    pthread_cond_timedwait(&m_worker_cond, &m_lock, &outtime);
    ret = m_spi_test_result;
    pthread_mutex_unlock(&m_lock);

    LOG_MSG_DEBUG("result (%d)", ret);
    return (ret == 1)? 0 : 1;
}

int32_t silfp_ext_cb_image_quality_get()
{
    LOG_MSG_DEBUG("image quality get");
    m_cmd_id = TEST_IMAGE_QUALITY_GET;
    silfp_finger_set_work_state(STATE_EXT_CB);
    return 0;
}

int32_t silfp_ext_cb_image_quality_finish()
{
    LOG_MSG_DEBUG("image quality finish");
    silfp_finger_set_work_state(STATE_IDLE);
    return 0;
}

int32_t silfp_ext_cb_image_capture_loop()
{
    LOG_MSG_DEBUG("image capture test");
    m_cmd_id = TEST_IMAGE_CAPTURE_LOOP;
    silfp_finger_set_work_state(STATE_EXT_CB);
    return 0;
}

int32_t silfp_ext_cb_calibrate_step(int32_t cmd_id)
{
    LOG_MSG_DEBUG("calibrate cmd %d", cmd_id);
    m_cmd_id = TEST_CALIBRATE_STEP;
    m_calibrate_cmd_id = cmd_id;
    silfp_finger_set_work_state(STATE_EXT_CB);
    return 0;
}

int32_t silfp_et_cb_optic_test_factory_quality()
{
    LOG_MSG_DEBUG("optic quality test");
    m_cmd_id = TEST_OPTIC_TEST_FACTORY_QUALITY;
    silfp_finger_set_work_state(STATE_EXT_CB);
    return 0;
}

static int32_t _ext_cb_spi_test()
{
    int32_t ret;
    uint32_t chipId = 0;
    uint32_t subId = 0;
    uint32_t result;
    uint32_t deadpx;
    uint32_t badline;

    ret = sl_fp_get_chipid(&chipId, &subId);
    if (ret < 0) {
        LOG_MSG_DEBUG("spi test failed (%d)", ret);
        ret = 0;
    } else {
        LOG_MSG_DEBUG("spi test successed (%x,%x)", chipId, subId);
        ret = sl_fp_test_deadpx(&result, &deadpx, &badline);
        LOG_MSG_DEBUG("sl_fp_test_deadpx: result=%d, deadpx=%d, badline=%d", result, deadpx, badline);
        if (ret >= 0 && result == 0) {
            ret = 1;
        } else {
            ret = 0;
        }
    }

    pthread_mutex_lock(&m_lock);
    m_spi_test_result = ret;
    pthread_cond_signal(&m_worker_cond);
    pthread_mutex_unlock(&m_lock);

    return 0;
}

static int32_t _ext_cb_image_quality_get()
{
    int32_t ret;
    uint8_t quality = 0;

    char temp[4];
    uint32_t len;

    uint32_t feature = IMG_TEST_FEATURE_IMG_QUALITY_MASK;

    ret = sl_fp_get_finger_down();
    sl_fp_set_hbm_mode(1);
    if (ret >= 0) {
        ret = sl_fp_capture_image(0);
    }

    if (ret >= 0) {
        len = 4;
        ret = sl_fp_test_image_capture(feature, temp, &len, &quality, NULL, NULL);
    }

    if (ret < 0) {
        LOG_MSG_DEBUG("get image quality failed (%d)", ret);
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
        silfp_send_engineering_image_quality_notice(1, quality);
#endif
    } else {
        LOG_MSG_DEBUG("get image quality successed (%d)", quality);
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
        silfp_send_engineering_image_quality_notice(1, quality);
#endif
    }
    sl_fp_set_hbm_mode(0);
    sl_fp_get_finger_up_uncancelable();

    return 0;
}

int32_t _ext_cb_image_capture_loop()
{
    int32_t ret = 0;

    sl_fp_download_normal();
    sl_fp_ci_chk_finger();
    do {
        sileadHypnusSetAction();

        sl_fp_set_hbm_mode(1);
        do {
            ret = sl_fp_ci_adj_gain();
        } while(ret > 0);
        sl_fp_set_hbm_mode(0);

        silfp_send_fingerprint_cmd_notice(FUN_AGING_TEST, 0);

        if (!silfp_finger_is_canceled()) {
            silfp_finger_wait_work_condition(1);
        }
    } while (!silfp_finger_is_canceled());

    return 0;
}

int32_t _ext_cb_calibrate_step(int32_t cmd_id)
{
    int32_t ret = 0;
    switch (cmd_id) {
        case FUN_CALIBRATE_CMD1:
            sl_fp_set_hbm_mode(1);
            ret = sl_fp_calibrate_step(cmd_id);
            sl_fp_set_hbm_mode(0);
            break;
        case FUN_CALIBRATE_CMD2:
            sl_fp_set_hbm_mode(1);
            ret = sl_fp_calibrate_step(cmd_id);
            sl_fp_set_hbm_mode(0);
            break;
        case FUN_CALIBRATE_CMD3:
            sl_fp_set_hbm_mode(0);
            sl_fp_set_brightness(BRIGHTNESS_ALL);
            ret = sl_fp_calibrate_step(cmd_id);
            break;
        case FUN_CALIBRATE_CMD4:
        case FUN_CALIBRATE_CMD5:
        case FUN_CALIBRATE_CMD6:
        case FUN_CALIBRATE_CMD7:
        case FUN_CALIBRATE_CMD8:
        case FUN_CALIBRATE_CMD9:
        case FUN_CALIBRATE_CMD10:
            LOG_MSG_DEBUG("test %d not implement", cmd_id);
            break;
    }
    silfp_finger_set_work_state_no_signal(STATE_IDLE);
    silfp_send_fingerprint_cmd_notice(cmd_id, ret);
    return 0;
}

int32_t _ext_cb_optic_test_factory_quality()
{
    int32_t ret = 0;
    int32_t snr_ret = 0;
    uint32_t snr = 0;
    uint32_t noise = 0;
    uint32_t signal = 0;
    uint32_t qa_result = 0;
    uint32_t quality  = 0;
    uint32_t length = 0;

    sl_fp_set_hbm_mode(1);
    sl_fp_download_normal();

    ret = sl_fp_optic_test_factory_quality(&qa_result, &quality, &length);
    if (qa_result == 0 && ret >= 0) {
        ret = 0;
    } else {
        ret = 1;
        LOG_MSG_ERROR("ret: %d, result: %u, quality:%u, length:%u", ret, qa_result, quality, length);
    }

    snr_ret = sl_fp_optic_test_snr(&snr, &noise, &signal);
    if (ret == 0 && snr_ret < 0) { // quality test pass, snr fail, return fail
        ret = 1;
    }
    sl_fp_set_hbm_mode(0);

    silfp_send_fingerprint_cmd_3v_notice(FUN_FINGERPRINT_TEST1, ret, snr, noise, signal);
    return 0;
}

int32_t silfp_ext_cb_command()
{
    switch (m_cmd_id) {
    case TEST_CMD_SPI: {
        _ext_cb_spi_test();
        silfp_finger_set_work_state_no_signal(STATE_IDLE);
        break;
    }
    case TEST_IMAGE_QUALITY_GET: {
        _ext_cb_image_quality_get();
        break;
    }
    case TEST_IMAGE_CAPTURE_LOOP: {
        _ext_cb_image_capture_loop();
        break;
    }
    case TEST_CALIBRATE_STEP: {
        _ext_cb_calibrate_step(m_calibrate_cmd_id);
        break;
    }
    case TEST_OPTIC_TEST_FACTORY_QUALITY: {
        _ext_cb_optic_test_factory_quality();
        silfp_finger_set_work_state_no_signal(STATE_IDLE);
        break;
    }
    }
    return 0;
}