/******************************************************************************
 * @file   silead_log_ctrl.c
 * @brief  Contains fingerprint log level control functions.
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
 * Frank Yang  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#include "silead_logmsg.h"

#include "silead_base.h"

// 0:VERBOSE 1:DEBUG 2:INFO 3:WARNING 4:ERROR
static uint8_t m_log_ta_level = 1;

int32_t silfp_log_ta_logable(uint8_t level)
{
    return (level >= m_log_ta_level) ? 1 : 0;
}

void silfp_log_ta_set_level(const uint8_t level)
{
    m_log_ta_level = level;
}


static uint8_t m_sil_algo_loglevel = 0;
void silfp_log_algo_set_level(const uint8_t level)
{
    if (level <= 3) {
        m_sil_algo_loglevel = level;
    }
}

void sil_loge(const char *fmt, ...)
{
    va_list argp;
    char log[1024] = { 0 };

    va_start(argp, fmt);
    vsnprintf(log, sizeof(log)-1, fmt, argp);
    va_end(argp);

    SLFP_LOG("[fp-algo] %s\n",log);
}

void sil_logd(const char *fmt, ...)
{
    va_list argp;
    char log[1024] = { 0 };

    if ( m_sil_algo_loglevel >= 1 ) {
        va_start(argp, fmt);
        vsnprintf(log, sizeof(log)-1, fmt, argp);
        va_end(argp);

        SLFP_LOG("[fp-algo] %s\n",log);
    }
}

void sil_logi(const char *fmt, ...)
{
    va_list argp;
    char log[1024] = { 0 };

    if ( m_sil_algo_loglevel >= 2 ) {
        va_start(argp, fmt);
        vsnprintf(log, sizeof(log)-1, fmt, argp);
        va_end(argp);

        SLFP_LOG("[fp-algo] %s\n",log);
    }
}
