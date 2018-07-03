/******************************************************************************
 * @file   silead_qsee_keymaster.c
 * @brief  Contains QSEE keymaster functions.
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

#define FILE_TAG "QSEE_KEYMASTER"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "QSEEComFunc.h"
#include "silead_error.h"

#define KM_TZAPP_PATH "/firmware/image/"
#define KM_TZAPP_NAME "keymaster"
#define KM_TZAPP_NAME64 "keymaster64"

typedef struct _km_get_auth_token_req_t {
    uint32_t cmd_id;
    uint32_t auth_type;
} __attribute__ ((packed)) km_get_auth_token_req_t;

typedef struct _km_get_auth_token_rsp_t {
    int32_t status;
    uint32_t auth_token_key_offset;
    uint32_t auth_token_key_len;
} __attribute__ ((packed)) km_get_auth_token_rsp_t;

int32_t silfp_qsee_keymaster_get(qsee_handle_t *qsee_handle, void **buffer)
{
    int32_t ret = 0;
    struct QSEECom_handle * keymasterHandle = NULL;

    if (qsee_handle == NULL) {
        ret = -SL_ERROR_TA_OPEN_FAILED;
        return ret;
    }

    do {
        ret = qsee_handle->QSEECom_load_trustlet(qsee_handle, &keymasterHandle, KM_TZAPP_PATH, KM_TZAPP_NAME, 1024);
        if (ret < 0) {
            ret = qsee_handle->QSEECom_load_trustlet(qsee_handle, &keymasterHandle, KM_TZAPP_PATH, KM_TZAPP_NAME64, 1024);
            if (ret < 0) {
                LOG_MSG_ERROR("load keymaster fail: %d", ret);
                ret = -SL_ERROR_TA_OPEN_FAILED;
                break;
            }
        }

        // Start creating one of command to get cert from keymaster
        km_get_auth_token_req_t *req = (km_get_auth_token_req_t *) keymasterHandle->ion_sbuffer;
        req->cmd_id = 0x205;
        req->auth_type = 0x02;

        uint8_t * rec_buf = keymasterHandle->ion_sbuffer + QSEECOM_ALIGN(sizeof(*req));

        //Send command to keymaster
        ret = qsee_handle->QSEECom_send_cmd(keymasterHandle, req, QSEECOM_ALIGN(sizeof(*req)), rec_buf, 1024 - QSEECOM_ALIGN(sizeof(*req)));
        if (ret < 0) {
            ret = -SL_ERROR_TA_SEND_FAILED;
            break;
        }

        km_get_auth_token_rsp_t* rsp_data = (km_get_auth_token_rsp_t*) rec_buf;

        LOG_MSG_VERBOSE("rsp status : %d", rsp_data->status);
        LOG_MSG_VERBOSE("rsp Length : %u", rsp_data->auth_token_key_len);
        LOG_MSG_VERBOSE("rsp Offset: %u", rsp_data->auth_token_key_offset);

        if (rsp_data->status == 0) {
            if (buffer != NULL) {
                *buffer = malloc(rsp_data->auth_token_key_len);
                if (*buffer != NULL) {
                    void *data_buff = &rec_buf[rsp_data->auth_token_key_offset];
                    memcpy(*buffer, data_buff, rsp_data->auth_token_key_len);
                    ret = rsp_data->auth_token_key_len;
                    break;
                }
            }
        }
        ret = -SL_ERROR_TA_SEND_FAILED;
    } while (0);

    qsee_handle->QSEECom_shutdown_app(&keymasterHandle);

    return ret;
}
