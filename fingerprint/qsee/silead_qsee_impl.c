/******************************************************************************
 * @file   silead_qsee_impl.c
 * @brief  Contains QSEE communication implements.
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
 * Willian Kin 2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "QSEE_IMP"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "QSEEComFunc.h"
#include "silead_qsee_common.h"
#include "silead_qsee_keymaster.h"
#include "silead_error.h"
#include "silead_qsee_impl.h"

#define FP_QSEE_TZAPP_PATH "/vendor/firmware/"
#define FP_QSEE_TZAPP_PATH2 "/firmware/image/"
#define FP_QSEE_TZAPP_PATH3 "/etc/firmware/"
#ifndef FP_QSEE_TZAPP_NAME
#define FP_QSEE_TZAPP_NAME "sileadta"
#endif
#ifndef FP_QSEE_TZAPP64_NAME
#define FP_QSEE_TZAPP64_NAME (FP_QSEE_TZAPP_NAME "64")
#endif

static struct QSEECom_handle *m_fp_handle = NULL;
static qsee_handle_t *m_tz_handle = NULL;
extern int32_t sl_fp_wakelock(uint8_t lock);

static pthread_mutex_t m_tz_lock;
#define mutex_lock()    pthread_mutex_lock(&m_tz_lock); sl_fp_wakelock(1)
#define mutex_unlock()  sl_fp_wakelock(0); pthread_mutex_unlock(&m_tz_lock)
#define mutex_init()    pthread_mutex_init(&m_tz_lock, NULL)
#define mutex_destroy() pthread_mutex_destroy(&m_tz_lock);

static struct qseecom_app_info m_tz_app_info;
#define is_app_64bit() (m_tz_app_info.is_secure_app_64bit)

int32_t __attribute__((weak)) silfp_ext_qsee_init(qsee_handle_t *handle);
void __attribute__((weak)) silfp_ext_qsee_deinit();

static int32_t _ca_send_modified_command(uint32_t cmd, void *buffer, uint32_t len, uint32_t isget, uint32_t v1, uint32_t v2,
        uint32_t *data1, uint32_t *data2)
{
    int32_t ret = 0;
    int32_t always_get = 0;
    struct qsc_send_cmd* send_cmd = NULL;
    struct qsc_send_cmd_rsp* rcv_buf = NULL;

    struct QSEECom_ion_fd_info ion_fd_info;
    struct qcom_km_ion_info ihandle;

    if (m_tz_handle == NULL || m_fp_handle == NULL) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    if (buffer == NULL || len <= 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    ihandle.ion_fd = 0;
    if (m_tz_handle->QCom_ion_alloc(&ihandle, len) < 0) {
        LOG_MSG_ERROR("ION allocation failed");
        return -SL_ERROR_SHARED_ALLOC_FAILED;
    }

    memset(&ion_fd_info, 0, sizeof(struct QSEECom_ion_fd_info));
    ion_fd_info.data[0].fd = ihandle.ifd_data_fd;
    ion_fd_info.data[0].cmd_buf_offset = sizeof(uint32_t);

    if (isget != 0 && isget != 2 && isget != 4) {
        memset((void *)ihandle.ion_sbuffer, 0, len);
    } else {
        memcpy((void *)ihandle.ion_sbuffer, buffer, len);
    }

    mutex_lock();

    send_cmd = (struct qsc_send_cmd*) m_fp_handle->ion_sbuffer;
    rcv_buf = (struct qsc_send_cmd_rsp*) m_fp_handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(*send_cmd));

    send_cmd->cmd_id = cmd;
    send_cmd->length = len;
    send_cmd->data1 = v1;
    send_cmd->data2 = v2;

    LOG_MSG_VERBOSE("> 0x%x%s", send_cmd->cmd_id, is_app_64bit()?"(64bit)":"");
    ret = m_tz_handle->QSEECom_send_modified_cmd(m_fp_handle, send_cmd, QSEECOM_ALIGN(sizeof(*send_cmd)),
            rcv_buf, QSEECOM_ALIGN(sizeof(*rcv_buf)), &ion_fd_info);

    LOG_MSG_VERBOSE("< 0x%x (%d)", cmd, ret);
    if (ret < 0) {
        LOG_MSG_ERROR("send cmd(0x%02X) error (%d:%d:%s)", cmd, ret, errno, strerror(errno));
        ret = -SL_ERROR_TA_SEND_FAILED;
    } else {
        if (send_cmd->v_addr != 0) {
            LOG_MSG_ERROR("Error on TZ");
            ret = -SL_ERROR_TA_OP_FAILED;
        } else {
            if (isget == 3 || isget == 4) {
                always_get = 1;
            }
            if (always_get || (isget != 0 && rcv_buf->status >= 0)) {
                memcpy(buffer, (void *)ihandle.ion_sbuffer, len);
            }
            ret = rcv_buf->status;
        }
    }

    if (ret >= 0 || always_get) {
        if (data1 != NULL) {
            *data1 = rcv_buf->data1;
        }
        if (data2 != NULL) {
            *data2 = rcv_buf->data2;
        }
    }

    mutex_unlock();

    m_tz_handle->QCom_ion_free(&ihandle);

    return ret;
}

static int32_t _ca_send_normal_command(uint32_t cmd, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4,
                                       uint32_t *data1, uint32_t *data2, uint32_t *data3)
{
    int32_t ret = 0;
    struct qsc_send_cmd* send_cmd = NULL;
    struct qsc_send_cmd_rsp* rcv_buf = NULL;

    if (m_tz_handle == NULL || m_fp_handle == NULL) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    mutex_lock();

    send_cmd = (struct qsc_send_cmd*) m_fp_handle->ion_sbuffer;
    rcv_buf = (struct qsc_send_cmd_rsp*) m_fp_handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(*send_cmd));

    send_cmd->cmd_id = cmd;
    send_cmd->length = 0;
    send_cmd->data1 = v1;
    send_cmd->data2 = v2;
    send_cmd->data3 = v3;
    send_cmd->data4 = v4;

    LOG_MSG_VERBOSE("> 0x%x", send_cmd->cmd_id);
    ret = m_tz_handle->QSEECom_send_cmd(m_fp_handle, send_cmd, QSEECOM_ALIGN(sizeof(*send_cmd)),
                                        rcv_buf, QSEECOM_ALIGN(sizeof(*rcv_buf)));

    LOG_MSG_VERBOSE("< 0x%x (%d)", cmd, ret);
    if(ret < 0) {
        LOG_MSG_ERROR("send cmd(0x%02X) error (%d:%d:%s)", cmd, ret, errno, strerror(errno));
        ret = -SL_ERROR_TA_SEND_FAILED;
    } else {
        ret = rcv_buf->status;
    }

    if (data1 != NULL) {
        *data1 = rcv_buf->data1;
    }
    if (data2 != NULL) {
        *data2 = rcv_buf->data2;
    }
    if (data3 != NULL) {
        *data3 = rcv_buf->data3;
    }

    mutex_unlock();

    return ret;
}

static int32_t _ca_open(const void *ta_name)
{
    int32_t ret = 0;
    void *buffer = NULL;
    int32_t len;
    uint32_t cb_svr_id = 0x100;

    mutex_init();

    ret = qsee_open_handle(&m_tz_handle);
    if (ret < 0) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    memset(&m_tz_app_info, 0, sizeof(m_tz_app_info));

    if (ta_name && (strlen(ta_name) > 0)) {
        ret = m_tz_handle->QSEECom_load_trustlet(m_tz_handle, &m_fp_handle, FP_QSEE_TZAPP_PATH, ta_name, 1024);
        if (ret < 0) {
            ret = m_tz_handle->QSEECom_load_trustlet(m_tz_handle, &m_fp_handle, FP_QSEE_TZAPP_PATH2, ta_name, 1024);
            if (ret < 0) {
                ret = m_tz_handle->QSEECom_load_trustlet(m_tz_handle, &m_fp_handle, FP_QSEE_TZAPP_PATH3, ta_name, 1024);
            }
        }
    } else {
        ret = -1;
    }

    if (ret < 0) {
        ret = m_tz_handle->QSEECom_load_trustlet(m_tz_handle, &m_fp_handle, FP_QSEE_TZAPP_PATH, FP_QSEE_TZAPP_NAME, 1024);
        if (ret < 0) {
            ret = m_tz_handle->QSEECom_load_trustlet(m_tz_handle, &m_fp_handle, FP_QSEE_TZAPP_PATH2, FP_QSEE_TZAPP_NAME, 1024);
            if (ret < 0) {
                ret = m_tz_handle->QSEECom_load_trustlet(m_tz_handle, &m_fp_handle, FP_QSEE_TZAPP_PATH3, FP_QSEE_TZAPP_NAME, 1024);
                if (ret < 0) {
#ifdef FP_QSEE_TZAPP64_NAME
                    ret = m_tz_handle->QSEECom_load_trustlet(m_tz_handle, &m_fp_handle, FP_QSEE_TZAPP_PATH, FP_QSEE_TZAPP64_NAME, 1024);
                    if (ret < 0) {
                        ret = m_tz_handle->QSEECom_load_trustlet(m_tz_handle, &m_fp_handle, FP_QSEE_TZAPP_PATH2, FP_QSEE_TZAPP64_NAME, 1024);
                        if (ret < 0) {
                            ret = m_tz_handle->QSEECom_load_trustlet(m_tz_handle, &m_fp_handle, FP_QSEE_TZAPP_PATH3, FP_QSEE_TZAPP64_NAME, 1024);
                            if (ret < 0) {
                                return -SL_ERROR_TA_OPEN_FAILED;
                            }
                        }
                    }
#else
                    return -SL_ERROR_TA_OPEN_FAILED;
#endif /* FP_QSEE_TZAPP64_NAME */
                }
            }
        }
    }

    if (m_tz_handle->QSEECom_get_app_info != NULL) {
        ret = m_tz_handle->QSEECom_get_app_info(m_fp_handle, &m_tz_app_info);
        if (ret) {
            return -SL_ERROR_TA_OPEN_FAILED;
        }
    }

    if (is_app_64bit() && m_tz_handle->QSEECom_send_modified_cmd_64 != NULL) {
        m_tz_handle->QSEECom_send_modified_cmd = m_tz_handle->QSEECom_send_modified_cmd_64;
    } else {
        m_tz_handle->QSEECom_send_modified_cmd = m_tz_handle->QSEECom_send_modified_cmd_32;
    }

    if (silfp_ext_qsee_init) {
        ret = silfp_ext_qsee_init(m_tz_handle);
    }

    return ret;
}

