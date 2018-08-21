/******************************************************************************
 * @file   silead_fp.h
 * @brief  Contains CA communication interfaces.
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
 * David Wang  2018/5/15   0.1.1      Support get finger status
 * John Zhang  2018/5/15   0.1.2      Support get config
 * Jack Zhang  2018/5/17   0.1.3      Change test process to simplify app use
 * Rich Li     2018/5/28   0.1.4      Add get enroll number command ID.
 * Davie Wang  2018/6/1    0.1.5      Add capture image sub command ID.
 * Davie Wang  2018/6/15   0.1.6      Add optics command ID.
 * Rich Li     2018/7/2    0.1.7      Add algo set param command ID.
 *
 *****************************************************************************/

#ifndef __SILEAD_FP_H__
#define __SILEAD_FP_H__

#include "silead_dev.h"

enum _module_mode_ {
    IRQ_UP = 0,
    IRQ_DOWN = 1,
    IRQ_NAV = 2,
    NORMAL = 3,
};

typedef struct silead_fp_handle {
    int32_t (*fp_wait_finger_status)(uint32_t status);
    int32_t (*fp_capture_image)(void);
    int32_t (*fp_nav_capture_image)(void);

    int32_t (*fp_auth_start)(void);
    int32_t (*fp_auth_step)(uint64_t op_id, uint32_t step, uint32_t *fid);
    int32_t (*fp_auth_end)(void);

    int32_t (*fp_enroll_start)(void);
    int32_t (*fp_enroll_step)(uint32_t *remaining);
    int32_t (*fp_enroll_end)(int32_t status, uint32_t *fid);

    int32_t (*fp_nav_support)(uint32_t *type);
    int32_t (*fp_nav_start)(void);
    int32_t (*fp_nav_step)(uint32_t *pkey);
    int32_t (*fp_nav_end)(void);

    int32_t (*fp_download_normal)(void);
    int32_t (*fp_init)(FP_DEV_CONF *dev_conf, uint32_t *chipid, uint32_t *subid, uint32_t *vid, uint32_t *update_cfg);
    int32_t (*fp_deinit)(void);

    int32_t (*fp_set_gid)(uint32_t gid, uint32_t serial);
    int32_t (*fp_load_user_db)(const char* db_path);
    int32_t (*fp_remove_finger)(uint32_t fid);
    int32_t (*fp_get_db_count)(void);
    int32_t (*fp_get_finger_prints)(uint32_t *ids, uint32_t size);

    int64_t (*fp_load_enroll_challenge)(void);
    int32_t (*fp_set_enroll_challenge)(uint64_t challenge);
    int32_t (*fp_verify_enroll_challenge)(const void* hat, uint32_t size);
    int64_t (*fp_load_auth_id)(void);
    int32_t (*fp_get_hw_auth_obj)(void * buffer, uint32_t length);

    int32_t (*fp_update_cfg)(const void* buffer, uint32_t len);
    int32_t (*fp_init2)(const void *buffer, const uint32_t len, uint32_t *feature, uint32_t *tpl_size);

    int32_t (*fp_load_template)(uint32_t fid, const void *buffer, uint32_t len);
    int32_t (*fp_save_template)(void *buffer, uint32_t *plen);
    int32_t (*fp_update_template)(void *buffer, uint32_t *plen, uint32_t *fid);

    int32_t (*fp_set_log_mode)(uint8_t tam, uint8_t agm);
    int32_t (*fp_set_nav_type)(uint32_t mode);
    int32_t (*fp_get_finger_status)(uint32_t *status, uint32_t *addition);
    int32_t (*fp_get_config)(void *buffer, uint32_t *plen);
    int32_t (*fp_get_enroll_num)(uint32_t *num);
    int32_t (*fp_calibrate)(void *buffer, uint32_t len);
    int32_t (*fp_calibrate2)(void);
    int32_t (*fp_ci_chk_finger)(void);
    int32_t (*fp_ci_adj_gain)(int32_t enroll);
    int32_t (*fp_ci_shot)(int32_t enroll);
    int32_t (*fp_alg_set_param)(uint32_t cmd, void *buffer, uint32_t *plen, uint32_t *result);
    int32_t (*fp_chk_esd)(void);
    int32_t (*fp_get_otp)(uint32_t *otp1, uint32_t *otp2, uint32_t *otp3);
    int32_t (*fp_calibrate_optic)(uint32_t step, uint8_t *buffer, uint32_t *plen);

    int32_t (*fp_get_version)(uint32_t *algo, uint32_t *ta);
    int32_t (*fp_get_chip_id)(uint32_t *chipid, uint32_t *subid);

    int32_t (*fp_test_get_image_info)(uint32_t *w, uint32_t *h, uint32_t *size, uint32_t *w_ori, uint32_t *h_ori);
    int32_t (*fp_test_dump_data)(uint32_t type, void *buffer, uint32_t len, uint32_t *size, uint32_t *w, uint32_t *h);

    int32_t (*fp_test_image_capture)(uint32_t gentpl, void *buffer, uint32_t *len, uint8_t *quality, uint8_t *area, uint8_t *istpl);
    int32_t (*fp_test_frrfar_send_group_image)(const uint32_t frr, void *buffer, uint32_t *plen);
    int32_t (*fp_test_image_finish)(void);

    int32_t (*fp_test_deadpx)(uint32_t *result, uint32_t *deadpx, uint32_t *badline);
    int32_t (*fp_test_speed)(void *buffer, uint32_t *plen);
    int32_t (*fp_test_cmd)(uint32_t cmd, void *buffer, uint32_t *plen, uint32_t *result);
} silead_fp_handle_t;

const silead_fp_handle_t * silfp_get_impl_handler(const void *ta_name);

#endif /* __SILEAD_FP_H__ */

