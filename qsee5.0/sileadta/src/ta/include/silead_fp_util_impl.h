/******************************************************************************
 * @file   silead_fp_util_impl.h
 * @brief  Contains fingerprint misc utilities implements header file.
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
 * Thomas He   2018/4/2    0.1.0      Init version
 *
 *
 *****************************************************************************/

#ifndef __SILEAD_FP_UTIL_IMPL_H__
#define __SILEAD_FP_UTIL_IMPL_H__

#define STATS_TIME_MODE_CA 0x01
#define STATS_TIME_MODE_TA 0x02

typedef struct {
    int32_t (*is_fake_crypto)(void);
    int32_t (*unk0_init)(uint32_t *pid);             // if not used, not need implement
    int32_t (*unk1_init)(void *addr, uint32_t len);  // if not used, not need implement
    int32_t (*unk1_deinit)(void);                    // if not used, not need implement
    int32_t (*unk2_init)(void *addr, uint32_t len);  // if not used, not need implement
    int32_t (*unk2_deinit)(void);                    // if not used, not need implement
    int64_t (*get_64bit_rand)(void);
    uint64_t (*get_time)(void);
    int32_t (*msleep)(uint32_t ms);
    int32_t (*auth_tpl_update_support)(void);
    uint32_t (*auth_tpl_upd_store_threshold)(void);
    void (*set_fd)(int32_t fd);
    int32_t (*sync_auth_result)(uint64_t op_id, uint32_t fid);
    int32_t (*sync_enroll_id_list)(uint32_t *fid_list, uint32_t count);
    int32_t (*sync_fp_version)(uint32_t version);
} util_if_t;

int32_t silfp_util_getif(util_if_t *util);

#endif /* __SILEAD_FP_UTIL_IMPL_H__ */

