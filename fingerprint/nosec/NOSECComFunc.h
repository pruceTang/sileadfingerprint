/******************************************************************************
 * @file   NOSECComFunc.h
 * @brief  Contains Non-secure communication functions header file.
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

#ifndef __NOSEC_COMM_FUN_H__
#define __NOSEC_COMM_FUN_H__

#include <dlfcn.h>
#include <sys/mman.h>
#include <fcntl.h>

typedef struct nosec_handle {
    void *_data;

    void (*tz_app_init)(void);
    int32_t (*tz_app_cmd_handler)(void* cmd, uint32_t cmdlen, void* rsp, uint32_t rsplen);
    void (*tz_app_shutdown)(void);
} nosec_handle_t;

int32_t nosec_open_handle(nosec_handle_t **handle);
int32_t nosec_free_handle(nosec_handle_t** handle);

#endif /* __NOSEC_COMM_FUN_H__ */
