/******************************************************************************
 * @file   silead_hw_auth_token.h
 * @brief  Contains fingerprint Auth token structures.
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

#ifndef __SILEAD_HW_AUTH_TOKEN_H__
#define __SILEAD_HW_AUTH_TOKEN_H__

typedef enum {
    AUTH_TYPE_GATEKEEPER = 0,
    AUTH_TYPE_FINGERPRINT = 1,
} hw_auth_token_type_t;

typedef enum {
    HW_AUTH_GATEKEEPER = 0,
    HW_AUTH_FINGERPRINT = 1,
} hw_authenticator_type_t;

#define HW_AUTH_TOKEN_VERSION 0
typedef struct __attribute__((packed)) _auth_token {
    uint8_t version;
    uint64_t challenge;
    uint64_t user_id;
    uint64_t authenticator_id;
    uint32_t authenticator_type;
    uint64_t timestamp;
    uint8_t hmac[32];
} hw_auth_token_t;

#endif // __SILEAD_HW_AUTH_TOKEN_H__
