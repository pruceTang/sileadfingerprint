/******************************************************************************
 * @file   QSEEComFunc.c
 * @brief  Contains QSEE communication functions.
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

#define FILE_TAG "QSEE_WRAPPER"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "qsee/QSEEComFunc.h"

#define QSEE_LIBRARY "libQSEEComAPI.so"

typedef struct {
    void *libHandle;
} _priv_data_t;

static int32_t qcom_km_ion_dealloc(qcom_km_ion_info_t *handle);
static int32_t qcom_km_ion_memalloc(qcom_km_ion_info_t *handle, uint32_t size);
static int qsee_load_trustlet(qsee_handle_t* qsee_handle, struct QSEECom_handle **clnt_handle,
                              const char *path, const char *fname,uint32_t sb_size);

int32_t qsee_open_handle(qsee_handle_t** ret_handle)
{
    qsee_handle_t *handle = NULL;
    _priv_data_t *data = NULL;

    LOG_MSG_VERBOSE("Using Target Lib : %s", QSEE_LIBRARY);
    data = (_priv_data_t*)malloc(sizeof(_priv_data_t));
    if(data == NULL) {
        LOG_MSG_DEBUG("Allocating memory failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    data->libHandle = dlopen(QSEE_LIBRARY, RTLD_NOW);
    if(data->libHandle == NULL) {
        LOG_MSG_DEBUG("load QSEECom API library failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }
    LOG_MSG_VERBOSE("QSEECom API library successed (%p)", data->libHandle);

    handle = (qsee_handle_t*)malloc(sizeof(qsee_handle_t));
    if(handle == NULL) {
        LOG_MSG_DEBUG("Allocating memory failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->_data = data;

    // Setup internal functions
    handle->QCom_ion_alloc = qcom_km_ion_memalloc;
    handle->QCom_ion_free = qcom_km_ion_dealloc;
    handle->QSEECom_load_trustlet = qsee_load_trustlet;

    // Setup QSEECom Functions
    handle->QSEECom_start_app = dlsym(data->libHandle, "QSEECom_start_app");
    if(handle->QSEECom_start_app == NULL) {
        LOG_MSG_DEBUG("Loading QSEECom_start_app failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->QSEECom_shutdown_app = dlsym(data->libHandle, "QSEECom_shutdown_app");
    if(handle->QSEECom_shutdown_app == NULL) {
        LOG_MSG_DEBUG("Loading QSEECom_shutdown_app failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->QSEECom_load_external_elf  = dlsym(data->libHandle, "QSEECom_load_external_elf");
    if(handle->QSEECom_load_external_elf == NULL) {
        LOG_MSG_DEBUG("Loading QSEECom_load_external_elf failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->QSEECom_unload_external_elf = dlsym(data->libHandle, "QSEECom_unload_external_elf");
    if(handle->QSEECom_unload_external_elf == NULL) {
        LOG_MSG_DEBUG("Loading QSEECom_unload_external_elf failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->QSEECom_register_listener = dlsym(data->libHandle, "QSEECom_register_listener");
    if(handle->QSEECom_register_listener == NULL) {
        LOG_MSG_DEBUG("Loading QSEECom_register_listener failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->QSEECom_unregister_listener = dlsym(data->libHandle, "QSEECom_unregister_listener");
    if(handle->QSEECom_unregister_listener == NULL) {
        LOG_MSG_DEBUG("Loading QSEECom_unregister_listener failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->QSEECom_send_cmd = dlsym(data->libHandle, "QSEECom_send_cmd");
    if(handle->QSEECom_send_cmd == NULL) {
        LOG_MSG_DEBUG("Loading QSEECom_send_cmd failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->QSEECom_send_modified_cmd_32 = dlsym(data->libHandle, "QSEECom_send_modified_cmd");
    if(handle->QSEECom_send_modified_cmd_32 == NULL) {
        LOG_MSG_DEBUG("Loading QSEECom_send_modified_cmd failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->QSEECom_receive_req = dlsym(data->libHandle, "QSEECom_receive_req");
    if(handle->QSEECom_receive_req == NULL) {
        LOG_MSG_DEBUG("Loading QSEECom_receive_req failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->QSEECom_send_resp = dlsym(data->libHandle, "QSEECom_send_resp");
    if(handle->QSEECom_send_resp == NULL) {
        LOG_MSG_DEBUG("Loading QSEECom_send_resp failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->QSEECom_set_bandwidth = dlsym(data->libHandle, "QSEECom_set_bandwidth");
    if(handle->QSEECom_set_bandwidth == NULL) {
        LOG_MSG_DEBUG("Loading QSEECom_set_bandwidth failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->QSEECom_app_load_query = dlsym(data->libHandle, "QSEECom_app_load_query");
    if(handle->QSEECom_app_load_query == NULL) {
        LOG_MSG_DEBUG("Loading QSEECom_app_load_query failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->QSEECom_get_app_info = dlsym(data->libHandle, "QSEECom_get_app_info");
    handle->QSEECom_send_modified_cmd_64 = dlsym(data->libHandle, "QSEECom_send_modified_cmd_64");
    handle->QSEECom_start_app_V2 = dlsym(data->libHandle, "QSEECom_start_app_V2");

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

int qsee_free_handle(qsee_handle_t** handle_ptr)
{
    _priv_data_t *data = NULL;
    qsee_handle_t *handle = NULL;

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

static int32_t qcom_km_ion_memalloc(qcom_km_ion_info_t *handle, uint32_t size)
{
    int32_t ret;
    int32_t ion_fd;
    struct ion_allocation_data ion_alloc_data;
    struct ion_fd_data ifd_data;
    struct ion_handle_data handle_data;
    unsigned char *v_addr;

    if(handle == NULL) {
        LOG_MSG_ERROR("null handle");
        return -1;
    }

    memset(handle, 0, sizeof(qcom_km_ion_info_t));

    /* open ION device for memory management, O_DSYNC -> uncached memory */
    ion_fd = open("/dev/ion", O_RDONLY | O_DSYNC);
    if (ion_fd < 0) {
        LOG_MSG_ERROR("Cannot open ION device (%d:%s)", errno, strerror(errno));
        return -1;
    }

    handle->ion_fd = ion_fd;

    /* Size of allocation */
    ion_alloc_data.len = (size + 4095) & (~4095);
    /* 4K aligned */
    ion_alloc_data.align = 4096;
    /* memory is allocated from EBI heap */
    ion_alloc_data.heap_id_mask = ION_HEAP(ION_QSECOM_HEAP_ID);
    /* Set the memory to be uncached */
    ion_alloc_data.flags = 0;
    /* IOCTL call to ION for memory request */
    ret = ioctl(ion_fd, ION_IOC_ALLOC, &ion_alloc_data);
    if (ret) {
        LOG_MSG_ERROR("ION ALLOC failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    if (ion_alloc_data.handle == 0) {
        LOG_MSG_ERROR("ION alloc data returned a NULL");
        goto exit;
    }

    ifd_data.handle = ion_alloc_data.handle;
    handle->ion_alloc_handle.handle = ion_alloc_data.handle;
    LOG_MSG_VERBOSE("handle=%d", handle->ion_alloc_handle.handle);

    /* Call MAP ioctl to retrieve the ifd_data.fd file descriptor */
    ret = ioctl(ion_fd, ION_IOC_MAP, &ifd_data);
    if (ret) {
        goto exit;
    }

    handle->ifd_data_fd = ifd_data.fd;

    /* Make the ion mmap call */
    v_addr = (unsigned char *)mmap(NULL, ion_alloc_data.len, PROT_READ | PROT_WRITE,
                                   MAP_SHARED, ifd_data.fd, 0);
    if (v_addr == MAP_FAILED) {
        LOG_MSG_ERROR("ION MMAP failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->ion_sbuffer = v_addr;
    handle->sbuf_len = size;

    return 0;

exit:
    if (handle->ion_sbuffer != NULL) {
        ret = munmap(handle->ion_sbuffer, ion_alloc_data.len);
        if (ret) {
            LOG_MSG_ERROR("ION unmap failed (%d:%s)", errno, strerror(errno));
        }
        handle->ion_sbuffer = NULL;
    }

    if (handle->ifd_data_fd > 0) {
        close(handle->ifd_data_fd);
        handle->ifd_data_fd = -1;
    }

    handle_data.handle = handle->ion_alloc_handle.handle;
    if (handle_data.handle) {
        ret = ioctl(ion_fd, ION_IOC_FREE, &handle_data);
        if (ret) {
            LOG_MSG_ERROR("ION FREE failed (%d:%s)", errno, strerror(errno));
        }
        handle->ion_alloc_handle.handle = 0;
    }

    if (handle->ion_fd > 0) {
        close(handle->ion_fd);
        handle->ion_fd = -1;
    }

    return -1;
}

static int32_t qcom_km_ion_dealloc(qcom_km_ion_info_t *handle)
{
    int32_t ret = 0;
    struct ion_handle_data handle_data;

    if(handle == NULL) {
        return -1;
    }

    /* Deallocate the memory for the listener */
    if (handle->ion_sbuffer != NULL) {
        ret = munmap(handle->ion_sbuffer, (handle->sbuf_len + 4095) & (~4095));
        if (ret) {
            LOG_MSG_ERROR("ION unmap failed (%d:%s)", errno, strerror(errno));
        }
        handle->ion_sbuffer = NULL;
    }

    if (handle->ifd_data_fd > 0) {
        close(handle->ifd_data_fd);
        handle->ifd_data_fd = -1;
    }

    handle_data.handle = handle->ion_alloc_handle.handle;
    if (handle_data.handle) {
        ret = ioctl(handle->ion_fd, ION_IOC_FREE, &handle_data);
        if (ret) {
            LOG_MSG_ERROR("ION FREE failed (%d:%s)", errno, strerror(errno));
        }
        handle->ion_alloc_handle.handle = 0;
    }

    if (handle->ion_fd > 0) {
        close(handle->ion_fd);
        handle->ion_fd = -1;
    }

    LOG_MSG_VERBOSE("dealloc");
    return ret;
}

char* qsee_error_strings(int err)
{
    switch (err) {
    case QSEECOM_LISTENER_REGISTER_FAIL:
        return "QSEECom: Failed to register listener";
    case QSEECOM_LISTENER_ALREADY_REGISTERED:
        return "QSEECom: Listener already registered";
    case QSEECOM_LISTENER_UNREGISTERED:
        return "QSEECom: Listener unregistered";
    case QSEECOM_APP_ALREADY_LOADED:
        return "QSEECom: Trustlet already loaded";
    case QSEECOM_APP_NOT_LOADED:
        return "QSEECom: Trustlet not loaded";
    case QSEECOM_APP_QUERY_FAILED:
        return "QSEECom: Failed to query trustlet";
    default:
        return "QSEECom: Unknown error";
    }
}

static int qsee_load_trustlet(qsee_handle_t* qsee_handle, struct QSEECom_handle **clnt_handle,
                              const char *path, const char *fname, uint32_t sb_size)
{
    int ret = 0;
    int sz = sb_size;

    if (qsee_handle == NULL || clnt_handle == NULL || fname == NULL) {
        return -1;
    }

    if(sz < 1024) {
        LOG_MSG_DEBUG("Warning: size too small, increasing");
        sz = 1024;
    }

    LOG_MSG_VERBOSE("Starting app %s", fname);
    ret = qsee_handle->QSEECom_start_app(clnt_handle, path, fname, sz);
    if (ret < 0) {
        LOG_MSG_DEBUG("Could not load app %s (%d:%d:%s)", fname, ret, errno, strerror(errno));
    } else {
        LOG_MSG_VERBOSE("TZ App loaded : %s", fname);
    }

    return ret;
}

