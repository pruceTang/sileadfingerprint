/******************************************************************************
 * @file   silead_qsee_cb.c
 * @brief  Contains QSEE communication callback functions.
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

#define FILE_TAG "QSEE_CB"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>

#include "silead_error.h"
#include "silead_qsee_cb.h"
#include "silead_qsee_spi_cb.h"
#include "tz_cmd.h"
#include "../QSEEComFunc.h"
#include "../silead_qsee_common.h"
#include "silead_qsee_extend.h"

#define ION_BUFF_LEN  64*1024
#ifndef SPI_DEV
#define SPI_DEV "/dev/spidev0.0"
#endif

static qsee_handle_t *m_qsee_handle = NULL;
static struct QSEECom_handle *m_cb_handle;
static struct qcom_km_ion_info m_ion_handle;
static int32_t m_spi_fd = -1;
static sem_t m_sem;
static pthread_t m_thread_id;
static int32_t m_exit;

static spi_cb_handle_t *m_spi_cb_handler = NULL;

static void *_thr_cb_listener(void *data);

int32_t silfp_cb_qsee_init(qsee_handle_t *handle, uint32_t svrid)
{
    int32_t ret = 0;
    int32_t len = ION_BUFF_LEN;

    if (handle == NULL) {
        LOG_MSG_ERROR("Invalid qsee handle");
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    ret = silfp_spi_cb_open_handle(&m_spi_cb_handler);
    if (ret < 0) {
        LOG_MSG_ERROR("can't get spi cb handle (%d)", ret);
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    m_qsee_handle = handle;
    ret = sem_init(&m_sem, 0, 0);
    if (ret < 0) {
        LOG_MSG_ERROR("sem_init fail (%d:%s)", errno, strerror(errno));
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    m_ion_handle.ion_fd = 0;
    if (m_qsee_handle->QCom_ion_alloc(&m_ion_handle, len) < 0) {
        LOG_MSG_ERROR("ION allocation failed, try again");
        if (m_qsee_handle->QCom_ion_alloc(&m_ion_handle, len) < 0) {
            LOG_MSG_ERROR("ION allocation failed");
            return -SL_ERROR_SHARED_ALLOC_FAILED;
        }
    }
    memset((void *)m_ion_handle.ion_sbuffer, 0, len);

    ret = pthread_create(&m_thread_id, NULL, &_thr_cb_listener, (void*)(unsigned long)svrid);
    if (ret < 0) {
        LOG_MSG_ERROR("thread_create fail (%d:%s)", errno, strerror(errno));
        sem_destroy(&m_sem);
        return -SL_ERROR_TA_OPEN_FAILED;
    }
    sem_wait(&m_sem);

    ret = silfp_qsee_send_modified_command_to_tz_with_ion(TZ_FP_CMD_INIT_UNK_1, m_ion_handle, len);

    return ret;
}

void silfp_cb_qsee_deinit()
{
    m_exit = 1;
    silfp_qsee_send_normal_command(TZ_FP_CMD_DEINIT_UNK_1, 0, NULL, NULL);
    if (pthread_join(m_thread_id, NULL) != 0 ) {
        LOG_MSG_VERBOSE("pthread_join failed");
    }

    sem_destroy(&m_sem);
    if (m_qsee_handle != NULL) {
        m_qsee_handle->QCom_ion_free(&m_ion_handle);
    }

    if (m_spi_cb_handler != NULL) {
        silfp_spi_cb_free_handle(&m_spi_cb_handler);
        m_spi_cb_handler = NULL;
    }
}

static int32_t _qsee_cb_read_reg(uint32_t reg, uint32_t *value)
{
    int32_t ret = -1;
    if (m_spi_fd >= 0) {
        if (m_spi_cb_handler != NULL) {
            ret = m_spi_cb_handler->fp_spi_read_reg(m_spi_fd, reg, value);
        }
    }

    return ret;
}

static int32_t _qsee_cb_write_reg(uint32_t reg, uint32_t value)
{
    int32_t ret = -1;
    if (m_spi_fd > 0) {
        if (m_spi_cb_handler != NULL) {
            ret = m_spi_cb_handler->fp_spi_write_reg(m_spi_fd, reg, value);
        }
    }

    return ret;
}

static int32_t _qsee_cb_download_cfg(uint32_t count)
{
    int32_t ret = -1;
    if (m_spi_fd > 0) {
        if (m_spi_cb_handler != NULL) {
            ret = m_spi_cb_handler->fp_spi_write_cfg2(m_spi_fd, (const void *)m_ion_handle.ion_sbuffer, count);
        }
    }

    return ret;
}

static int32_t _qsee_cb_get_frame(uint32_t reg, uint32_t size, uint16_t ms_frm, uint16_t spi_retry)
{
    int32_t ret = -1;
    if (m_spi_fd > 0) {
        if (m_spi_cb_handler != NULL) {
            ret = m_spi_cb_handler->fp_spi_get_frame2(m_spi_fd, reg, (char *)m_ion_handle.ion_sbuffer, size, ms_frm, spi_retry);
        }
    }

    return ret;
}

static int32_t _qsee_cb_open(uint32_t data1, uint32_t data2, uint32_t data3, uint32_t data4)
{
    int32_t ret = 0;

    if (m_spi_fd < 0) {
        if (m_spi_cb_handler != NULL) {
            m_spi_fd = m_spi_cb_handler->fp_spi_open(SPI_DEV);
        }
        if (m_spi_fd < 0) {
            ret = -1;
            LOG_MSG_ERROR("open SPI %s fail", SPI_DEV);
        } else {
            if (m_spi_cb_handler != NULL) {
                ret = m_spi_cb_handler->fp_spi_init(m_spi_fd, data1, data2, data3, data4);
            }
        }
    }

    return ret;
}

static int32_t _qsee_cb_close()
{
    m_exit = 1;
    if (m_spi_fd >= 0) {
        if (m_spi_cb_handler != NULL) {
            m_spi_cb_handler->fp_spi_deinit(m_spi_fd);
            m_spi_cb_handler->fp_spi_close(m_spi_fd);
        }
        m_spi_fd = -1;
    }
    return 0;
}

static void *_thr_cb_listener(void *data)
{
    struct qsc_send_cmd *req_res;
    struct qsc_send_cmd_rsp rsp;

    struct timeval tv;
    uint64_t times;
    int32_t ret;
    uint32_t svrid = (uint32_t)(unsigned long)data;

    do {
        /* Register as a listener with the QSApp */
        ret = m_qsee_handle->QSEECom_register_listener(&m_cb_handle, svrid, sizeof(*req_res), 0);
        LOG_MSG_VERBOSE("Registering as listener (%d), ret = %d", svrid, ret);

        if (ret < 0) {
            sem_post(&m_sem);
            break;
        }

        /* Before we check the return, unblock the parent in case we failed and are exiting */
        /* Overlay the request/response structure on the ion allocated memory */
        req_res = (struct qsc_send_cmd *)(m_cb_handle->ion_sbuffer);

        sem_post(&m_sem);

        while(!m_exit) {
            /* Wait for request from the QSApp */
            ret = m_qsee_handle->QSEECom_receive_req(m_cb_handle, req_res, sizeof(*req_res));
            if(ret) {
                LOG_MSG_DEBUG("get service fail, ret %d",ret);
                break;
            }

            LOG_MSG_DEBUG("get service req %X", req_res->cmd_id);

            /* Attempt to read the parents request and response buffers, MUST be all zeros.
             * This ensures that XPU protection is in place during the send_cmd() call.
             */

            rsp.status = -1;
            switch(req_res->cmd_id) {
            case TZ_FP_CB_SPI_OPEN: {
                rsp.status = _qsee_cb_open(req_res->data1, req_res->data2, req_res->data3, req_res->data4);
                break;
            }
            case TZ_FP_CB_EXIT: {
                rsp.status = _qsee_cb_close();
                break;
            }
            case TZ_FP_CB_SPI_RD: {
                uint32_t value = 0;
                rsp.status = _qsee_cb_read_reg(req_res->data1, &value);
                rsp.data1 = value;
                break;
            }
            case TZ_FP_CB_SPI_WR: {
                rsp.status = _qsee_cb_write_reg(req_res->data1, req_res->data2);
                break;
            }
            case TZ_FP_CB_SPI_DOWN_CFG: {
                rsp.status = _qsee_cb_download_cfg(req_res->length);
                break;
            }
            case TZ_FP_CB_SPI_GET_FRAME: {
                rsp.status = _qsee_cb_get_frame(req_res->data1, req_res->data2, req_res->data3, req_res->data4);
                break;
            }
            default:
                LOG_MSG_DEBUG("Unsupport callback id %d", req_res->cmd_id);
                break;
            }

            /* Send the response to the QSApp */
            LOG_MSG_VERBOSE("callback rsp %d", rsp.status);
            ret = m_qsee_handle->QSEECom_send_resp(m_cb_handle, &rsp, sizeof(rsp));
        }
    } while(0);

    m_qsee_handle->QSEECom_unregister_listener(m_cb_handle);
    return NULL;
}
