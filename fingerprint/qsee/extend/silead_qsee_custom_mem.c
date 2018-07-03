/******************************************************************************
 * @file   silead_qsee_custom_mem.c
 * @brief  Contains QSEE customized memory functions.
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

#define FILE_TAG "QSEE_MEM"
#include "log/logmsg.h"

#include <string.h>

#include "silead_error.h"
#include "tz_cmd.h"
#include "../QSEEComFunc.h"
#include "silead_qsee_extend.h"

#ifndef CUSTOM_MEM_ION_BUFF_LEN
#define CUSTOM_MEM_ION_BUFF_LEN  (2*1024*1024)
#endif

static qsee_handle_t *m_qsee_handle = NULL;
static struct qcom_km_ion_info m_ion_handle;

int32_t silfp_qsee_custom_mem_init(qsee_handle_t *handle)
{
    int32_t ret = 0;
    int32_t len = CUSTOM_MEM_ION_BUFF_LEN;

    if (handle == NULL) {
        LOG_MSG_ERROR("Invalid qsee handle");
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    m_qsee_handle = handle;

    m_ion_handle.ion_fd = 0;
    if (m_qsee_handle->QCom_ion_alloc(&m_ion_handle, len) < 0) {
        LOG_MSG_ERROR("ION allocation failed, try again");
        if (m_qsee_handle->QCom_ion_alloc(&m_ion_handle, len) < 0) {
            LOG_MSG_ERROR("ION allocation failed");
            return -SL_ERROR_SHARED_ALLOC_FAILED;
        }
    }

    memset((void *)m_ion_handle.ion_sbuffer, 0, len);

    ret = silfp_qsee_send_modified_command_to_tz_with_ion(TZ_FP_CMD_INIT_UNK_2, m_ion_handle, len);

    return ret;
}

void silfp_qsee_custom_mem_deinit()
{
    silfp_qsee_send_normal_command(TZ_FP_CMD_DEINIT_UNK_2, 0, NULL, NULL);

    if (m_qsee_handle != NULL) {
        m_qsee_handle->QCom_ion_free(&m_ion_handle);
    }
}