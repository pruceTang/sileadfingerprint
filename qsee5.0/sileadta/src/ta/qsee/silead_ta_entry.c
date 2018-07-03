/******************************************************************************
 * @file   silead_ta_entry.c
 * @brief  Contains fingerprint TA entry.
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
 * Gorden Zhu  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "qsee-entry"
#include "silead_logmsg.h"

#include "qsee_services.h"

#include "silead_base.h"

char TZ_APP_NAME[] = {"sileadta"};

extern int32_t fp_ta_main_entry_point(void* cmd, uint32_t cmdlen, void* rsp, uint32_t rsplen);

/**
  @brief
    Add any app specific initialization code here
    QSEE will call this function after secure app is loaded and
    authenticated
*/
void tz_app_init(void)
{
    /*  App specific initialization code*/
#ifdef AARCH64_SUPPORT
    LOG_MSG_DEBUG("%s64 Init", TZ_APP_NAME);
#else
    LOG_MSG_DEBUG("%s Init", TZ_APP_NAME);
#endif /* AARCH64_SUPPORT */
}

void tz_app_cmd_handler(void* cmd, uint32_t cmdlen, void* rsp, uint32_t rsplen)
{
    /* Request-response buffers are allocated by non-secure side*/
    /* They are MPU protected by QSEE kernel before reaching here*/
    /* Add code to process requests and set response (if any)*/

    fp_ta_main_entry_point(cmd, cmdlen, rsp, rsplen);
}

/**
  @brief
    App specific shutdown
    App will be given a chance to shutdown gracefully
*/
void tz_app_shutdown(void)
{
    /* app specific shutdown code*/
#ifdef AARCH64_SUPPORT
    QSEE_LOG(QSEE_LOG_MSG_DEBUG, "%s64 App shutdown", TZ_APP_NAME);
#else
    QSEE_LOG(QSEE_LOG_MSG_DEBUG, "%s App shutdown", TZ_APP_NAME);
#endif /* AARCH64_SUPPORT */
    return;
}
