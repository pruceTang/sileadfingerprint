/******************************************************************************
 * @file   logmsg.h
 * @brief  Contains log message header file.
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

#ifndef __LOGMSG_H__
#define __LOGMSG_H__

#ifndef LOG_DBG_DYNAMIC
    #define LOG_DBG_DYNAMIC 0
#endif
#ifndef LOG_DBG
    #define LOG_DBG 1
#endif
#ifndef LOG_DBG_VERBOSE
    #ifdef SIL_DEBUG_ALL_LOG
        #define LOG_DBG_VERBOSE 1
    #else
        #define LOG_DBG_VERBOSE 0
    #endif
#endif

#undef LOG_TAG
#define LOG_TAG "fingerprint"
//#define FILE_TAG ""  // should be defined in each file

//==================================================================================================
#ifndef LOG_DBG_DYNAMIC
    #define LOG_DBG_DYNAMIC 0
#endif

#ifndef LOG_DBG
    #define LOG_DBG 0
#endif

#ifndef LOG_DBG_VERBOSE
    #define LOG_DBG_VERBOSE 0
#endif

#if LOG_DBG_DYNAMIC
    #undef LOG_DBG
    #define LOG_DBG 1
#endif

#if LOG_DBG
    #undef LOG_NDEBUG
    #define LOG_NDEBUG 0
#endif

#ifndef FILE_TAG
    #define FILE_TAG "fp"
#endif

#include <utils/Log.h>

#if LOG_DBG
    #if LOG_DBG_DYNAMIC
        #if LOG_DBG_VERBOSE
            #define LOG_MSG_VERBOSE(fmt, ...) ALOGV_IF(isLoggable(0), "[%s][VRB][%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
        #else
            #define LOG_MSG_VERBOSE(fmt, ...) ((void)0)
        #endif
        #define LOG_MSG_DEBUG(fmt, ...)   ALOGD_IF(isLoggable(1), "[%s][DBG][%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
        #define LOG_MSG_INFO(fmt, ...)    ALOGI_IF(isLoggable(2), "[%s][INFO][%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)

        #define LOG_FUNC_ENTRY()          ALOGD_IF(isLoggable(1), "[%s]~~~~~~~~~~~ +%s ~~~~~~~~~~~", FILE_TAG, __FUNCTION__ )
        #define LOG_FUNC_EXIT()           ALOGD_IF(isLoggable(1), "[%s]~~~~~~~~~~~ -%s ~~~~~~~~~~~", FILE_TAG, __FUNCTION__ )
    #else
        #if LOG_DBG_VERBOSE
            #define LOG_MSG_VERBOSE(fmt, ...) ALOGV("[%s][VRB][%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
            #define LOG_MSG_DEBUG(fmt, ...)   ALOGD("[%s][DBG][%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
        #else
            #define LOG_MSG_VERBOSE(fmt, ...) ((void)0)
            #define LOG_MSG_DEBUG(fmt, ...)   ((void)0)
       #endif
        #define LOG_MSG_INFO(fmt, ...)    ALOGI("[%s][INFO][%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)

        #define LOG_FUNC_ENTRY()          ALOGD("[%s]~~~~~~~~~~~ +%s ~~~~~~~~~~~", FILE_TAG, __FUNCTION__ )
        #define LOG_FUNC_EXIT()           ALOGD("[%s]~~~~~~~~~~~ -%s ~~~~~~~~~~~", FILE_TAG, __FUNCTION__ )
    #endif
#else
    #define LOG_MSG_VERBOSE(fmt, ...) ((void)0)
    #define LOG_MSG_DEBUG(fmt, ...)   ((void)0)
    #define LOG_MSG_INFO(fmt, ...)    ((void)0)

    #define LOG_FUNC_ENTRY()          ((void)0)
    #define LOG_FUNC_EXIT()           ((void)0)
#endif

#define LOG_MSG_WARNING(fmt, ...) ALOGW("[%s][WRN][%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_MSG_ERROR(fmt, ...)   ALOGE("[%s][ERR][%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

int isLoggable(int level);

#ifdef __cplusplus
}
#endif

//==================================================================================================

#endif // __LOGMSG_H__
