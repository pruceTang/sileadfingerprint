/******************************************************************************
 * @file   silead_fp_spi_impl.h
 * @brief  Contains fingerprint SPI implements header file.
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
 * Frank Yang  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_FP_SPI_IMPL_H__
#define __SILEAD_FP_SPI_IMPL_H__

typedef struct _transfer_t {
    uint8_t *tx;
    uint8_t *rx;
    uint16_t len;
} TRANSFER_T;

typedef struct {
    int32_t (*get_page_len)(void);
    int32_t (*spi_read)(int32_t fd, TRANSFER_T *tr);
    int32_t (*spi_write)(int32_t fd, TRANSFER_T *tr);
    int32_t (*write_cfg)(int32_t fd, const void *reg, const uint32_t len);
    int32_t (*init)(int32_t fd, const uint32_t data1, const uint32_t data2, const uint32_t data3, const uint32_t data4);
    int32_t (*deinit)(int32_t fd);
    int32_t (*open)(const uint8_t *spi_path);
    int32_t (*close)(int32_t fd);

    int32_t (*get_virtual_id)(void);

    int32_t (*read_reg)(int32_t fd, const uint32_t reg, uint32_t *val);
    int32_t (*write_reg)(int32_t fd, const uint32_t reg, const uint32_t val);
    int32_t (*get_frame)(int32_t fd, const uint32_t reg, void *buf, const uint32_t size, const uint8_t ms_frm, const uint16_t spi_retry);
} spi_if_t;

int32_t silfp_spi_getif(spi_if_t *spi);

#endif /* __SILEAD_FP_SPI_IMPL_H__ */

