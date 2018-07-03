/******************************************************************************
 * @file   NOSECComFunc.c
 * @brief  Contains Non-secure communication functions.
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

#define FILE_TAG "NOSEC_WRAPPER"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "nosec/NOSECComFunc.h"
#include "silead_error.h"

#define NOSEC_LIBRARY "libsl_fp_nosec.so"

typedef struct {
    void *libHandle;
} _priv_data_t;

#define DECLARE_LIB_FUNC(x, sx) \
    do { \
        handle->x = dlsym(data->libHandle, sx); \
        if(handle->x == NULL) { \
            LOG_MSG_DEBUG("Loading %s failed (%d:%s)", sx, errno, strerror(errno)); \
            goto exit; \
        } \
    } while(0)

int32_t nosec_open_handle(nosec_handle_t** ret_handle)
{
    nosec_handle_t *handle = NULL;
    _priv_data_t *data = NULL;

    LOG_MSG_VERBOSE("Using Target Lib : %s", NOSEC_LIBRARY);
    data = (_priv_data_t*)malloc(sizeof(_priv_data_t));
    if(data == NULL) {
        LOG_MSG_DEBUG("Allocating memory failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    data->libHandle = dlopen(NOSEC_LIBRARY, RTLD_NOW);
    if(data->libHandle == NULL) {
        LOG_MSG_DEBUG("load NOSEC API library failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }
    LOG_MSG_VERBOSE("NOSEC API library successed (%p)", data->libHandle);

    handle = (nosec_handle_t*)malloc(sizeof(nosec_handle_t));
    if(handle == NULL) {
        LOG_MSG_DEBUG("Allocating memory failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->_data = data;

    DECLARE_LIB_FUNC(tz_app_init, "tz_app_init");
    DECLARE_LIB_FUNC(tz_app_cmd_handler, "tz_app_cmd_handler");
    DECLARE_LIB_FUNC(tz_app_shutdown, "tz_app_shutdown");

    if (ret_handle) {
        *ret_handle = handle;
        return 0;
    }

exit:
    if(handle != NULL) {
        free(handle);
        handle = NULL;
    }
    if(data->libHandle != NULL) {
        dlclose(data->libHandle);
        data->libHandle = NULL;
    }
    if(data != NULL) {
        free(data);
        data = NULL;
    }
    return -1;
}

int32_t nosec_free_handle(nosec_handle_t** handle_ptr)
{
    _priv_data_t *data = NULL;
    nosec_handle_t *handle = NULL;

    if ((handle_ptr != NULL) && (*handle_ptr != NULL)) {
        handle = *handle_ptr;
        data = (_priv_data_t*)handle->_data;

        if(data->libHandle != NULL) {
            dlclose(data->libHandle);
            data->libHandle = NULL;
        }
        if(data != NULL) {
            free(data);
            data = NULL;
        }
        if(handle != NULL) {
            free(handle);
            handle = NULL;
        }
        *handle_ptr = NULL;
    }

    return 0;
}
