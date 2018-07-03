/******************************************************************************
 * @file   silead_fingerext.c
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
 * David Wang  2018/4/2    0.1.0      Init version
 * David Wang  2018/5/15   0.1.1      Support get finger status
 * Jack Zhang  2018/5/17   0.1.2      Change test process to simplify app use
 *
 *****************************************************************************/

#define FILE_TAG "FINGEREXT"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_impl.h"
#include "silead_error.h"
#include "silead_test.h"
#include "silead_fingerext_hub.h"
#include "silead_finger_internal.h"
#include "silead_fingerext.h"

typedef struct _test_cmd_data {
    uint32_t cmd_id;
    int32_t req_len;
    int32_t offset;
    uint8_t *req;
} test_cmd_data_t;

typedef struct _test_cmd_image_reponse {
    int32_t rsp_len;
    uint8_t *rsp;
} test_cmd_image_reponse_t;

pthread_mutex_t m_test_data_lock;
test_cmd_data_t m_test_cmd_data;
test_cmd_image_reponse_t m_test_cmd_img_reponse;

int32_t silfp_ext_start(void)
{
    memset(&m_test_cmd_data, 0, sizeof(m_test_cmd_data));
    memset(&m_test_cmd_img_reponse, 0, sizeof(m_test_cmd_img_reponse));
    pthread_mutex_init(&m_test_data_lock, NULL);

    silfp_ext_hub_start();

    return 0;
}

int32_t silfp_ext_stop(void)
{
    silfp_ext_hub_stop();
    silfp_test_deinit();

    if (m_test_cmd_data.req) {
        free(m_test_cmd_data.req);
        m_test_cmd_data.req = NULL;
    }
    if (m_test_cmd_img_reponse.rsp) {
        free(m_test_cmd_img_reponse.rsp);
        m_test_cmd_img_reponse.rsp = NULL;
    }
    pthread_mutex_destroy(&m_test_data_lock);

    return 0;
}

static inline int32_t _ext_send_response(uint32_t cmdid, const void *data, size_t data_size)
{
    return silfp_ext_hub_send_response_raw(cmdid, data, data_size);
}

static void _ext_send_test_error_notice(int32_t cmd, int32_t err)
{
    char buffer[32];
    int32_t len = silfp_test_set_err(buffer, sizeof(buffer), err);
    _ext_send_response(cmd, (int8_t *)buffer, len);
}

static int32_t _ext_test_cmd_finger_down()
{
    sl_fp_sync_finger_status_optic(1);
    return 1;
}

static int32_t _ext_test_cmd_finger_up()
{
    sl_fp_sync_finger_status_optic(0);
    return 1;
}

int32_t silfp_ext_test_cmd(const uint8_t *param, int32_t len)
{
    int32_t ret = SL_SUCCESS;
    uint32_t cmd_id = -1;
    int32_t offset = 0;

    if (len < 4) {
        ret = -SL_ERROR_BAD_PARAMS;
        _ext_send_test_error_notice(cmd_id, ret);
        return ret;
    }

    cmd_id = ((param[offset++] << 24) & 0xFF000000);
    cmd_id |= ((param[offset++] << 16) & 0x00FF0000);
    cmd_id |= ((param[offset++] << 8) & 0x0000FF00);
    cmd_id |= (param[offset++] & 0x000000FF);

    len -= offset;

    if (TEST_CMD_SEND_FINGER_DOWN == cmd_id) {
        _ext_test_cmd_finger_down();
        return 0;
    } else if (TEST_CMD_SEND_FINGER_UP == cmd_id) {
        _ext_test_cmd_finger_up();
        return 0;
    }

    pthread_mutex_lock(&m_test_data_lock);
    do {
        if (m_test_cmd_data.req_len < len) { // not have enough buffer
            if (m_test_cmd_data.req) {
                free(m_test_cmd_data.req);
                m_test_cmd_data.req = NULL;
            }
        }
        m_test_cmd_data.cmd_id = cmd_id;
        m_test_cmd_data.req_len = len;
        m_test_cmd_data.offset = offset;

        if (len > 0) {
            if (!m_test_cmd_data.req) {
                m_test_cmd_data.req = (uint8_t *)malloc(len);
            }
            if (!m_test_cmd_data.req) {
                m_test_cmd_data.req_len = 0;
                ret = -SL_ERROR_OUT_OF_MEMORY;
                break;
            }
            memcpy(m_test_cmd_data.req, param + offset, len);
        }
    } while (0);
    pthread_mutex_unlock(&m_test_data_lock);

    if (ret < 0) {
        LOG_MSG_DEBUG("test error cmd_id:%d, err:%d", cmd_id, ret);
        _ext_send_test_error_notice(cmd_id, ret);
    } else {
        silfp_finger_set_work_state(STATE_TEST);
    }

    return ret;
}


