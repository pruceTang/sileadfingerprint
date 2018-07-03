/******************************************************************************
 * @file   silead_fp_storage.c
 * @brief  Contains fingerprint storage functions.
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
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * Luke Ma     2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifdef SIL_SEC_STORE
#define __TEE_FP_BASE_H__
#include "gpPersistObjFileIO.h"
#endif /* SIL_SEC_STORE */

#define FILE_TAG "store_qsee"
#include "silead_logmsg.h"

#include "silead_base.h"
#include "silead_error.h"
#include "silead_fp_storage_impl.h"

static uint32_t silfp_storage_normal_style()
{
#ifdef SIL_SEC_STORE
    return 0;
#else
    return 1;
#endif /* SIL_SEC_STORE */
}

#ifdef SIL_SEC_STORE
static int32_t silfp_storage_read_file(const uint8_t *fname, uint8_t *buf, uint32_t size)
{
    TEE_Result ret;
    uint32_t count;

    ret = gpFileIO_Read_File(NULL, (const char*)fname, buf, size, 0, &count);
    if (ret != TEE_SUCCESS) {
        return -SL_ERROR_STO_OP_FAILED;
    }

    LOG_MSG_VERBOSE("read_file %s, %d", fname, count);
    return (int32_t)count;
}

static int32_t silfp_storage_write_file(const uint8_t *fname, const uint8_t *buf, uint32_t len)
{
    TEE_Result ret;
    uint32_t count;

    ret = gpFileIO_Write_File(NULL, (const char*)fname, (void *)buf, len, 0, &count);
    if (TEE_SUCCESS != ret) {
        return -SL_ERROR_STO_OP_FAILED;
    }

    if (count != len) {
        gpFileIO_Remove_File(NULL, (const char*)fname);
        return -SL_ERROR_STO_OP_FAILED;
    }

    LOG_MSG_VERBOSE("write_file %s, %d", fname, len);
    return len;
}

static int32_t silfp_storage_remove_file(const uint8_t *fname)
{
    TEE_Result ret = TEE_SUCCESS;

    ret = gpFileIO_Remove_File(NULL, (const char*)fname);
    if (TEE_SUCCESS != ret) {
        LOG_MSG_DEBUG("remove_file failed (%d)", ret);
    }

    LOG_MSG_VERBOSE("remove_file %s", fname);
    return 0;
}
#endif /* SIL_SEC_STORE */

int32_t silfp_storage_getif(store_if_t *store)
{
    if (store == NULL) {
        return -1;
    }

    memset(store, 0, sizeof(*store));
    store->normal_style = silfp_storage_normal_style;
#ifdef SIL_SEC_STORE
    store->read_file    = silfp_storage_read_file;
    store->write_file   = silfp_storage_write_file;
    store->remove_file  = silfp_storage_remove_file;
#endif /* SIL_SEC_STORE */

    return 0;
}

