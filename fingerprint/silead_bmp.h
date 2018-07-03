/******************************************************************************
 * @file   silead_bmp.h
 * @brief  Contains Bitmap operate functions header file.
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
 * Jack Zhang  2018/4/2    0.1.0      Init version
 * Rich Li     2018/6/7    0.1.1      Support dump image
 *
 *****************************************************************************/

#ifndef __SILEAD_BITMAP_H__
#define __SILEAD_BITMAP_H__

uint32_t silfp_bmp_get_size(const uint32_t w, const uint32_t h);
int32_t silfp_bmp_get_img(void *dest, const uint32_t size, const void *buf, const uint32_t w, const uint32_t h);
int32_t silfp_bmp_get_data(void *dest, const uint32_t w, const uint32_t h, const void *buf, const uint32_t size);

int32_t silfp_bmp_save(const void *buf, const char *prefix, const uint32_t size, const uint32_t w, const uint32_t h);

#endif /* __SILEAD_BITMAP_H__ */