static inline int32_t _ext_do_get_version(void)
{
    char buffer[256];
    int32_t len = 0;

    len = silfp_test_get_versions(buffer, sizeof(buffer));
    _ext_send_response(TEST_CMD_GET_VERSION, (int8_t *)buffer, len);

    return 0;
}

static inline int32_t _ext_do_get_chipid(void)
{
    char buffer[256];
    int32_t len = 0;

    len = silfp_test_get_chipid(buffer, sizeof(buffer));
    _ext_send_response(TEST_CMD_SPI, (int8_t *)buffer, len);

    return 0;
}

static inline int32_t _ext_do_reset(void)
{
    char buffer[256];
    int32_t len = 0;

    len = silfp_test_reset_test(buffer, sizeof(buffer));
    _ext_send_response(TEST_CMD_TEST_RESET_PIN, (int8_t *)buffer, len);

    return 0;
}

static int32_t _ext_do_test_get_image(void)
{
    int32_t ret;
    uint32_t feature = IMG_TEST_FEATURE_GEN_TPL_MASK | IMG_TEST_FEATURE_IMG_DATA_MASK | IMG_TEST_FEATURE_IMG_QUALITY_MASK;
    uint32_t feature_ori = IMG_TEST_FEATURE_ORIGINAL_MASK | IMG_TEST_FEATURE_IMG_DATA_MASK | IMG_TEST_FEATURE_IMG_QUALITY_MASK;

    ret = silfp_test_get_image_size();
    if (m_test_cmd_img_reponse.rsp == NULL) {
        if (ret > 0) {
            m_test_cmd_img_reponse.rsp_len = ret;
            m_test_cmd_img_reponse.rsp = (uint8_t *)malloc(m_test_cmd_img_reponse.rsp_len);
        }
        if (m_test_cmd_img_reponse.rsp == NULL) {
            m_test_cmd_img_reponse.rsp_len = 0;
            ret = -SL_ERROR_OUT_OF_MEMORY;
        }
    }
    if (ret < 0) {
        _ext_send_test_error_notice(TEST_CMD_TEST_IMAGE_CAPTURE, ret);
        return ret;
    }

    sl_fp_set_hbm_mode(1);
    do {
        ret = sl_fp_get_finger_down();
        if (ret < 0) {
            continue;
        }

        sileadHypnusSetAction();

        ret = sl_fp_ci_chk_finger();
        if (ret < 0) {
            continue;
        }

        do {
            ret = sl_fp_ci_adj_gain();
        } while(ret > 0);
        if (ret < 0) {
            continue;
        }

        ret = silfp_test_image_capture(feature_ori, (char *)m_test_cmd_img_reponse.rsp, m_test_cmd_img_reponse.rsp_len);
        if (ret >= 0) {
            _ext_send_response(TEST_CMD_TEST_IMAGE_CAPTURE, (int8_t *)m_test_cmd_img_reponse.rsp, ret);
        } else {
            _ext_send_test_error_notice(TEST_CMD_TEST_IMAGE_CAPTURE, ret);
        }

        if (ret >= 0) {
            ret = sl_fp_ci_shot();
            if (ret >= 0) {
                ret = silfp_test_image_capture(feature, (char *)m_test_cmd_img_reponse.rsp, m_test_cmd_img_reponse.rsp_len);
                if (ret >= 0) {
                    _ext_send_response(TEST_CMD_TEST_IMAGE_CAPTURE, (int8_t *)m_test_cmd_img_reponse.rsp, ret);
                } else {
                    _ext_send_test_error_notice(TEST_CMD_TEST_IMAGE_CAPTURE, ret);
                }
            }
        }

        if (ret != -SL_ERROR_CANCELED && !silfp_finger_is_canceled()) {
            sl_fp_get_finger_up();
        }
    } while (!silfp_finger_is_canceled());
    sl_fp_set_hbm_mode(0);

    if (ret == -SL_ERROR_CANCELED || silfp_finger_is_canceled()) {
        silfp_test_image_finish();
    }

    sl_fp_get_finger_up();

    return 0;
}

