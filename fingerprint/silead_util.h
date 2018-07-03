/******************************************************************************
 * @file   silead_util.h
 * @brief  Contains fingerprint utilities functions header file.
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

#ifndef __SILEAD_UTIL_H__
#define __SILEAD_UTIL_H__

int32_t silfp_util_need_cancel_notice(void);

int32_t silfp_util_update_all_log_level(uint8_t *logs, uint32_t size);
int32_t silfp_util_get_dump_level(void);

#endif /* __SILEAD_UTIL_H__ */
