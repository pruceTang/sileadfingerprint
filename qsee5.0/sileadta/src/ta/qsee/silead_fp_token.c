/******************************************************************************
 * @file   silead_fp_token.c
 * @brief  Contains fingerprint token functions.
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
 * Luke Ma     2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "token_qsee"
#include "silead_logmsg.h"

#include "silead_base.h"
#include "silead_fp_token_impl.h"

extern int32_t fp_token_internal_set_key_data(const void *buffer, const uint32_t len);
extern int32_t fp_token_internal_generate_hmac(hw_auth_token_t *auth);

#ifdef SIL_TOKEN_FAKE
static int32_t silfp_is_fake_token()
{
    return SIL_FAKE;
}
#endif /* SIL_TOKEN_FAKE */

static int32_t silfp_token_set_key_data(const void *buffer, const uint32_t len)
{
    return fp_token_internal_set_key_data(buffer, len);
}

static int32_t silfp_token_generate_hmac(hw_auth_token_t *auth)
{
    return fp_token_internal_generate_hmac(auth);
}

int32_t silfp_token_getif(token_if_t *token)
{
    if (token == NULL) {
        return -1;
    }
    
    memset(token, 0, sizeof(*token));
    token->set_key_data = silfp_token_set_key_data;
    token->generate_hmac = silfp_token_generate_hmac;
#ifdef SIL_TOKEN_FAKE
    token->is_fake_token = silfp_is_fake_token;
#endif /* SIL_TOKEN_FAKE */

    return 0;
}

