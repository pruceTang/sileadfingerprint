/******************************************************************************
 * @file   silead_qsee_spi_cb.h
 * @brief  Contains QSEE Callback SPI functions header file.
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

#ifndef __SILEAD_QSEE_SPI_CB_H__
#define __SILEAD_QSEE_SPI_CB_H__

#include <dlfcn.h>
#include <sys/mman.h>
#include <fcntl.h>

typedef struct spi_cb_handle {
    void *_data;

    int32_t (*fp_spi_init)(int32_t fd, const uint32_t data1, const uint32_t data2, const uint32_t data3, const uint32_t data4);
    int32_t (*fp_spi_deinit)(int32_t fd);
    int32_t (*fp_spi_open)(const char *spi_path);
    int32_t (*fp_spi_close)(int32_t fd);
    int32_t (*fp_spi_read_reg)(int32_t fd, const uint32_t reg, uint32_t *val);
    int32_t (*fp_spi_write_reg)(int32_t fd, const uint32_t reg, const uint32_t val);
    int32_t (*fp_spi_write_cfg2)(int32_t fd, const void *reg, const uint32_t len);
    int32_t (*fp_spi_get_frame2)(int32_t fd, const uint32_t reg, void *buf, const uint32_t size, const uint8_t ms_frm, const uint16_t spi_retry);
} spi_cb_handle_t;

int silfp_spi_cb_open_handle(spi_cb_handle_t **handle);
int silfp_spi_cb_free_handle(spi_cb_handle_t **handle);

#endif /* __SILEAD_QSEE_SPI_CB_H__ */
