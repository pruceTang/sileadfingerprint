/******************************************************************************
 * @file   TEEComFunc.c
 * @brief  Contains TEE communication functions.
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

#define FILE_TAG "TEE_WRAPPER"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "tee/TEEComFunc.h"

#define TEE_LIBRARY "libMcClient.so"

static const uint32_t DEVICE_ID = MC_DEVICE_ID_DEFAULT;
static tciSilMessage_t  *m_tci;
static mcSessionHandle_t m_sessionHandle;

typedef struct {
    void *libHandle;
} _priv_data_t;

static tciSilMessage_t *tee_get_tci(void);
static int32_t tee_send(tee_handle_t* tee_handle, uint32_t cmd, tciSilMessage_t* rcv);
static mcResult_t _tlcOpen(tee_handle_t* tee_handle,const char* path,const char *taname);
static void _tlcClose(tee_handle_t* tee_handle);
static int tee_load_trustlet(tee_handle_t* tee_handle, mcSessionHandle_t **clnt_handle, const char *path, const char *taname);

#define DECLARE_LIB_FUNC(x, sx) \
    do { \
        handle->x = dlsym(data->libHandle, sx); \
        if(handle->x == NULL) { \
            LOG_MSG_DEBUG("Loading %s failed (%d:%s)", sx, errno, strerror(errno)); \
            goto exit; \
        } \
    } while(0)

int32_t tee_open_handle(tee_handle_t** ret_handle)
{
    tee_handle_t *handle = NULL;
    _priv_data_t *data = NULL;

    LOG_MSG_VERBOSE("Using Target Lib : %s", TEE_LIBRARY);
    data = (_priv_data_t*)malloc(sizeof(_priv_data_t));
    if(data == NULL) {
        LOG_MSG_DEBUG("Allocating memory failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    data->libHandle = dlopen(TEE_LIBRARY, RTLD_NOW);
    if(data->libHandle == NULL) {
        LOG_MSG_DEBUG("load TEECom API library failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }
    LOG_MSG_VERBOSE("TEECom API library successed (%p)", data->libHandle);

    handle = (tee_handle_t*)malloc(sizeof(tee_handle_t));
    if(handle == NULL) {
        LOG_MSG_DEBUG("Allocating memory failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->_data = data;

    // Setup internal functions
    handle->TEECom_load_trustlet = tee_load_trustlet;
    handle->TEECom_close_trustlet = _tlcClose;
    handle->TEECom_get_tci = tee_get_tci;
    handle->TEEComSend = tee_send;

    // Setup TEECom Functions
    DECLARE_LIB_FUNC(mcOpenDevice, "mcOpenDevice");
    DECLARE_LIB_FUNC(mcCloseDevice, "mcCloseDevice");
    DECLARE_LIB_FUNC(mcOpenSession, "mcOpenSession");
    DECLARE_LIB_FUNC(mcOpenTrustlet, "mcOpenTrustlet");
    DECLARE_LIB_FUNC(mcCloseSession, "mcCloseSession");
    DECLARE_LIB_FUNC(mcNotify, "mcNotify");
    DECLARE_LIB_FUNC(mcWaitNotification, "mcWaitNotification");
    DECLARE_LIB_FUNC(mcMallocWsm, "mcMallocWsm");
    DECLARE_LIB_FUNC(mcFreeWsm, "mcFreeWsm");
    DECLARE_LIB_FUNC(mcMap, "mcMap");
    DECLARE_LIB_FUNC(mcUnmap, "mcUnmap");
    DECLARE_LIB_FUNC(mcGetSessionErrorCode, "mcGetSessionErrorCode");
    DECLARE_LIB_FUNC(mcGetMobiCoreVersion, "mcGetMobiCoreVersion");

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

int tee_free_handle(tee_handle_t** handle_ptr)
{
    _priv_data_t *data = NULL;
    tee_handle_t *handle = NULL;

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

char* tee_error_strings(int err)
{
    switch (err) {
    case MC_DRV_NO_NOTIFICATION:
        return "TEECom: No notification available.";
    case MC_DRV_ERR_NOTIFICATION:
        return "TEECom: Error during notification on communication level.";
    case MC_DRV_ERR_NOT_IMPLEMENTED:
        return "TEECom: Function not implemented.";
    case MC_DRV_ERR_OUT_OF_RESOURCES:
        return "TEECom: No more resources available.";
    case MC_DRV_ERR_INIT:
        return "TEECom: Driver initialization failed.";
    case MC_DRV_ERR_UNKNOWN:
        return "TEECom: Unknown error.";
    case MC_DRV_ERR_UNKNOWN_DEVICE:
        return "TEECom: The specified device is unknown.";
    case MC_DRV_ERR_UNKNOWN_SESSION:
        return "TEECom: The specified session is unknown.";
    case MC_DRV_ERR_INVALID_OPERATION:
        return "TEECom: The specified operation is not allowed.";
    case MC_DRV_ERR_INVALID_RESPONSE:
        return "TEECom: The response header from the MC is invalid.";
    case MC_DRV_ERR_TIMEOUT:
        return "TEECom: Function call timed out.";
    case MC_DRV_ERR_NO_FREE_MEMORY:
        return "TEECom: Can not allocate additional memory.";
    case MC_DRV_ERR_FREE_MEMORY_FAILED:
        return "TEECom: Free memory failed.";
    case MC_DRV_ERR_SESSION_PENDING:
        return "TEECom: Still some open sessions pending.";
    case MC_DRV_ERR_DAEMON_UNREACHABLE:
        return "TEECom: MC daemon not reachable.";
    case MC_DRV_ERR_INVALID_DEVICE_FILE:
        return "TEECom: The device file of the kernel module could not be opened.";
    case MC_DRV_ERR_INVALID_PARAMETER:
        return "TEECom: Invalid parameter.";
    case MC_DRV_ERR_KERNEL_MODULE:
        return "TEECom: Error from Kernel Module, see DETAIL for errno.";
    case MC_DRV_ERR_BULK_MAPPING:
        return "TEECom: Error during mapping of additional bulk memory to session.";
    case MC_DRV_ERR_BULK_UNMAPPING:
        return "TEECom: Error during unmapping of additional bulk memory to session.";
    case MC_DRV_INFO_NOTIFICATION:
        return "TEECom: Notification received, exit code available.";
    case MC_DRV_ERR_NQ_FAILED:
        return "TEECom: Set up of NWd connection failed.";
    case MC_DRV_ERR_DAEMON_VERSION:
        return "TEECom: Wrong daemon version.";
    case MC_DRV_ERR_CONTAINER_VERSION:
        return "TEECom: Wrong container version.";
    case MC_DRV_ERR_WRONG_PUBLIC_KEY:
        return "TEECom: System Trustlet public key is wrong.";
    case MC_DRV_ERR_CONTAINER_TYPE_MISMATCH:
        return "TEECom: Wrong container type(s).";
    case MC_DRV_ERR_CONTAINER_LOCKED:
        return "TEECom: Container is locked (or not activated).";
    case MC_DRV_ERR_SP_NO_CHILD:
        return "TEECom: SPID is not registered with root container.";
    case MC_DRV_ERR_TL_NO_CHILD:
        return "TEECom: UUID is not registered with sp container.";
    case MC_DRV_ERR_UNWRAP_ROOT_FAILED:
        return "TEECom: Unwrapping of root container failed.";
    case MC_DRV_ERR_UNWRAP_SP_FAILED:
        return "TEECom: Unwrapping of service provider container failed.";
    case MC_DRV_ERR_UNWRAP_TRUSTLET_FAILED:
        return "TEECom: Unwrapping of Trustlet container failed.";
    default:
        return "TEECom: Unknown error";
    }
}

static int tee_load_trustlet(tee_handle_t* tee_handle, mcSessionHandle_t **clnt_handle,
                             const char *path, const char *taname)
{
    int ret = 0;

    LOG_MSG_VERBOSE("Starting app %s", taname);
    ret = _tlcOpen(tee_handle, path, taname);
    if (ret < 0) {
        LOG_MSG_DEBUG("Could not load app %s (%d:%d:%s)", taname, ret, errno, strerror(errno));
    } else if (clnt_handle) {
        LOG_MSG_DEBUG("TZ App loaded : %s", taname);
        *clnt_handle = &m_sessionHandle;
    }

    return ret;
}

static tciSilMessage_t *tee_get_tci(void)
{
    return m_tci;
}

static size_t getFileContent(const char* path, const char *fname, const char *ext, uint8_t** ppContent)
{
    FILE* pStream = NULL;
    long filesize;
    uint8_t* content = NULL;
    char fullname[255];

    if (!path || !fname) {
        return 0;
    }

    strcpy(fullname, path);
    if (fullname[strlen(path)-1] != '/') {
        strcat(fullname,"/");
    }
    strcat(fullname, fname);
    if (ext != NULL) {
        strcat(fullname, ext);
    }

    /* Open the file */
    pStream = fopen(fullname, "rb");
    if (pStream == NULL) {
        LOG_MSG_DEBUG("Error: Cannot open file: %s.", fullname);
        return 0;
    }

    if (fseek(pStream, 0L, SEEK_END) != 0) {
        LOG_MSG_DEBUG("Error: Cannot read file: %s.", fullname);
        goto error;
    }

    filesize = ftell(pStream);
    if (filesize < 0) {
        LOG_MSG_DEBUG("Error: Cannot get the file size: %s.", fullname);
        goto error;
    }

    if (filesize == 0) {
        LOG_MSG_DEBUG("Error: Empty file: %s.", fullname);
        goto error;
    }

    /* Set the file pointer at the beginning of the file */
    if (fseek(pStream, 0L, SEEK_SET) != 0) {
        LOG_MSG_DEBUG("Error: Cannot read file: %s.", fullname);
        goto error;
    }

    /* Allocate a buffer for the content */
    content = (uint8_t*)malloc(filesize);
    if (content == NULL) {
        LOG_MSG_DEBUG("Error: Cannot read file: Out of memory.");
        goto error;
    }

    /* Read data from the file into the buffer */
    if (fread(content, (size_t)filesize, 1, pStream) != 1) {
        LOG_MSG_DEBUG("Error: Cannot read file: %s.", fullname);
        goto error;
    }

    /* Close the file */
    fclose(pStream);
    if (ppContent != NULL) {
        *ppContent = content;
    } else {
        free(content);
    }

    /* Return number of bytes read */
    return (size_t)filesize;