static int32_t _ext_do_test_frrfar_send_image(void)
{
    int32_t ret;
    char buffer[256];
    int32_t len = 0;

    pthread_mutex_lock(&m_test_data_lock);
    m_test_cmd_data.offset += 4; // compatible legacy test
    ret = silfp_test_frrfar_send_group_image((const char *)m_test_cmd_data.req, m_test_cmd_data.offset, m_test_cmd_data.req_len, (char *)buffer, sizeof(buffer));
    len = ret;
    pthread_mutex_unlock(&m_test_data_lock);

    if (ret >= 0) {
        _ext_send_response(TEST_CMD_FRR_FAR_SEND_IMAGE, (int8_t *)buffer, len);
    } else {
        _ext_send_test_error_notice(TEST_CMD_FRR_FAR_SEND_IMAGE, ret);
    }

    return 0;
}

static inline int32_t _ext_do_test_send_image_clear(void)
{
    int32_t ret;

    ret = silfp_test_image_finish();
    _ext_send_test_error_notice(TEST_CMD_FRR_FAR_SEND_IMAGE_CLEAR, 0);

    return ret;
}

static int32_t _ext_do_test_deadpixel(void)
{
    int32_t ret;
    char buffer[256];

    ret = silfp_test_deadpx(buffer, sizeof(buffer));
    if (ret >= 0) {
        _ext_send_response(TEST_CMD_TEST_DEAD_PIXEL, (int8_t *)buffer, ret);
    } else {
        _ext_send_test_error_notice(TEST_CMD_TEST_DEAD_PIXEL, ret);
    }

    return 0;
}

static int32_t _ext_do_test_speed(void)
{
    int32_t ret;
    char buffer[256];

    sl_fp_set_hbm_mode(1);
    do {
        ret = sl_fp_get_finger_down();
        if (ret < 0) {
            continue;
        }

        ret = silfp_test_speed(buffer, sizeof(buffer));
        if (ret >= 0) {
            _ext_send_response(TEST_CMD_TEST_SPEED, (int8_t *)buffer, ret);
        } else {
            _ext_send_test_error_notice(TEST_CMD_TEST_SPEED, ret);
        }

        if (ret != -SL_ERROR_CANCELED && !silfp_finger_is_canceled()) {
            sl_fp_get_finger_up();
        }
    } while (!silfp_finger_is_canceled());
    sl_fp_set_hbm_mode(0);

    return 0;
}

static inline int32_t _ext_do_test_image_finish(void)
{
    int32_t ret;

    ret = silfp_test_image_finish();
    _ext_send_test_error_notice(TEST_CMD_TEST_FINISH, 0);

    return ret;
}

static inline int32_t _ext_do_test_calibrate(void)
{
    int32_t ret;

    ret = silfp_test_calibrate();
    if (ret >= 0) {
        _ext_send_test_error_notice(TEST_CMD_CALIBRATE, 0);
    } else {
        _ext_send_test_error_notice(TEST_CMD_CALIBRATE, ret);
    }

    return ret;
}

