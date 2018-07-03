/******************************************************************************
 * @file   silead_fp_storage_impl.h
 * @brief  Contains fingerprint Storage implements header file.
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
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * Luke Ma     2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_FP_STORAGE_IMPL_H__
#define __SILEAD_FP_STORAGE_IMPL_H__

typedef struct {
    uint32_t (*normal_style)(void);
    int32_t (*file_exsit)(const uint8_t *path);
    int32_t (*check_dir)(const uint8_t *path);
    int32_t (*read_file)(const uint8_t *fname, uint8_t *buf, uint32_t size);
    int32_t (*write_file)(const uint8_t *fname, const uint8_t *buf, uint32_t len);
    int32_t (*remove_file)(const uint8_t *fname);
} store_if_t;

int32_t silfp_storage_getif(store_if_t *store);

#endif /* __SILEAD_FP_STORAGE_IMPL_H__ */