error:
    if (content  != NULL) {
        free(content);
    }
    fclose(pStream);
    return 0;
}

static mcResult_t _tlcOpen(tee_handle_t* tee_handle, const char* path, const char *taname)
{
    mcResult_t mcRet;
    mcVersionInfo_t versionInfo;
    uint8_t* pTrustletData = NULL;
    uint32_t nTrustletSize;
    
    if (tee_handle == NULL || taname == NULL) {
        return -1;
    }

    do {
        LOG_MSG_VERBOSE("Opening <t-base device");
        mcRet = tee_handle->mcOpenDevice(DEVICE_ID);
        if (MC_DRV_OK != mcRet) {
            LOG_MSG_DEBUG("Error opening device: %d", mcRet);
            break;
        }

        mcRet = tee_handle->mcGetMobiCoreVersion(MC_DEVICE_ID_DEFAULT, &versionInfo);
        if (MC_DRV_OK != mcRet) {
            LOG_MSG_DEBUG("mcGetMobiCoreVersion failed %d", mcRet);
            break;
        }

        LOG_MSG_VERBOSE("productId        = %s", versionInfo.productId);
        LOG_MSG_VERBOSE("versionMci       = 0x%08X", versionInfo.versionMci);
        LOG_MSG_VERBOSE("versionSo        = 0x%08X", versionInfo.versionSo);
        LOG_MSG_VERBOSE("versionMclf      = 0x%08X", versionInfo.versionMclf);
        LOG_MSG_VERBOSE("versionContainer = 0x%08X", versionInfo.versionContainer);
        LOG_MSG_VERBOSE("versionMcConfig  = 0x%08X", versionInfo.versionMcConfig);
        LOG_MSG_VERBOSE("versionTlApi     = 0x%08X", versionInfo.versionTlApi);
        LOG_MSG_VERBOSE("versionDrApi     = 0x%08X", versionInfo.versionDrApi);
        LOG_MSG_VERBOSE("versionCmp       = 0x%08X", versionInfo.versionCmp);

        m_tci = (tciSilMessage_t*)malloc(sizeof(tciSilMessage_t));

        if (m_tci == NULL) {
            LOG_MSG_DEBUG("Allocation of TCI failed");
            mcRet = -1;
            break;
        }
        memset(m_tci, 0, sizeof(tciSilMessage_t));

        nTrustletSize = getFileContent(path, taname, TAEXT, &pTrustletData);
        if (nTrustletSize == 0) {
            LOG_MSG_DEBUG("Trustlet not found");
            mcRet = -1;
            break;
        }

        LOG_MSG_VERBOSE("Opening the session");
        memset(&m_sessionHandle, 0, sizeof(m_sessionHandle));
        m_sessionHandle.deviceId = DEVICE_ID; // The device ID (default device is used)
        mcRet = tee_handle->mcOpenTrustlet(
                    &m_sessionHandle,
                    MC_SPID_RESERVED_TEST, /* mcSpid_t */
                    pTrustletData,
                    nTrustletSize,
                    (uint8_t *) m_tci,
                    sizeof(tciSilMessage_t));

        if (MC_DRV_OK != mcRet) {
            LOG_MSG_DEBUG("Open session failed: %d", mcRet);
            break;
        } else {
            LOG_MSG_VERBOSE("open() succeeded");
        }

        free(pTrustletData);
        pTrustletData = NULL;
    } while (0);

    if (pTrustletData != NULL) {
        free(pTrustletData);
        pTrustletData = NULL;
    }

    if (mcRet != MC_DRV_OK) {
        LOG_MSG_DEBUG("Open session failed: %d", mcRet);

        tee_handle->mcCloseSession(&m_sessionHandle);
        tee_handle->mcCloseDevice(DEVICE_ID);
        if (m_tci != NULL) {
            free(m_tci);
            m_tci = NULL;
        }

        return -1;
    } else {
        LOG_MSG_VERBOSE("open() succeeded");
    }

    return 0;
}