static inline int32_t _ext_do_test_calibrate_step(void)
{
    int32_t ret = 0;
    int32_t step = -1;
    char buffer[256];

    pthread_mutex_lock(&m_test_data_lock);
    if (m_test_cmd_data.req_len >= m_test_cmd_data.offset + 1) {
        step = (m_test_cmd_data.req[m_test_cmd_data.offset] & 0xFF);
    } else {
        ret = -SL_ERROR_BAD_PARAMS;
    }
    pthread_mutex_unlock(&m_test_data_lock);

    if (ret >= 0) {
        ret = silfp_test_calibrate_step(step);
    }

    if (step >= 0) {
        ret = silfp_test_set_err_with_byte(buffer, sizeof(buffer), ret, step);
    }

    if (ret > 0) {
        _ext_send_response(TEST_CMD_CALIBRATE_STEP, (int8_t *)buffer, ret);
    } else {
        _ext_send_test_error_notice(TEST_CMD_CALIBRATE_STEP, ret);
    }

    return 0;
}

static inline int32_t _ext_should_goto_wait_status(uint32_t cmd)
{
    int32_t wait = 0;
    switch (cmd) {
    case TEST_CMD_FRR_FAR_SEND_IMAGE:
    case TEST_CMD_FRR_FAR_SEND_IMAGE_CLEAR:
        wait = 1;
        break;
    }
    return wait;
}

static inline int32_t _ext_should_loop(uint32_t cmd)
{
    int32_t loop = 0;
    switch (cmd) {
    case TEST_CMD_TEST_IMAGE_CAPTURE:
    case TEST_CMD_TEST_SPEED:
        loop = 1;
        break;
    }
    return loop;
}

int32_t silfp_ext_request_commond()
{
    int32_t ret = SL_SUCCESS;
    uint32_t cmd;

    pthread_mutex_lock(&m_test_data_lock);
    cmd = m_test_cmd_data.cmd_id;

    ret = silfp_test_data_verify((char *)m_test_cmd_data.req, m_test_cmd_data.req_len, (uint32_t*)&m_test_cmd_data.offset);
    if (ret < 0) {
        _ext_send_test_error_notice(m_test_cmd_data.cmd_id, ret);

        if (ret != -SL_ERROR_CANCELED && !silfp_finger_is_canceled()) {
            LOG_MSG_DEBUG("verify data failed, goto idle status");
            silfp_finger_set_work_state_no_signal(STATE_IDLE);
        }
    }
    pthread_mutex_unlock(&m_test_data_lock);

    if (ret < 0 || silfp_finger_is_canceled()) {
        return 0;
    }

    LOG_MSG_DEBUG("test------------- %d", cmd);

    switch (cmd) {
    case TEST_CMD_TEST_IMAGE_CAPTURE: {
        _ext_do_test_get_image();
        break;
    }
    case TEST_CMD_FRR_FAR_SEND_IMAGE: {
        _ext_do_test_frrfar_send_image();
        break;
    }
    case TEST_CMD_FRR_FAR_SEND_IMAGE_CLEAR: {
        _ext_do_test_send_image_clear();
        break;
    }
    case TEST_CMD_GET_VERSION: {
        _ext_do_get_version();
        break;
    }
    case TEST_CMD_SPI: {
        _ext_do_get_chipid();
        break;
    }
    case TEST_CMD_TEST_RESET_PIN: {
        _ext_do_reset();
        break;
    }
    case TEST_CMD_TEST_DEAD_PIXEL: {
        _ext_do_test_deadpixel();
        break;
    }
    case TEST_CMD_TEST_SPEED: {
        _ext_do_test_speed();
        break;
    }
    case TEST_CMD_TEST_FINISH: {
        _ext_do_test_image_finish();
        break;
    }
    case TEST_CMD_CALIBRATE: {
        _ext_do_test_calibrate();
        break;
    }
    case TEST_CMD_CALIBRATE_STEP: {
        _ext_do_test_calibrate_step();
        break;
    }
    default: {
        _ext_send_test_error_notice(cmd, 0);
        break;
    }
    }
    LOG_MSG_DEBUG("test finish------------- %d", ret);

    if (ret != -SL_ERROR_CANCELED && !silfp_finger_is_canceled()) {
        if (_ext_should_goto_wait_status(cmd)) {
            silfp_finger_set_work_state_no_signal(STATE_WAIT);
        } else if (!_ext_should_loop(cmd)) {
            silfp_finger_set_work_state_no_signal(STATE_IDLE);
        }
    }

    return 0;
}