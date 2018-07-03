/******************************************************************************
 * @file   silead_test.c
 * @brief  Contains factory test functions.
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
 * Jack Zhang  2018/4/2    0.1.0      Init version
 * Jack Zhang  2018/5/17   0.1.1      Change test process to simplify app use
 *
 *****************************************************************************/

#define FILE_TAG "silead_test"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>

#include "silead_test.h"
#include "silead_error.h"
#include "silead_bmp.h"
#include "silead_version.h"
#include "silead_impl.h"
#include "silead_fingerext.h"
#include "silead_finger_internal.h"

#ifndef CHECK_DATA_MAGIC_SUPPORT
#define CHECK_DATA_MAGIC_SUPPORT 1
#endif

#define DATA_MAGIC_STRING "slfp"
#define DATA_MAGIC_LENGTH 4

static uint32_t m_test_image_w = 0;
static uint32_t m_test_image_h = 0;
static uint32_t m_test_image_w_ori = 0;
static uint32_t m_test_image_h_ori = 0;
static uint32_t m_test_image_buffer_size = 0;
static char *m_test_image_buffer = NULL;

int32_t silfp_test_deinit(void)
{
    if (m_test_image_buffer != NULL) {
        free(m_test_image_buffer);
        m_test_image_buffer = NULL;
    }
    m_test_image_w = 0;
    m_test_image_h = 0;
    m_test_image_w_ori = 0;
    m_test_image_h_ori = 0;
    m_test_image_buffer_size = 0;

    return SL_SUCCESS;
}

static uint32_t _test_get_bmp_report_size(uint32_t w, uint32_t h)
{
    return silfp_bmp_get_size(w, h) + 12;
}

int32_t silfp_test_data_verify(const void *buffer, uint32_t len, uint32_t *offset)
{
    int32_t ret = 0;
    uint32_t i = 0;
    const unsigned char *p = (const unsigned char *)buffer;
    uint32_t start = 0;

#if CHECK_DATA_MAGIC_SUPPORT
    if (p == NULL || len < DATA_MAGIC_LENGTH + start) {
        ret = -SL_ERROR_BAD_PARAMS;
    } else {
        for (i = 0; i < DATA_MAGIC_LENGTH; i++) {
            if ((p[start+i] & 0xFF) != DATA_MAGIC_STRING[i]) {
                break;
            }
        }
        if (i >= DATA_MAGIC_LENGTH) { // match
            if (offset) {
                *offset = start + DATA_MAGIC_LENGTH;
            }
            ret = DATA_MAGIC_LENGTH;
        } else {
            ret = -SL_ERROR_BAD_PARAMS;
        }
    }
#else
    if (offset) {
        *offset = start;
    }
    ret = 0;
#endif

    return ret;
}

int32_t silfp_test_set_err(void *buffer, uint32_t size, int32_t err)
{
    int32_t offset = 0;
    unsigned char *p = (unsigned char *)buffer;

    if (p != NULL && size >= 4) {
        if (err < 0) {
            err = -err;
        }

        // err code (4 Byte)
        p[offset++] = (err >> 24) & 0xFF;
        p[offset++] = (err >> 16) & 0xFF;
        p[offset++] = (err >> 8) & 0xFF;
        p[offset++] = err & 0xFF;
    }
    return offset;
}

static int32_t _test_set_version(void *buffer, uint32_t offset, uint32_t size, void *value, uint32_t vsize)
{
    uint32_t len = vsize;
    unsigned char *p = (unsigned char *)buffer;

    if (p == NULL) {
        return offset;
    }

    if (len + offset + 1 > size) {
        len = size - offset - 1;
    }
    p[offset++] = len;
    if (len > 0 && value != NULL) {
        memcpy(p + offset, value, len);
    }
    offset += len;
    if (offset >= size) {
        offset = size;
    }

    return offset;
}

static int32_t _test_set_versions(void *buffer, uint32_t offset, uint32_t size, void *value, uint32_t len, int32_t first)
{
    unsigned char *p = (unsigned char *)buffer;

    if (p != NULL && size > 4) {
        if (first != 0) {
            p[offset++] = 0;
            p[offset++] = 0;
            p[offset++] = 0;
            p[offset++] = 0;
        }
        offset = _test_set_version(buffer, offset, size, value, len);
    }
    return offset;
}

static int32_t _test_set_chipid(void *buffer, uint32_t size, uint32_t chipid, uint32_t subId)
{
    uint32_t offset = 0;
    const uint32_t chip_id_len = 21;
    char *p = (char *)buffer;

    if (p != NULL && size >= chip_id_len + 5) {
        // err code (4 Byte)
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;

        // chip id length (1 Byte)
        p[offset++] = chip_id_len;

        // chip id
        snprintf(p + offset, size - offset, "0x%08X,0x%08X", chipid, subId);
        offset += chip_id_len;
    }

    return offset;
}

