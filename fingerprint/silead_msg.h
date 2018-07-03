/******************************************************************************
 * @file   silead_msg.h
 * @brief  Contains pipe communication header file.
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

#ifndef __SILEAD_MESSAGE_H__
#define __SILEAD_MESSAGE_H__

enum _msg_t {
    SIFP_MSG_UNKNOW = -1,
    SIFP_MSG_CANCEL = 0,
    SIFP_MSG_IRQ = 1,
    SIFP_MSG_MAX,
};

int32_t silfp_msg_init(void);
void silfp_msg_deinit(void);

void silfp_msg_send(int32_t type);
int32_t silfp_msg_recv(int32_t uncancelable);
void silfp_msg_clean(void);

#endif /* __SILEAD_MESSAGE_H__ */