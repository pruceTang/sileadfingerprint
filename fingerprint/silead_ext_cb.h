/******************************************************************************
 * @file   silead_ext_cb.h
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
 * Jack Zhang  2018/5/18    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_EXT_CB_H__
#define __SILEAD_EXT_CB_H__

#ifdef __cplusplus
extern "C" {
#endif

int32_t silfp_ext_cb_init();
int32_t silfp_ext_cb_deinit();

int32_t silfp_ext_cb_get_module(char *buffer, uint32_t size);
int32_t silfp_ext_cb_spi_test();
int32_t silfp_ext_cb_image_quality_get();
int32_t silfp_ext_cb_image_quality_finish();
int32_t silfp_ext_cb_image_capture_loop();
int32_t silfp_ext_cb_calibrate_step(int32_t step);
int32_t silfp_et_cb_optic_test_factory_quality();

#ifdef __cplusplus
}
#endif

#endif /* __SILEAD_EXT_CB_H__ */

