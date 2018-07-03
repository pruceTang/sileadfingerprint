/******************************************************************************
 * @file   TEEComFunc.c
 * @brief  Contains TEE communication functions header file.
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

#ifndef __TEECOMFUNC_H__
#define __TEECOMFUNC_H__

#include <dlfcn.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "public/MobiCoreDriverApi.h"

#include "tlfp_Api.h"

#define TAEXT ".tlbin"
#define DREXT ".drbin"

#define UUID_LEN  16

typedef struct tee_handle {
    void *_data;

    mcResult_t (*mcOpenDevice)(uint32_t deviceId);
    mcResult_t (*mcCloseDevice)(uint32_t deviceId);
    mcResult_t (*mcOpenSession)( mcSessionHandle_t *session, const mcUuid_t *uuid, uint8_t *tci, uint32_t tciLen);
    mcResult_t (*mcOpenTrustlet)(mcSessionHandle_t *session, mcSpid_t spid, uint8_t *trustedapp, uint32_t tLen, uint8_t *tci,uint32_t tciLen);
    mcResult_t (*mcCloseSession)(mcSessionHandle_t *session);
    mcResult_t (*mcNotify)(mcSessionHandle_t *session);
    mcResult_t (*mcWaitNotification)(mcSessionHandle_t *session, int32_t timeout);
    mcResult_t (*mcMallocWsm)(uint32_t deviceId, uint32_t align, uint32_t len, uint8_t **wsm, uint32_t wsmFlags);
    mcResult_t (*mcFreeWsm)(uint32_t deviceId, uint8_t *wsm);
    mcResult_t (*mcMap)(mcSessionHandle_t *session, void *buf, uint32_t len, mcBulkMap_t *mapInfo);
    mcResult_t (*mcUnmap)(mcSessionHandle_t *session, void *buf, mcBulkMap_t *mapInfo);
    mcResult_t (*mcGetSessionErrorCode)(mcSessionHandle_t *session, int32_t *lastErr);
    mcResult_t (*mcGetMobiCoreVersion)(uint32_t  deviceId, mcVersionInfo_t *versionInfo);

    int32_t (*TEECom_load_trustlet)(struct tee_handle* tee_handle, mcSessionHandle_t **clnt_handle,
                                    const char *path, const char *taname);
    void (*TEECom_close_trustlet)(struct tee_handle* tee_handle);
    tciSilMessage_t *(*TEECom_get_tci)(void);
    int32_t (*TEEComSend)(struct tee_handle* tee_handle, uint32_t cmd, tciSilMessage_t* rcv);
} tee_handle_t;

int tee_open_handle(tee_handle_t **handle);
int tee_free_handle(tee_handle_t** handle);
char* tee_error_strings(int err);

#endif /* __TEECOMFUNC_H__ */