static int32_t _ca_close(void)
{
    LOG_MSG_DEBUG("qsee close");

    if (silfp_ext_qsee_deinit) {
        silfp_ext_qsee_deinit();
    }

    if (m_tz_handle != NULL) {
        if (m_fp_handle != NULL) {
            m_tz_handle->QSEECom_shutdown_app(&m_fp_handle);
            m_fp_handle = NULL;
        }
        qsee_free_handle(&m_tz_handle);
        m_tz_handle = NULL;
    }

    mutex_destroy();

    return 0;
}

static int32_t _ca_keymaster_get(void **buffer)
{
    int32_t ret;
    ret = silfp_qsee_keymaster_get(m_tz_handle, buffer);
    if (ret < 0) {
        ret = -SL_ERROR_TA_OPEN_FAILED;
    }
    return ret;
}

int32_t silfp_ca_qsee_register(ca_impl_handle_t* handle, const void *ta_name)
{
    int32_t ret;

    if (handle == NULL) {
        LOG_MSG_VERBOSE("handle buffer is invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _ca_open(ta_name);
    if (ret < 0) {
        _ca_close();
    }

    memset(handle, 0, sizeof(ca_impl_handle_t));
    handle->ca_send_modified_command = _ca_send_modified_command;
    handle->ca_send_normal_command = _ca_send_normal_command;
    handle->ca_close = _ca_close;
    handle->ca_keymaster_get = _ca_keymaster_get;

    return ret;
}

int32_t silfp_qsee_send_modified_command_to_tz_with_ion(uint32_t cmd, struct qcom_km_ion_info ihandle, uint32_t len)
{
    int32_t ret = 0;
    struct qsc_send_cmd* send_cmd = NULL;
    struct qsc_send_cmd_rsp* rcv_buf = NULL;

    struct QSEECom_ion_fd_info ion_fd_info;

    if (m_tz_handle == NULL || m_fp_handle == NULL) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    memset(&ion_fd_info, 0, sizeof(struct QSEECom_ion_fd_info));
    ion_fd_info.data[0].fd = ihandle.ifd_data_fd;
    ion_fd_info.data[0].cmd_buf_offset = sizeof(uint32_t);

    mutex_lock();

    send_cmd = (struct qsc_send_cmd*) m_fp_handle->ion_sbuffer;
    rcv_buf = (struct qsc_send_cmd_rsp*) m_fp_handle->ion_sbuffer + QSEECOM_ALIGN(sizeof(*send_cmd));

    send_cmd->cmd_id = cmd;
    send_cmd->length = len;

    LOG_MSG_VERBOSE("> 0x%x%s", send_cmd->cmd_id, is_app_64bit()?"(64bit)":"");
    ret = m_tz_handle->QSEECom_send_modified_cmd(m_fp_handle, send_cmd, QSEECOM_ALIGN(sizeof(*send_cmd)),
            rcv_buf, QSEECOM_ALIGN(sizeof(*rcv_buf)), &ion_fd_info);

    LOG_MSG_VERBOSE("< 0x%x (%d)", cmd, ret);
    if (ret < 0) {
        LOG_MSG_ERROR("send cmd(0x%02X) error (%d:%d:%s)", cmd, ret, errno, strerror(errno));
        ret = -SL_ERROR_TA_SEND_FAILED;
    } else {
        if (send_cmd->v_addr != 0) {
            LOG_MSG_ERROR("Error on TZ");
            ret = -SL_ERROR_TA_OP_FAILED;
        } else {
            ret = rcv_buf->status;
        }
    }

    mutex_unlock();

    return ret;
}

int32_t silfp_qsee_send_normal_command(uint32_t cmd, uint32_t v, uint32_t *r1, uint32_t *r2)
{
    return _ca_send_normal_command(cmd, v, 0, 0, 0, r1, r2, NULL);
}