static int32_t _test_set_img_data(void *buffer, uint32_t size, uint32_t offset, const void *data, uint32_t len, uint32_t w, uint32_t h)
{
    int32_t ret;
    uint32_t pos = offset;
    unsigned char *p = (unsigned char *)buffer;

    if (buffer == NULL || offset + 5 > size || data == NULL || (w * h > len)) {
        return offset;
    }

    // compatible legacy test
    //p[offset++] = IMG_TEST_FEATURE_IMG_DATA_MASK;
    //offset += 4;

    ret = silfp_bmp_get_img(p + offset, size - offset, data, w, h);
    if (ret > 0) {
        /*pos++;
        p[pos++] = (ret >> 24) & 0xFF;
        p[pos++] = (ret >> 16) & 0xFF;
        p[pos++] = (ret >> 8) & 0xFF;
        p[pos++] = ret & 0xFF;*/

        offset += ret;
    } else {
        offset = pos;
    }

    return offset;
}

static int32_t _test_set_img_data_extra(void *buffer, uint32_t size, uint32_t offset, uint8_t quality, uint8_t area, uint8_t istpl, uint8_t original)
{
    unsigned char *p = (unsigned char *)buffer;

    if (p != NULL && size > offset + 4) {
        //p[offset++] = IMG_TEST_FEATURE_IMG_QUALITY_MASK; // compatible legacy test
        p[offset++] = quality;
        p[offset++] = area;
        p[offset++] = original;
        p[offset++] = istpl;
    }

    return offset;
}

// compatible legacy test
static int32_t _test_set_img_data_count(void *buffer, uint32_t size, uint32_t offset, uint32_t count)
{
    unsigned char *p = (unsigned char *)buffer;

    if (p != NULL && size >= 4) {
        // err code (4 Byte)
        p[offset++] = (count >> 24) & 0xFF;
        p[offset++] = (count >> 16) & 0xFF;
        p[offset++] = (count >> 8) & 0xFF;
        p[offset++] = count & 0xFF;
    }
    return offset;
}

static int32_t _test_get_img_data(const void *buffer, const uint32_t size, const uint32_t offset, void *data, const uint32_t len, const uint32_t w, const uint32_t h)
{
    int32_t ret;
    const unsigned char *p = (const unsigned char *)buffer + offset;

    if (buffer == NULL || data == NULL || size <= offset || (w * h > len)) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = silfp_bmp_get_data(data, w, h, (void *)p, size - offset);

    return ret;
}

static int32_t _test_set_dead_pixel_result(void *buffer, uint32_t size, int32_t result, uint32_t deadpx, uint32_t badline)
{
    int32_t offset = 0;
    unsigned char *p = (unsigned char *)buffer;

    if (p != NULL && size >= 13) {
        // err code (4 Byte)
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;

        p[offset++] = result;
        p[offset++] = (deadpx >> 24) & 0xFF;
        p[offset++] = (deadpx >> 16) & 0xFF;
        p[offset++] = (deadpx >> 8) & 0xFF;
        p[offset++] = deadpx & 0xFF;
        p[offset++] = (badline >> 24) & 0xFF;
        p[offset++] = (badline >> 16) & 0xFF;
        p[offset++] = (badline >> 8) & 0xFF;
        p[offset++] = badline & 0xFF;
    }
    return offset;
}

static int32_t _test_set_result(void *buffer, uint32_t maxlen, const void *data, const uint32_t len)
{
    int32_t offset = 0;
    unsigned char *p = (unsigned char *)buffer;
    unsigned char *pdata = (unsigned char *)data;

    if (p != NULL && maxlen >= len + 4 && pdata != NULL && len >= 4) {
        // err code (4 Byte)
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;

        memcpy(p + offset, pdata, len);
        offset += len;
    }
    return offset;
}

// compatible legacy test
static int32_t _test_set_ffrfar_result(void *buffer, uint32_t maxlen, void *data, uint32_t len, uint32_t index)
{
    int32_t offset = 0;
    unsigned char *p = (unsigned char *)buffer;
    unsigned char *pdata = (unsigned char *)data;

    if (p != NULL && maxlen >= len + 8 && pdata != NULL && len >= 6) {
        // err code (4 Byte)
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;

        p[offset++] = (index >> 24) & 0xFF;
        p[offset++] = (index >> 16) & 0xFF;
        p[offset++] = (index >> 8) & 0xFF;
        p[offset++] = index & 0xFF;

        memcpy(p + offset, pdata, len);
        offset += len;
    }
    return offset;
}

