/******************************************************************************
 * @file   silead_tee_impl.c
 * @brief  Contains TEE communication implements.
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

#define FILE_TAG "TEE_IMP"
#include "log/logmsg.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "TEEComFunc.h"
#include "silead_error.h"
#include "silead_tee_impl.h"

#define FP_TZAPP_PATH "/system/vendor/app/mcRegistry"
#ifndef FP_TEE_TZAPP_NAME
#define FP_TEE_TZAPP_NAME "0511eadf00000000000000000000001a"
#endif

extern int32_t sl_fp_wakelock(uint8_t lock);
static mcSessionHandle_t *m_fp_handle = NULL;
static tee_handle_t *m_tz_handle = NULL;
static pthread_mutex_t tz_lock;

#define TZ_OK MC_DRV_OK

#define LOCK() pthread_mutex_lock(&tz_lock); sl_fp_wakelock(1)
#define UNLOCK() sl_fp_wakelock(0); pthread_mutex_unlock(&tz_lock)

// 0: send buffer 1: get buffer 2: send & get buffer 3:always get even error
static int32_t _ca_send_modified_command(uint32_t cmd, void *buffer, uint32_t len, uint32_t isget, uint32_t v1, uint32_t v2,
        uint32_t *data1, uint32_t *data2)
{
    int32_t ret = 0;
    int32_t always_get = 0;
    mcBulkMap_t mapInfo;
    tciSilMessage_t* send_cmd_ptr = NULL;
    tciSilMessage_t* rcv_buf_ptr = NULL;
    sil_cmd_t* send_cmd = NULL;
    sil_rsp_t* rcv_buf = NULL;

    if (m_tz_handle == NULL || m_fp_handle == NULL) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    if (buffer == NULL || len <= 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    memset(&mapInfo, 0, sizeof(mapInfo));
    ret = m_tz_handle->mcMap(m_fp_handle, buffer, len, &mapInfo);
    if (TZ_OK != ret) {
        LOG_MSG_ERROR("mcMap fail (%d), len = %d", ret, len);
        return -SL_ERROR_SHARED_ALLOC_FAILED;
    }

    LOCK();
    do {
        send_cmd_ptr = (tciSilMessage_t*) m_tz_handle->TEECom_get_tci();
        rcv_buf_ptr = (tciSilMessage_t*) m_tz_handle->TEECom_get_tci();
        if (send_cmd_ptr == NULL || rcv_buf_ptr == NULL) {
            ret = -SL_ERROR_OUT_OF_MEMORY;
            break;
        }
        send_cmd = &send_cmd_ptr->cmd;
        rcv_buf = &rcv_buf_ptr->rsp;
        send_cmd->header.commandId = cmd;

        send_cmd->len = len;
        send_cmd->data = v1;
        send_cmd->data2 = v2;
        send_cmd->v_addr = (intptr_t)(long)mapInfo.sVirtualAddr;

        if (isget != 0 && isget != 2) {
            memset((void *)buffer, 0, len);
        }

        LOG_MSG_VERBOSE("> 0x%x", cmd);
        ret = m_tz_handle->TEEComSend(m_tz_handle, cmd, rcv_buf_ptr);
        LOG_MSG_VERBOSE("< 0x%x (%d)", cmd, ret);
        if (TZ_OK != ret) {
            LOG_MSG_ERROR("send cmd(0x%02X) error (%d:%d:%s)", cmd, ret, errno, strerror(errno));
            ret = -SL_ERROR_TA_SEND_FAILED;
        } else {
            ret = rcv_buf->status;
            if (isget == 3) {
                always_get = 1;
            }
        }

        if (ret >= 0 || always_get) {
            if (data1 != NULL) {
                *data1 = rcv_buf->data;
            }
            if (data2 != NULL) {
                *data2 = rcv_buf->data2;
            }
        }
    } while (0);

    UNLOCK();

    if (TZ_OK != m_tz_handle->mcUnmap(m_fp_handle, buffer, &mapInfo)) {
        LOG_MSG_ERROR("Error during memory unmapping");
    }

    return ret;
}

static int32_t _ca_send_normal_command(uint32_t cmd, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4,
                                       uint32_t *data1, uint32_t *data2, uint32_t *data3)
{
    int32_t ret = 0;
    tciSilMessage_t* send_cmd_ptr = NULL;
    tciSilMessage_t* rcv_buf_ptr = NULL;
    sil_cmd_t* send_cmd = NULL;
    sil_rsp_t* rcv_buf = NULL;

    if (m_tz_handle == NULL || m_fp_handle == NULL) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    LOCK();
    do {
        send_cmd_ptr = (tciSilMessage_t*) m_tz_handle->TEECom_get_tci();
        rcv_buf_ptr = (tciSilMessage_t*) m_tz_handle->TEECom_get_tci();
        if (send_cmd_ptr == NULL || rcv_buf_ptr == NULL) {
            ret = -SL_ERROR_OUT_OF_MEMORY;
            break;
        }
        send_cmd = &send_cmd_ptr->cmd;
        rcv_buf = &rcv_buf_ptr->rsp;
        send_cmd->header.commandId = cmd;

        send_cmd->len = 0;
        send_cmd->data  = v1;
        send_cmd->data2 = v2;
        send_cmd->data3 = v3;
        send_cmd->data4 = v4;

        LOG_MSG_VERBOSE("> 0x%x", cmd);
        ret = m_tz_handle->TEEComSend(m_tz_handle, cmd, rcv_buf_ptr);
        LOG_MSG_VERBOSE("< 0x%x (%d)", cmd, ret);
        if (TZ_OK != ret) {
            LOG_MSG_ERROR("send cmd(0x%02X) error (%d:%d:%s)", cmd, ret, errno, strerror(errno));
            ret = -SL_ERROR_TA_SEND_FAILED;
        } else {
            ret = rcv_buf->status;
        }

        //if (ret >= 0) {
            if (data1 != NULL) {
                *data1 = rcv_buf->data;
            }
            if (data2 != NULL) {
                *data2 = rcv_buf->data2;
            }
            if (data3 != NULL) {
                *data3 = rcv_buf->data3;
            }
        //}
    } while (0);

    UNLOCK();

    return ret;
}

static int32_t _ca_open(void)
{
    int32_t ret = 0;

    pthread_mutex_init(&tz_lock, NULL);

    if (tee_open_handle(&m_tz_handle) < 0) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    if (m_tz_handle->TEECom_load_trustlet(m_tz_handle, &m_fp_handle, FP_TZAPP_PATH, FP_TEE_TZAPP_NAME) < 0) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    return ret;
}

static int32_t _ca_close(void)
{
    LOG_MSG_DEBUG("tee close");

    if (m_tz_handle != NULL) {
        if (m_fp_handle != NULL) {
            m_tz_handle->TEECom_close_trustlet(m_tz_handle);
            m_fp_handle = NULL;
        }
        tee_free_handle(&m_tz_handle);
        m_tz_handle = NULL;
    }

    pthread_mutex_destroy(&tz_lock);
    return 0;
}

int32_t silfp_ca_tee_register(ca_impl_handle_t* handle, const void *ta_name)
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
