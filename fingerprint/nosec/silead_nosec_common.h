/******************************************************************************
 * @file   silead_nosec_common.h
 * @brief  Contains Non-secure communication structures.
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
 *
 *****************************************************************************/

#ifndef __SILEAD_NOSEC_COMMON_H__
#define __SILEAD_NOSEC_COMMON_H__

struct __attribute__ ((packed)) send_cmd_rsp {
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
    uint32_t data4;
    int32_t status;
};

struct __attribute__ ((packed)) send_cmd {
    uint32_t cmd_id;
    void *v_addr;
    void *v_addr2;
    uint32_t length;
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
    uint32_t data4;
};

#endif /* __SILEAD_NOSEC_COMMON_H__ */
