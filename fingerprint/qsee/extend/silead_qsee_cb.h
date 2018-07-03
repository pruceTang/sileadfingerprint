/******************************************************************************
 * @file   silead_qsee_cb.h
 * @brief  Contains QSEE communication callback command IDs.
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

#ifndef __SILEAD_QSEE_CB_H__
#define __SILEAD_QSEE_CB_H__

enum fp_tz_cb_cmd {
    TZ_FP_CB_BASE = 0x00010000,
    TZ_FP_CB_EXIT,
    TZ_FP_CB_SPI_OPEN,
    TZ_FP_CB_SPI_CLOSE,
    TZ_FP_CB_SPI_RD,
    TZ_FP_CB_SPI_WR,
    TZ_FP_CB_SPI_DOWN_CFG,
    TZ_FP_CB_SPI_GET_FRAME,
    TZ_FB_CB_MAX,
};

#endif /* __SILEAD_QSEE_CB_H__ */