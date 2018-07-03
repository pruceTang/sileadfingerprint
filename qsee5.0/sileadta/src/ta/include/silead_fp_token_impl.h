/******************************************************************************
 * @file   silead_fp_token_impl.h
 * @brief  Contains fingerprint Token implements header file.
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

#ifndef __SILEAD_FP_TOKEN_IMPL_H__
#define __SILEAD_FP_TOKEN_IMPL_H__

#include "silead_hw_auth_token.h"

typedef struct {
    int32_t (*is_fake_token)(void);
    int32_t (*set_key_data)(const void *buffer, const uint32_t len);
    int32_t (*generate_hmac)(hw_auth_token_t *auth);
    int32_t (*get_hmac_key)(void *buffer, uint32_t len);
} token_if_t;

int32_t silfp_token_getif(token_if_t *token);

#endif /* __SILEAD_FP_TOKEN_IMPL_H__ */
