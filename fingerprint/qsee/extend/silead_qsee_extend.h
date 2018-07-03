/******************************************************************************
 * @file   silead_qsee_extend.h
 * @brief  Contains QSEE communication extension functions header file.
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
 *****************************************************************************/

#ifndef __SILEAD_QSEE_EXTEND_H__
#define __SILEAD_QSEE_EXTEND_H__

int32_t silfp_qsee_send_modified_command_to_tz_with_ion(uint32_t cmd, struct qcom_km_ion_info ihandle, uint32_t len);
int32_t silfp_qsee_send_normal_command(uint32_t cmd, uint32_t v, uint32_t *r1, uint32_t *r2);

int32_t silfp_qsee_custom_mem_init(qsee_handle_t *handle);
void silfp_qsee_custom_mem_deinit();

int32_t silfp_cb_qsee_init(qsee_handle_t *handle, uint32_t svrid);
void silfp_cb_qsee_deinit();

#endif /* __SILEAD_QSEE_EXTEND_H__ */