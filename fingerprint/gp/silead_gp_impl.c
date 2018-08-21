/******************************************************************************
 * @file   silead_gp_impl.c
 * @brief  Contains GP communication implements.
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
 * Daniel Ye   2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "GP_IMP"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "GPComFunc.h"
#include "silead_error.h"
#include "silead_gp_impl.h"

#define FP_TZAPP_PATH "/system/vendor/app/mcRegistry"
#ifndef FP_GP_TZAPP_NAME
#define FP_GP_TZAPP_NAME (TEEC_UUID){0x0511eadf, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a}}
#endif

extern int32_t sl_fp_wakelock(uint8_t lock);
static TEEC_Session *m_fp_handle = NULL;
static gp_handle_t *m_tz_handle = NULL;
static pthread_mutex_t tz_lock;

#define TZ_OK TEEC_SUCCESS

#define LOCK() pthread_mutex_lock(&tz_lock); sl_fp_wakelock(1)
#define UNLOCK() sl_fp_wakelock(0); pthread_mutex_unlock(&tz_lock)

// 0: send buffer 1: get buffer 2: send & get buffer 3:always get even error
static int32_t _ca_send_modified_command(uint32_t cmd, void *buffer, uint32_t len, uint32_t isget, uint32_t v1, uint32_t v2,
        uint32_t *data1, uint32_t *data2)
{
    int32_t ret = 0;
    int32_t always_get = 0;
    TEEC_Operation sOperation;

    if (m_tz_handle == NULL || m_fp_handle == NULL) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    if (buffer == NULL || len <= 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    LOCK();

    memset(&sOperation, 0, sizeof(TEEC_Operation));
    if (isget != 0) {
        sOperation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_VALUE_OUTPUT, TEEC_MEMREF_TEMP_INOUT, TEEC_NONE);
    } else {
        sOperation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_VALUE_OUTPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE);
    }

    sOperation.params[0].value.a = v1;
    sOperation.params[0].value.b = v2;
    sOperation.params[2].tmpref.buffer = buffer;
    sOperation.params[2].tmpref.size = len;

    if (isget != 0 && isget != 2) {
        memset((void *)buffer, 0, len);
    }

    LOG_MSG_VERBOSE("> 0x%x", cmd);
    ret = m_tz_handle->TEEC_InvokeCommand(m_fp_handle, cmd, &sOperation, NULL);
    LOG_MSG_VERBOSE("< 0x%x (%d)", cmd, ret);
    if (TZ_OK != ret) {
        LOG_MSG_ERROR("send cmd(0x%02X) error (%d:%d:%s)", cmd, ret, errno, strerror(errno));
        ret = -SL_ERROR_TA_SEND_FAILED;
    } else {
        ret = sOperation.params[0].value.a;
        if (isget == 3) {
            always_get = 1;
        }
    }

    if (ret >= 0 || always_get) {
        if (data1 != NULL) {
            *data1 = sOperation.params[0].value.b;
        }
        if (data2 != NULL) {
            *data2 = sOperation.params[1].value.a;
        }
    }

    UNLOCK();

    return ret;
}

static int32_t _ca_send_normal_command(uint32_t cmd, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4,
                                       uint32_t *data1, uint32_t *data2, uint32_t *data3)
{
    int32_t ret = 0;
    TEEC_Operation sOperation;

    if (m_tz_handle == NULL || m_fp_handle == NULL) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    LOCK();

    memset(&sOperation, 0, sizeof(TEEC_Operation));
    sOperation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_VALUE_INOUT, TEEC_NONE, TEEC_NONE);

    sOperation.params[0].value.a = v1;
    sOperation.params[0].value.b = v2;
    sOperation.params[1].value.a = v3;
    sOperation.params[1].value.b = v4;

    LOG_MSG_VERBOSE("> 0x%x", cmd);
    ret = m_tz_handle->TEEC_InvokeCommand(m_fp_handle, cmd, &sOperation, NULL);
    LOG_MSG_VERBOSE("< 0x%x (%d)", cmd, ret);
    if (TZ_OK != ret) {
        LOG_MSG_ERROR("send cmd(0x%02X) error (%d:%d:%s)", cmd, ret, errno, strerror(errno));
        ret = -SL_ERROR_TA_SEND_FAILED;
    } else {
        ret = sOperation.params[0].value.a;
    }

    //if (ret >= 0) {
        if (data1 != NULL) {
            *data1 = sOperation.params[0].value.b;
        }
        if (data2 != NULL) {
            *data2 = sOperation.params[1].value.a;
        }
        if (data3 != NULL) {
            *data3 = sOperation.params[1].value.b;
        }
    //}
    UNLOCK();

    return ret;
}

static int32_t _ca_open(void)
{
    int32_t ret = 0;

    pthread_mutex_init(&tz_lock, NULL);

    if (gp_open_handle(&m_tz_handle) < 0) {
        LOG_MSG_ERROR("gp_open_handle fail");
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    if (m_tz_handle->TEECCom_load_trustlet(m_tz_handle, &m_fp_handle, FP_TZAPP_PATH, FP_GP_TZAPP_NAME) < 0) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    return ret;
}

static int32_t _ca_close(void)
{
    LOG_MSG_DEBUG("tee close");

    if (m_tz_handle != NULL) {
        if (m_fp_handle != NULL) {
            m_tz_handle->TEECCom_close_trustlet(m_tz_handle);
            m_fp_handle = NULL;
        }
        gp_free_handle(&m_tz_handle);
        m_tz_handle = NULL;
    }

    pthread_mutex_destroy(&tz_lock);
    return 0;
}

int32_t silfp_ca_gp_register(ca_impl_handle_t* handle, const void *ta_name)
{
    int32_t ret;

    if (handle == NULL) {
        LOG_MSG_VERBOSE("handle buffer is invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _ca_open();
    if (ret < 0) {
        _ca_close();
    }

    memset(handle, 0, sizeof(ca_impl_handle_t));
    handle->ca_send_modified_command = _ca_send_modified_command;
    handle->ca_send_normal_command = _ca_send_normal_command;
    handle->ca_close = _ca_close;

    return ret;
}