int32_t silfp_test_get_versions(void *buffer, uint32_t size)
{
    char ver[32];
    uint32_t offset = 0;
    uint32_t algoVer, taVer;
    int32_t ret;

    do {
        // add hal version
        offset = _test_set_versions(buffer, offset, size, (char*)FP_HAL_VERSION, strlen(FP_HAL_VERSION), 1);
        if (offset >= size) {
            break;
        }

        // add dev version
        ret = sl_fp_get_dev_ver(ver, sizeof(ver));
        if (ret >= 0) {
            offset = _test_set_versions(buffer, offset, size, ver, strlen(ver), 0);
        } else {
            offset = _test_set_versions(buffer, offset, size, NULL, 0, 0);
        }
        if (offset >= size) {
            break;
        }

        //add algo & ta version
        ret = sl_fp_get_ta_ver(&algoVer, &taVer);
        if (ret >= 0) {
            memset(ver, 0, sizeof(ver));
            snprintf(ver, sizeof(ver), "v%d", algoVer);
            offset = _test_set_versions(buffer, offset, size, ver, strlen(ver), 0);

            memset(ver, 0, sizeof(ver));
            snprintf(ver, sizeof(ver), "v%d", taVer);
            offset = _test_set_versions(buffer, offset, size, ver, strlen(ver), 0);
        }
    } while (0);

    return offset;
}

int32_t silfp_test_get_chipid(void *buffer, uint32_t size)
{
    int32_t ret;
    uint32_t chipId = 0;
    uint32_t subId = 0;

    ret = sl_fp_get_chipid(&chipId, &subId);
    if (ret >= 0) {
        return _test_set_chipid(buffer, size, chipId, subId);
    }

    return 0;
}

int32_t silfp_test_reset_test(void *buffer, uint32_t size)
{
    int32_t ret;

    ret = sl_fp_reset_test();
    if (ret >= 0) {
        return silfp_test_set_err(buffer, size, 0);
    }
    return 0;
}

int32_t silfp_test_get_image_size(void)
{
    int32_t ret;
    uint32_t size = 0;

    ret = sl_fp_test_get_image_info(&m_test_image_w, &m_test_image_h, &size, &m_test_image_w_ori, &m_test_image_h_ori);
    if (size == 0 || m_test_image_w == 0 || m_test_image_h == 0) {
        LOG_MSG_DEBUG("get the config from ta is invalid");
        ret = -SL_ERROR_CONFIG_INVALID;
    }

    if (m_test_image_w_ori == 0) {
        m_test_image_w_ori = m_test_image_w;
    }
    if (m_test_image_h_ori == 0) {
        m_test_image_h_ori = m_test_image_h;
    }

    if (ret >= 0) {
        if (m_test_image_buffer == NULL) {
            m_test_image_buffer_size = size;
            m_test_image_buffer = (char *)malloc(m_test_image_buffer_size);
            if (m_test_image_buffer == NULL) {
                m_test_image_buffer_size = 0;
                ret = -SL_ERROR_OUT_OF_MEMORY;
            }
        }
    }

    if (ret >= 0) {
        ret = _test_get_bmp_report_size(m_test_image_w_ori, m_test_image_h_ori);
    }

    return ret;
}

static int32_t _test_get_img_raw_buffer()
{
    int32_t ret;

    if (m_test_image_buffer != NULL) {
        return 0;
    }

    ret = silfp_test_get_image_size();

    return ret;
}

int32_t silfp_test_image_capture(uint32_t feature, void *buffer, uint32_t size)
{
    int32_t ret;
    int32_t offset = 0;
    uint32_t len;
    uint8_t quality = 0;
    uint8_t area = 0;
    uint8_t istpl = 0;
    uint8_t original = !!(IMG_TEST_FEATURE_ORIGINAL_MASK & feature);

    ret = _test_get_img_raw_buffer();
    if (ret < 0) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    len = m_test_image_buffer_size;
    ret = sl_fp_test_image_capture(feature, m_test_image_buffer, &len, &quality, &area, &istpl);
    if (ret < 0) {
        if (original) {
            offset = silfp_test_set_err(buffer, size, 0);
        } else {
            offset = silfp_test_set_err(buffer, size, ret);
        }
    } else {
        offset = silfp_test_set_err(buffer, size, 0);
    }

    offset += 4; // compatible legacy test
    offset = _test_set_img_data_extra(buffer, size, offset, quality, area, istpl, original);

    if (feature & IMG_TEST_FEATURE_IMG_DATA_MASK) {
        if (original) {
            LOG_MSG_DEBUG("original w:%u, h:%u, size:%u", m_test_image_w_ori, m_test_image_h_ori, len);
            offset = _test_set_img_data(buffer, size, offset, (void *)m_test_image_buffer, len, m_test_image_w_ori, m_test_image_h_ori);
        } else {
            LOG_MSG_DEBUG("w:%u, h:%u, size:%u", m_test_image_w, m_test_image_h, len);
            offset = _test_set_img_data(buffer, size, offset, (void *)m_test_image_buffer, len, m_test_image_w, m_test_image_h);
        }
    }

    // compatible legacy test
    _test_set_img_data_count(buffer, size, 4, offset - 8);

    return offset;
}

