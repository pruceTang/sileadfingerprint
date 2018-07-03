/******************************************************************************
 * @file   silead_impl.h
 * @brief  Contains CA interfaces header file.
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
 * Jack Zhang  2018/5/17   0.1.2      Change test process to simplify app use
 * Rich Li     2018/5/28   0.1.4      Add get enroll number command ID.
 * Davie Wang  2018/6/1    0.1.5      Add capture image sub command ID.
 * David Wang  2018/6/5    0.1.6      Support wakelock & pwdn
 * Rich Li     2018/6/7    0.1.7      Support dump image
 * Jack Zhang  2018/6/15   0.1.8      Add read OTP I/F.
 * Rich Li     2018/7/2    0.1.9      Add algo set param command ID.
 *
 *****************************************************************************/

#ifndef __SILEAD_FINGERPRINT_IMPLEMENT_H__
#define __SILEAD_FINGERPRINT_IMPLEMENT_H__

#include "silead_screen.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t sl_fp_need_cancel_notice(void);

void sl_fp_cancel(void);
void sl_fp_sync_finger_status_optic(int32_t down);
int32_t sl_fp_get_screen_status(uint8_t *status);
int32_t sl_fp_set_screen_cb(screen_cb listen, void *param);

int32_t sl_fp_wait_finger_down(void);
int32_t sl_fp_wait_finger_up(void);
int32_t sl_fp_wait_finger_nav(void);
int32_t sl_fp_get_finger_down(void);
int32_t sl_fp_get_finger_up(void);
int32_t sl_fp_get_finger_up_uncancelable(void);

int32_t sl_fp_capture_image(int32_t times);
int32_t sl_fp_nav_capture_image(void);

int32_t sl_fp_auth_start(void);
int32_t sl_fp_auth_step(uint64_t op_id, uint32_t *fid);
int32_t sl_fp_auth_end(void);

int32_t sl_fp_enroll_start(void);
int32_t sl_fp_enroll_step(uint32_t *remaining);
int32_t sl_fp_enroll_end(uint32_t *fid);

int32_t sl_fp_nav_support(uint32_t *type);
int32_t sl_fp_nav_start(void);
int32_t sl_fp_nav_step(uint32_t *key);
int32_t sl_fp_nav_end(void);
int32_t sl_fp_send_key(uint32_t key);

int32_t sl_fp_download_normal(void);
int32_t sl_fp_init(void);
int32_t sl_fp_close(void);

int32_t sl_fp_set_gid(uint32_t gid);
int32_t sl_fp_load_user_db(const char* db_path);
int32_t sl_fp_remove_finger(uint32_t fid);
int32_t sl_fp_get_db_count(void);
int32_t sl_fp_get_finger_prints(uint32_t *ids, uint32_t count);

int64_t sl_fp_load_enroll_challenge(void);
int32_t sl_fp_set_enroll_challenge(uint64_t challenge);
int32_t sl_fp_verify_enroll_challenge(const void* hat, uint32_t size);
int64_t sl_fp_load_auth_id(void);
int32_t sl_fp_get_hw_auth_obj(void * buffer, uint32_t length);

int32_t sl_fp_set_log_mode(uint8_t krm, uint8_t tam, uint8_t agm);
int32_t sl_fp_set_nav_type(uint32_t mode);
int32_t sl_fp_calibrate(void);
int32_t sl_fp_get_enroll_num(uint32_t *num);
int32_t sl_fp_ci_chk_finger(void);
int32_t sl_fp_ci_adj_gain(void);
int32_t sl_fp_ci_shot(void);
int32_t sl_fp_alg_set_param(uint32_t idx, void *buffer, uint32_t *plen, uint32_t *result);
int32_t sl_fp_calibrate_step(uint32_t step);
int32_t sl_fp_optic_test_snr(uint32_t *snr, uint32_t *noise, uint32_t *signal);
int32_t sl_fp_optic_test_factory_quality(uint32_t *result, uint32_t *quality, uint32_t*length);

int32_t sl_fp_get_dev_ver(char *version, uint32_t len);
int32_t sl_fp_get_ta_ver(uint32_t *algoVer, uint32_t *taVer);
int32_t sl_fp_get_chipid(uint32_t *chipId, uint32_t *subId);
int32_t sl_fp_reset_test(void);
int32_t sl_fp_get_otp(void);

int32_t sl_fp_test_get_image_info(uint32_t *w, uint32_t *h, uint32_t *size, uint32_t *w_ori, uint32_t *h_ori);
int32_t sl_fp_test_image_capture(uint32_t gentpl, void *buffer, uint32_t *len, uint8_t *quality, uint8_t *area, uint8_t *istpl);
int32_t sl_fp_test_frrfar_send_group_image(uint32_t frr, void *buffer, uint32_t *len);
int32_t sl_fp_test_image_finish(void);
int32_t sl_fp_test_deadpx(uint32_t *result, uint32_t *deadpx, uint32_t *badline);
int32_t sl_fp_test_speed(void *buffer, uint32_t *len);

int32_t sl_fp_wakelock(uint8_t lock);
int32_t sl_fp_chip_pwdn(void);
int32_t sl_fp_create_proc_node(void *chipname);

int32_t sl_fp_set_hbm_mode(uint32_t mode);
int32_t sl_fp_set_brightness(uint32_t mode);
int32_t sl_cb_optic_test_factory_quality();
void sileadHypnusSetAction();

#ifdef SIL_DUMP_IMAGE
typedef enum {
    DUMP_IMG_AUTH_SUCC = 0,
    DUMP_IMG_AUTH_FAIL,
    DUMP_IMG_ENROLL_SUCC,
    DUMP_IMG_ENROLL_FAIL,
    DUMP_IMG_NAV_SUCC,
    DUMP_IMG_NAV_FAIL,
    DUMP_IMG_SHOT_SUCC,
    DUMP_IMG_SHOT_FAIL,
    DUMP_IMG_RAW,
    DUMP_IMG_FT,
    DUMP_IMG_AUTH_ORIG,
    DUMP_IMG_ENROLL_ORIG,
    DUMP_IMG_MAX,
} e_mode_dump_img_t;

int32_t sl_fp_dump_data(e_mode_dump_img_t type);
#endif /* SIL_DUMP_IMAGE */

typedef enum {
    FUN_CALIBRATE_START = 0,
    FUN_CALIBRATE_CMD1 = 1,
    FUN_CALIBRATE_CMD2,
    FUN_CALIBRATE_CMD3,
    FUN_CALIBRATE_CMD4,
    FUN_CALIBRATE_CMD5,
    FUN_CALIBRATE_CMD6,
    FUN_CALIBRATE_CMD7,
    FUN_CALIBRATE_CMD8,
    FUN_CALIBRATE_CMD9,
    FUN_CALIBRATE_CMD10,
    FUN_FINGERPRINT_TEST1 = 0x101,
    FUN_FINGERPRINT_TEST2 = 0x102,
    FUN_FINGERPRINT_TEST_FINISH = 0x110,
    FUN_AGING_TEST =  0x201,
    FUN_AGING_TEST_FINISH =  0x202,
} send_fingerprint_cmd_t;

#define BRIGHTNESS_1	205
#define BRIGHTNESS_2	410
#define BRIGHTNESS_3	615
#define BRIGHTNESS_4	820
#define BRIGHTNESS_5	1022
#define BRIGHTNESS_ALL 750

#ifdef __cplusplus
}
#endif

#endif /* __SILEAD_FINGERPRINT_IMPLEMENT_H__ */