static void _tlcClose(tee_handle_t* tee_handle)
{
    mcResult_t ret;

    if (tee_handle != NULL) {
        LOG_MSG_VERBOSE("Closing the session");
        ret = tee_handle->mcCloseSession(&m_sessionHandle);
        if (MC_DRV_OK != ret) {
            LOG_MSG_ERROR("Closing session failed: %d", ret);
        }

        LOG_MSG_VERBOSE("Closing <t-base device");
        ret = tee_handle->mcCloseDevice(DEVICE_ID);
        if (MC_DRV_OK != ret) {
            LOG_MSG_ERROR("Closing <t-base device failed: %d", ret);
        }
    }

    if (m_tci != NULL) {
        free(m_tci);
        m_tci = NULL;
    }
}

static int32_t tee_send(tee_handle_t* tee_handle, uint32_t cmd, tciSilMessage_t* rcv)
{
    int32_t ret;

    if (tee_handle == NULL || rcv == NULL) {
        return -1;
    }

    ret = tee_handle->mcNotify(&m_sessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_MSG_ERROR("send cmd(0x%02X) error (%d:%d:%s)", cmd, ret, errno, strerror(errno));
        return -1;
    }

    LOG_MSG_DEBUG("Waiting for the Trustlet response");
    ret = tee_handle->mcWaitNotification(&m_sessionHandle, -1);
    if (MC_DRV_OK != ret) {
        LOG_MSG_ERROR("Wait for response notification failed: 0x%x", ret);
        return -1;
    }

    LOG_MSG_DEBUG("Verifying that the Trustlet sent a response.");
    if (RSP_ID(cmd) != rcv->rsp.header.responseId) {
        LOG_MSG_ERROR("Trustlet did not send a response: %d", rcv->rsp.header.responseId);
        return -1;
    }

    /*if (RET_OK != rcv->rsp.header.returnCode) {
        // Ignore the return code check, bcos some command, the return code has different means .
        LOG_MSG_VERBOSE("Trustlet did not send a valid return code: %d", rcv->rsp.header.returnCode);
        ret = rcv->rsp.header.returnCode;
        return -1;
    }*/
    return MC_DRV_OK;
}

/* End of file TEEComFunc.c */