int32_t silfp_test_frrfar_send_group_image(const void *buffer, uint32_t offset, const uint32_t len, void *rsp, const uint32_t maxlen)
{
    int32_t ret = 0;
    uint32_t frr = 0;
    uint32_t size;
    const unsigned char *p = (const unsigned char *)buffer;

    ret = _test_get_img_raw_buffer();
    if (ret < 0) {
        return ret;
    }

    if (p != NULL && len >= offset + 1) {
        frr = (p[offset++] & 0x000000FF);
        ret = _test_get_img_data(buffer, len, offset, m_test_image_buffer, m_test_image_buffer_size, m_test_image_w, m_test_image_h);
        if (ret > 0 && ret <= (int32_t)m_test_image_buffer_size) {
            size = ret;
            ret = sl_fp_test_frrfar_send_group_image(frr, m_test_image_buffer, &size);
        } else {
            LOG_MSG_DEBUG("parse img data failed (%d)", ret);
            ret = -SL_ERROR_BAD_PARAMS;
        }
    } else {
        LOG_MSG_DEBUG("cmd buffer is invalid");
        ret = -SL_ERROR_BAD_PARAMS;
    }

    if (ret >= 0 && size > 0 && size <= m_test_image_buffer_size) {
        // compatible legacy test
        //ret = _test_set_result(rsp, maxlen, m_test_image_buffer, size);
        ret = _test_set_ffrfar_result(rsp, maxlen, m_test_image_buffer, size, 0);
    }

    return ret;
}

int32_t silfp_test_image_finish(void)
{
    sl_fp_test_image_finish();
    silfp_test_deinit();

    return SL_SUCCESS;
}

int32_t silfp_test_deadpx(void *buffer, uint32_t size)
{
    int32_t ret;
    uint32_t result = 0;
    uint32_t deadpx = 0;
    uint32_t badline = 0;

    ret = sl_fp_test_deadpx(&result, &deadpx, &badline);
    if (ret >= 0) {
        ret = _test_set_dead_pixel_result(buffer, size, result, deadpx, badline);
    }

    return ret;
}

int32_t silfp_test_speed(void *buffer, uint32_t size)
{
    int32_t ret;
    char response[256];
    uint32_t len = sizeof(response);

    ret = sl_fp_test_speed(response, &len);
    if (len == 0 || len > sizeof(response)) {
        ret = -SL_ERROR_BAD_PARAMS;
    }

    if (ret >= 0 ) {
        ret = _test_set_result(buffer, size, response, len);
    }

    return ret;
}

int32_t silfp_test_calibrate(void)
{
    return sl_fp_calibrate();
}

inline int32_t silfp_test_calibrate_step(uint32_t step)
{
    int32_t ret = 0;
    switch (step) {
        case FUN_CALIBRATE_CMD1:
            sl_fp_set_hbm_mode(1);
            ret = sl_fp_calibrate_step(step);
            sl_fp_set_hbm_mode(0);
            break;
        case FUN_CALIBRATE_CMD2:
            sl_fp_set_hbm_mode(1);
            ret = sl_fp_calibrate_step(step);
            sl_fp_set_hbm_mode(0);
            break;
        case FUN_CALIBRATE_CMD3:
            sl_fp_set_hbm_mode(0);
            sl_fp_set_brightness(BRIGHTNESS_ALL);
            ret = sl_fp_calibrate_step(step);
            break;
        case FUN_CALIBRATE_CMD4:
            ret = sl_cb_optic_test_factory_quality();
        //silfp_finger_set_work_state_no_signal(STATE_IDLE);			
        case FUN_CALIBRATE_CMD5:
        default: {
            LOG_MSG_DEBUG("test %d not implement", step);
            break;
        }
    }
    silfp_finger_set_work_state_no_signal(STATE_IDLE);	
    return ret;
}

int32_t silfp_test_set_err_with_byte(void *buffer, uint32_t size, int32_t err, int32_t data)
{
    int32_t offset = 0;
    unsigned char *p = (unsigned char *)buffer;

    if (p != NULL && size >= 5) {
        if (err < 0) {
            err = -err;
        }

        // err code (4 Byte)
        p[offset++] = (err >> 24) & 0xFF;
        p[offset++] = (err >> 16) & 0xFF;
        p[offset++] = (err >> 8) & 0xFF;
        p[offset++] = err & 0xFF;
        p[offset++] = data & 0xFF;
    }
    return offset;
}