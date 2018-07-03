/******************************************************************************
 * @file   silead_nosec_impl.c
 * @brief  Contains Non-secure communication implements.
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

#define FILE_TAG "FP_IMP"
#include "log/logmsg.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "NOSECComFunc.h"
#include "silead_nosec_common.h"
#include "silead_error.h"
#include "silead_nosec_impl.h"

extern int32_t sl_fp_wakelock(uint8_t lock);
static pthread_mutex_t m_tz_lock;
#define mutex_lock()    pthread_mutex_lock(&m_tz_lock); sl_fp_wakelock(1)
#define mutex_unlock()  sl_fp_wakelock(0); pthread_mutex_unlock(&m_tz_lock)
#define mutex_init()    pthread_mutex_init(&m_tz_lock, NULL)
#define mutex_destroy() pthread_mutex_destroy(&m_tz_lock);

static nosec_handle_t *m_tz_handle = NULL;

// 0: send buffer 1: get buffer 2: send & get buffer 3:always get even error
static int32_t _ca_send_modified_command(uint32_t cmd, void *buffer, uint32_t len, uint32_t isget, uint32_t v1, uint32_t v2,
        uint32_t *data1, uint32_t *data2)
{
    int32_t ret = 0;
    int32_t always_get = 0;
    struct send_cmd send_cmd;
    struct send_cmd_rsp rcv_buf;

    if (m_tz_handle == NULL) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    if (isget != 0 && isget != 2) {
        memset(buffer, 0, len);
    }

    mutex_lock();

    memset(&send_cmd, 0, sizeof(send_cmd));
    memset(&send_cmd, 0, sizeof(rcv_buf));

    send_cmd.cmd_id = cmd;
    send_cmd.v_addr = buffer;
    send_cmd.length = len;
    send_cmd.data1 = v1;
    send_cmd.data2 = v2;

    LOG_MSG_VERBOSE("> 0x%x", cmd);
    ret = m_tz_handle->tz_app_cmd_handler(&send_cmd, sizeof(send_cmd), &rcv_buf, sizeof(rcv_buf));

    LOG_MSG_VERBOSE("< 0x%x (%d)", cmd, ret);
    if(ret < 0) {
        LOG_MSG_ERROR("send cmd(0x%02X) error (%d:%d:%s)", cmd, ret, errno, strerror(errno));
        ret = -SL_ERROR_TA_SEND_FAILED;
    } else {
        ret = rcv_buf.status;
        if (isget == 3) {
            always_get = 1;
        }
    }

    if (ret >= 0 || always_get) {
        if (data1 != NULL) {
            *data1 = rcv_buf.data1;
        }
        if (data2 != NULL) {
            *data2 = rcv_buf.data2;
        }
    }

    mutex_unlock();

    return ret;
}

static int32_t _ca_send_normal_command(uint32_t cmd, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4,
                                       uint32_t *data1, uint32_t *data2, uint32_t *data3)
{
    int32_t ret = 0;
    struct send_cmd send_cmd;
    struct send_cmd_rsp rcv_buf;

    if (m_tz_handle == NULL) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    mutex_lock();

    memset(&send_cmd, 0, sizeof(send_cmd));
    memset(&send_cmd, 0, sizeof(rcv_buf));

    send_cmd.cmd_id = cmd;
    send_cmd.length = 0;
    send_cmd.data1 = v1;
    send_cmd.data2 = v2;
    send_cmd.data3 = v3;
    send_cmd.data4 = v4;

    LOG_MSG_VERBOSE("> 0x%x", cmd);
    ret = m_tz_handle->tz_app_cmd_handler(&send_cmd, sizeof(send_cmd), &rcv_buf, sizeof(rcv_buf));

    LOG_MSG_VERBOSE("< 0x%x (%d)", cmd, ret);
    if(ret < 0) {
        LOG_MSG_ERROR("send cmd(0x%02X) error (%d:%d:%s)", cmd, ret, errno, strerror(errno));
        ret = -SL_ERROR_TA_SEND_FAILED;
    } else {
        ret = rcv_buf.status;
    }

    //if (ret >= 0) {
        if (data1 != NULL) {
            *data1 = rcv_buf.data1;
        }
        if (data2 != NULL) {
            *data2 = rcv_buf.data2;
        }
        if (data3 != NULL) {
            *data3 = rcv_buf.data3;
        }
    //}

    mutex_unlock();

    return ret;
}

static int32_t _ca_open(void)
{
    int32_t ret = 0;

    mutex_init();

    if (nosec_open_handle(&m_tz_handle) < 0) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    m_tz_handle->tz_app_init();

    return ret;
}

static int32_t _ca_close(void)
{
    LOG_MSG_DEBUG("nosec close");

    if (m_tz_handle != NULL) {
        m_tz_handle->tz_app_shutdown();

        nosec_free_handle(&m_tz_handle);
        m_tz_handle = NULL;
    }

    mutex_destroy();
    return 0;
}

int32_t silfp_ca_nosec_register(ca_impl_handle_t* handle)
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
