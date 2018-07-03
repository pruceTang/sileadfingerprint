/******************************************************************************
 * @file   silead_ca_entry.c
 * @brief  Contains CA entry functions.
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
 *
 *****************************************************************************/

#define FILE_TAG "CA_IMP"
#include "log/logmsg.h"

#include <string.h>

#include "silead_error.h"
#include "silead_ca.h"
#include "silead_finger_internal.h"

#ifdef SECURITY_TYPE_QSEE
#include "qsee/silead_qsee_impl.h"
#endif
#ifdef SECURITY_TYPE_TEE
#include "tee/silead_tee_impl.h"
#endif
#ifdef SECURITY_TYPE_GP
#include "gp/silead_gp_impl.h"
#endif
#ifdef SECURITY_TYPE_NOSEC
#include "nosec/silead_nosec_impl.h"
#endif

ca_impl_handle_t m_ca_handler;
uint32_t m_ta_send_error_times = 0;

static void _ca_check_send_result(int32_t ret)
{
    if (ret == -SL_ERROR_TA_OPEN_FAILED || ret == -SL_ERROR_TA_SEND_FAILED) {
        m_ta_send_error_times++;
    } else {
        m_ta_send_error_times = 0;
    }

    if (m_ta_send_error_times >= 10) {
        silfp_finger_set_work_state(STATE_BREAK);
    }
}

int32_t silfp_ca_send_modified_command(uint32_t cmd, void *buffer, uint32_t len, uint32_t isget, uint32_t v1, uint32_t v2, uint32_t *data1, uint32_t *data2)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (m_ca_handler.ca_send_modified_command != NULL) {
        ret = m_ca_handler.ca_send_modified_command(cmd, buffer, len, isget, v1, v2, data1, data2);
    }

    _ca_check_send_result(ret);

    return ret;
}

int32_t silfp_ca_send_normal_command(uint32_t cmd, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4, uint32_t *data1, uint32_t *data2, uint32_t *data3)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (m_ca_handler.ca_send_normal_command != NULL) {
        ret = m_ca_handler.ca_send_normal_command(cmd, v1, v2, v3, v4, data1, data2, data3);
    }

    _ca_check_send_result(ret);

    return ret;
}

int32_t silfp_ca_open(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    memset(&m_ca_handler, 0, sizeof(m_ca_handler));

#ifdef SECURITY_TYPE_QSEE
    if (ret < 0) {
        ret = silfp_ca_qsee_register(&m_ca_handler);
    }
#endif
#ifdef SECURITY_TYPE_GP
    if (ret < 0) {
        ret = silfp_ca_gp_register(&m_ca_handler);
    }
#endif
#ifdef SECURITY_TYPE_TEE
    if (ret < 0) {
        ret = silfp_ca_tee_register(&m_ca_handler);
    }
#endif
#ifdef SECURITY_TYPE_NOSEC
    if (ret < 0) {
        ret = silfp_ca_nosec_register(&m_ca_handler);
    }
#endif

    return ret;
}

int32_t silfp_ca_close(void)
{
    if (m_ca_handler.ca_close == NULL) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    return m_ca_handler.ca_close();
}

int32_t silfp_ca_keymaster_get(void **buffer)
{
    if (m_ca_handler.ca_keymaster_get == NULL) {
        return 0;
    }

    return m_ca_handler.ca_keymaster_get(buffer);
}
