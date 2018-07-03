/******************************************************************************
 * @file   silead_device.h
 * @brief  Contains get target device information functions header file.
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

#ifndef __SILEAD_DEVICE_H__
#define __SILEAD_DEVICE_H__

int32_t slifp_device_print_version();
int32_t silfp_device_info_get(void **buffer);
void silfp_device_info_release(void *buffer);

int32_t silfp_device_get_user_key(uint32_t uid, const char *key_name);

#endif /* __SILEAD_DEVICE_H__ */
