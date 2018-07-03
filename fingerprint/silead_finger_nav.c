/******************************************************************************
 * @file   silead_finger_nav.c
 * @brief  Contains fingerprint navi operate functions.
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
 * Jack Zhang  2018/6/12   0.1.2      Add ESD protect under idle/nav mode
 *
 *****************************************************************************/

#define FILE_TAG "silead_nav"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_impl.h"
#include "silead_error.h"
#include "silead_finger_internal.h"
#include "silead_key.h"

static uint32_t m_nav_type = NAV_TYPE_NONE;

int32_t silfp_nav_init()
{
    m_nav_type = NAV_TYPE_NONE;
    sl_fp_nav_support(&m_nav_type);

    LOG_MSG_INFO("nav support:%d", m_nav_type);

    return 0;
}

int32_t silfp_nav_deinit()
{
    return 0;
}

int32_t silfp_nav_force_support(void)
{
    return !!(m_nav_type == NAV_TYPE_MAX);
}

int32_t silfp_nav_check_support(void)
{
    return ((m_nav_type > NAV_TYPE_NONE && m_nav_type <= NAV_TYPE_MAX)) ? 1 : 0;
}

static int32_t _finger_send_key_for_lite(uint32_t key, int32_t *status)
{
    int32_t ret = SL_SUCCESS;
    uint32_t up_key = 0;

    if (key == NAV_KEY_CLICK_DOWN) {
        sl_fp_send_key(key);
        do {
            ret = sl_fp_wait_finger_up();
            if (ret >= 0) {
                ret = sl_fp_nav_capture_image();
                if (ret >= 0) {
                    ret = sl_fp_nav_step(&up_key);
                    if (ret >= 0) {
                        if (up_key == NAV_KEY_CLICK_UP) {
                            sl_fp_send_key(up_key);
                            break;
                        }
                    }
                }
            }
        } while (!silfp_finger_is_canceled());
    }

    if (status != NULL) {
        *status = -1;
    }

    return ret;
}

int32_t silfp_nav_command()
{
    int32_t ret = SL_SUCCESS;
    uint32_t key = 0;
    int32_t status = -1;

    LOG_MSG_DEBUG("nav-------------");

    ret = sl_fp_nav_start();
    if (ret >= 0) {
        ret = sl_fp_wait_finger_nav();
    }

    if (ret >= 0) {
        ret = sl_fp_nav_capture_image();
        if (ret >= 0) {
            do {
                ret = sl_fp_nav_step(&key);
                if (ret >= 0) {
                    if (IS_KEY_VALID(key)) {
                        LOG_MSG_VERBOSE("*********report key = %s", silead_key_get_des((int32_t)key));
                        if (m_nav_type == NAV_TYPE_LITE) {
                            ret = _finger_send_key_for_lite(key, &status);
                        } else {
                            if (m_nav_type == NAV_TYPE_NORMAL) {
                                sl_fp_send_key(key);
                            }
                            status = 0;
                        }
                        break;
                    }
                } else { // some error or cancel
                    break;
                }
            } while (!silfp_finger_is_canceled());
        }
    }

    sl_fp_nav_end();

    if (ret != -SL_ERROR_CANCELED && !silfp_finger_is_canceled()) {
        silfp_finger_set_work_state_no_signal(STATE_IDLE);
    }

    LOG_MSG_DEBUG("nav finish------------- %d", ret);

    if (status >= 0) {
        sl_fp_wait_finger_up();
    }

    return ret;
}
