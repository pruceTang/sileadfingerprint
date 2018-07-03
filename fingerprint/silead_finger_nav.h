/******************************************************************************
 * @file   silead_finger_nav.h
 * @brief  Contains fingerprint navi operate functions header file.
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
 * Jack Zhang  2018/6/12   0.1.1      Add ESD protect under idle/nav mode
 *
 *****************************************************************************/

#ifndef __SILEAD_FINGER_NAV_H__
#define __SILEAD_FINGER_NAV_H__

int32_t silfp_nav_init();
int32_t silfp_nav_deinit();

int32_t silfp_nav_force_support(void);
int32_t silfp_nav_check_support(void);
int32_t silfp_nav_command();

#endif // __SILEAD_FINGER_NAV_H__