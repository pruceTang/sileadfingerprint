/******************************************************************************
 * @file   silead_qsee_extend.c
 * @brief  Contains QSEE communication extension functions.
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
 *****************************************************************************/

#define FILE_TAG "QSEE_EXT"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>

#include "silead_error.h"
#include "tz_cmd.h"
#include "../QSEEComFunc.h"
#include "silead_qsee_extend.h"

#define QSEE_FEATURE_CB_MASK 0x01
#define QSEE_FEATURE_CUSTOM_MALLOC_MASK 0x02

static uint32_t m_qsee_feature_value = 0;

int32_t silfp_ext_qsee_init(qsee_handle_t *handle)
{
    int32_t ret = 0;
    uint32_t cb_svr_id = 0x100;

    if (silfp_qsee_send_normal_command(TZ_FP_CMD_INIT_UNK_0, 0, &m_qsee_feature_value, &cb_svr_id) >= 0) {
        if (m_qsee_feature_value & QSEE_FEATURE_CB_MASK) {
            ret = silfp_cb_qsee_init(handle, cb_svr_id);
            if (ret < 0) {
                return -SL_ERROR_TA_OPEN_FAILED;
            }
        }

        if (m_qsee_feature_value & QSEE_FEATURE_CUSTOM_MALLOC_MASK) {
            ret = silfp_qsee_custom_mem_init(handle);
            if (ret < 0) {
                return -SL_ERROR_TA_OPEN_FAILED;
            }
        }
    } else {
        m_qsee_feature_value = 0;
    }

    return ret;
}

void silfp_ext_qsee_deinit()
{
    if (m_qsee_feature_value & QSEE_FEATURE_CB_MASK) {
        silfp_cb_qsee_deinit();
    }

    if (m_qsee_feature_value & QSEE_FEATURE_CUSTOM_MALLOC_MASK) {
        silfp_qsee_custom_mem_deinit();
    }
}
