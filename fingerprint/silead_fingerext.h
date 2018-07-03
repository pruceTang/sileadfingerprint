/******************************************************************************
 * @file   silead_fingerext.h
 * @brief  Contains fingerprint extension operate functions header file.
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
 * Jack Zhang  2018/5/17   0.1.1      Change test process to simplify app use
 *
 *****************************************************************************/

#ifndef __SILEAD_FINGER_EXT_HUB_H__
#define __SILEAD_FINGER_EXT_HUB_H__

#ifdef __cplusplus
extern "C" {
#endif

int32_t silfp_ext_start(void);
int32_t silfp_ext_stop(void);

int32_t silfp_ext_test_cmd(const uint8_t *param, int32_t len);
int32_t silfp_ext_request_commond();

#ifdef __cplusplus
}
#endif

#endif // __SILEAD_FINGER_EXT_HUB_H__