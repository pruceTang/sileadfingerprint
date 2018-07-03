/******************************************************************************
 * @file   silead_logmsg.h
 * @brief  Contains log control header file.
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

#ifndef __LOGMSG_QSEE_TZ_H__
#define __LOGMSG_QSEE_TZ_H__

#ifndef LOG_DBG
    #define LOG_DBG 1
#endif
#ifndef LOG_DBG_VERBOSE
    #define LOG_DBG_VERBOSE 1
#endif

#undef LOG_TAG
#define LOG_TAG "fp-qsee"
//#define FILE_TAG ""  // should be defined in each file

//==================================================================================================
#ifndef LOG_DBG
    #define LOG_DBG 0
#endif

#ifndef LOG_DBG_VERBOSE
    #define LOG_DBG_VERBOSE 0
#endif

#ifndef FILE_TAG
    #define FILE_TAG "fp"
#endif

#include "qsee_log.h"
#include "silead_base.h"

#ifndef QSEE_LOG
#define QSEE_LOG(xx_prio, xx_fmt, ...) qsee_log(xx_prio, #xx_fmt, ##__VA_ARGS__)
#endif /* !QSEE_LOG */

#define SLFP_LOG(fmt, ...) QSEE_LOG(QSEE_LOG_MSG_ERROR, fmt, ##__VA_ARGS__)

#if LOG_DBG
    #if LOG_DBG_VERBOSE
        #define LOG_MSG_VERBOSE(fmt, ...) SLFP_LOG_IF(0, "[%s][%s][VRB][%s:%d] " fmt, LOG_TAG, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #else
        #define LOG_MSG_VERBOSE(fmt, ...) ((void)0)
    #endif
    #define LOG_MSG_DEBUG(fmt, ...)   SLFP_LOG_IF(1, "[%s][%s][DBG][%s:%d] " fmt, LOG_TAG, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define LOG_MSG_INFO(fmt, ...)    SLFP_LOG_IF(2, "[%s][%s][INFO][%s:%d] " fmt, LOG_TAG, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)

    #define LOG_FUNC_ENTRY()          SLFP_LOG_IF(1, "[%s][%s]~~~~~~~~~~~ +%s ~~~~~~~~~~~", LOG_TAG, FILE_TAG, __FUNCTION__ )
    #define LOG_FUNC_EXIT()           SLFP_LOG_IF(1, "[%s][%s]~~~~~~~~~~~ -%s ~~~~~~~~~~~", LOG_TAG, FILE_TAG, __FUNCTION__ )
#else
    #define LOG_MSG_VERBOSE(fmt, ...) ((void)0)
    #define LOG_MSG_DEBUG(fmt, ...)   ((void)0)
    #define LOG_MSG_INFO(fmt, ...)    ((void)0)

    #define LOG_FUNC_ENTRY()          ((void)0)
    #define LOG_FUNC_EXIT()           ((void)0)
#endif

#define LOG_MSG_WARNING(fmt, ...) SLFP_LOG_IF(3, "[%s][%s][WRN][%s:%d] " fmt, LOG_TAG, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_MSG_ERROR(fmt, ...)   SLFP_LOG_IF(4, "[%s][%s][ERR][%s:%d] " fmt, LOG_TAG, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define SLFP_LOG_IF(level, ...) \
    if (silfp_log_ta_logable(level)) { \
        SLFP_LOG(__VA_ARGS__); \
    }

//==================================================================================================

int32_t silfp_log_ta_logable(uint8_t level);

#endif // __LOGMSG_QSEE_TZ_H__

