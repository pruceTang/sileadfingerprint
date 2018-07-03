/******************************************************************************
 * @file   silead_test.h
 * @brief  Contains factory test functions header file.
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
 * Jack Zhang  2018/5/17   0.1.1      Change test process to simplify app use
 *
 *****************************************************************************/

#ifndef __FINGERPRINT_TEST_H__
#define __FINGERPRINT_TEST_H__

#define IMG_TEST_FEATURE_GEN_TPL_MASK 0x00000001
#define IMG_TEST_FEATURE_ORIGINAL_MASK 0x00000002
#define IMG_TEST_FEATURE_IMG_QUALITY_MASK 0x00000004
#define IMG_TEST_FEATURE_IMG_DATA_MASK 0x00000008

typedef enum {
    TEST_CMD_SPI = 0,
    TEST_CMD_TEST_RESET_PIN,
    TEST_CMD_TEST_DEAD_PIXEL,
    TEST_CMD_GET_VERSION,
    TEST_CMD_TEST_IMAGE_CAPTURE,
    TEST_CMD_WAIT_FINGER_UP, // compatible legacy test
    TEST_CMD_FRR_FAR_SEND_IMAGE,
    TEST_CMD_FRR_FAR_SEND_IMAGE_CLEAR,
    TEST_CMD_IMAGE_FINISH,  // compatible legacy test
    TEST_CMD_TEST_SPEED,
    TEST_CMD_TEST_FINISH,
    TEST_CMD_CALIBRATE,
    TEST_CMD_CALIBRATE_STEP,
    TEST_CMD_SEND_FINGER_DOWN,
    TEST_CMD_SEND_FINGER_UP,
} test_cmd_type_t;

int32_t silfp_test_data_verify(const void *buffer, uint32_t len, uint32_t *offset);

/*return the buffer data length*/
int32_t silfp_test_set_err(void *buffer, uint32_t size, int32_t err);

int32_t silfp_test_deinit(void);

int32_t silfp_test_get_versions(void *buffer, uint32_t size);
int32_t silfp_test_get_chipid(void *buffer, uint32_t size);
int32_t silfp_test_reset_test(void *buffer, uint32_t size);

int32_t silfp_test_get_image_size(void);
int32_t silfp_test_image_capture(uint32_t feature, void *buffer, uint32_t size);
int32_t silfp_test_frrfar_send_group_image(const void *buffer, uint32_t offset, const uint32_t len, void *rsp, const uint32_t maxlen);
int32_t silfp_test_image_finish(void);
int32_t silfp_test_deadpx(void *buffer, uint32_t size);
int32_t silfp_test_speed(void *buffer, uint32_t size);

int32_t silfp_test_calibrate(void);
int32_t silfp_test_calibrate_step(uint32_t step);
int32_t silfp_test_set_err_with_byte(void *buffer, uint32_t size, int32_t err, int32_t data);

#endif /* __FINGERPRINT_TEST_H__ */
