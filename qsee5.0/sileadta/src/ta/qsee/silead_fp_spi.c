/******************************************************************************
 * @file   silead_fp_spi.c
 * @brief  Contains fingerprint SPI functions.
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
 * Gorden Zhu  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "spi_qsee"
#include "silead_logmsg.h"

#ifndef SECURE_QSEE_CB

#include "qsee_spi.h"

#include "silead_base.h"
#include "silead_fp_spi_impl.h"
#include "silead_error.h"

//static uint8 qsee_rx[1024] __attribute__((aligned(64)));
//static uint8 qsee_tx[1024] __attribute__((aligned(64)));

//#define SPI_DEV QSEE_SPI_DEVICE_1
static qsee_spi_device_id_t SPI_DEV = QSEE_SPI_DEVICE_16;
static qsee_spi_config_t m_spi_config;

static int32_t fp_get_page_len()
{
    return (1024*32 - 4);
}

static int32_t fp_spi_read(int32_t fd, TRANSFER_T *tr)
{
#if 1
    int32_t ret = 0;
    qsee_spi_transaction_info_t qtr, qtw;

    qtr.buf_addr = tr->rx;
    qtr.buf_len  = tr->len;
    qtw.buf_addr = tr->tx;
    qtw.buf_len  = tr->len;
	//LOG_MSG_DEBUG("fp_spi_read start ");

    ret = qsee_spi_full_duplex(SPI_DEV, &m_spi_config, &qtw, &qtr);
	//LOG_MSG_DEBUG("fp_spi_read end  ret %d", ret);

    if ( ret >= 0 ) {
        return qtr.total_bytes ? qtr.total_bytes : qtr.buf_len;
    }

    return ret;
#endif
#if 0
//
//QSEE_V5
	int32_t ret = 0;
	uint8_t *p = tr->tx;
	uint8_t *q = tr->rx;
	qsee_spi_transaction_info_t qtr, qtw;
	qtw.buf_addr = tr->tx;
	qtw.buf_len  = tr->len;
	memset(++p, 0, tr->len-1);
	memset(q, 0, tr->len);
	qtr.buf_addr = tr->rx;
	qtr.buf_len  = tr->len;
	ret = qsee_spi_full_duplex(SPI_DEV, &m_spi_config, &qtw, &qtr);
	if ( ret >= 0 ) {
		return qtr.total_bytes ? qtr.total_bytes : qtr.buf_len;
	}
	return ret;
#endif
}

static int32_t fp_spi_write(int32_t fd, TRANSFER_T *tr)
{
    int32_t ret = 0;
    qsee_spi_transaction_info_t qtw;

    qtw.buf_addr = tr->tx;
    qtw.buf_len  = tr->len;
	//LOG_MSG_DEBUG("fp_spi_write start ");

    ret = qsee_spi_write(SPI_DEV, &m_spi_config, &qtw);
    //LOG_MSG_DEBUG("fp_spi_write end  ret %d", ret);
    if ( ret >= 0 ) {
        return qtw.total_bytes ? qtw.total_bytes : qtw.buf_len;
    }
    return ret;
}

static int32_t fp_spi_init(int32_t fd, const uint32_t data1, const uint32_t data2, const uint32_t data3, const uint32_t data4)
{
    int32_t ret;

    //const uint32_t mode = (data1 & 0x000000FF);  // 8bits
    const uint32_t bits = ((data1 >> 8) & 0x000000FF); // 8bits
    //const uint32_t delay = ((data1 >> 16) & 0x0000FFFF); // 16bits
#ifdef QSEE_V4
    const uint32_t speed = data2;
#endif /* QSEE_V4 */
    const uint32_t dev_id = ((data3 >> 16) & 0x000000FF); // 8bits

    LOG_MSG_DEBUG("spi_init v01");

    if (dev_id > 0 && dev_id != 0xFF) {
#ifdef QSEE_V4
    	LOG_MSG_DEBUG("spi_id %d, speed = %d", dev_id, speed);
#else
    	LOG_MSG_DEBUG("spi_id %d", dev_id);
#endif
        SPI_DEV = (qsee_spi_device_id_t)(dev_id - 1);
    }

    ret = qsee_spi_open(SPI_DEV);
    if ( ret ) {
        qsee_spi_close(SPI_DEV);
        ret = qsee_spi_open(SPI_DEV);
    }
    LOG_MSG_DEBUG("open SPI(QSEE_SPI_DEVICE_%d) %s.", SPI_DEV+1, ret ? "fail":"succ");
    memset(&m_spi_config,0,sizeof(m_spi_config));

    /*if (mode & 0x01) {
        m_spi_config.spi_cs_polarity = QSEE_SPI_CS_ACTIVE_HIGH
    } else {
        m_spi_config.spi_cs_polarity = QSEE_SPI_CS_ACTIVE_LOW;
    }

    if (mode & 0x02) {
        m_spi_config.spi_clk_polarity = QSEE_SPI_CLK_IDLE_HIGH;
    } else {
        m_spi_config.spi_clk_polarity = QSEE_SPI_CLK_IDLE_LOW;
    }*/

    m_spi_config.spi_clk_polarity  = QSEE_SPI_CLK_IDLE_LOW;
    m_spi_config.spi_shift_mode    = QSEE_SPI_INPUT_FIRST_MODE;
    m_spi_config.spi_cs_polarity   = QSEE_SPI_CS_ACTIVE_LOW;
    //m_spi_config.spi_cs_mode     = QSEE_SPI_CS_DEASSERT;
    m_spi_config.spi_clk_always_on = QSEE_SPI_CLK_NORMAL;
    m_spi_config.spi_cs_mode       = QSEE_SPI_CS_KEEP_ASSERTED;
    m_spi_config.spi_bits_per_word = bits & SPI_BITS_MASK;
#ifdef QSEE_V4
    m_spi_config.max_freq          = 4800000; //speed; //4800000;
    m_spi_config.hs_mode           = 0;
    m_spi_config.loopback          = 0;
    m_spi_config.spi_slave_index   = 0;
    m_spi_config.deassertion_time_ns = 0;
#endif /* QSEE_V4 */

    if (m_spi_config.spi_bits_per_word != 8 && m_spi_config.spi_bits_per_word != 16 && m_spi_config.spi_bits_per_word != 32 ) {
        m_spi_config.spi_bits_per_word = 8;
    }

    return ret;
}

static int32_t fp_spi_deinit(int32_t fd)
{
    int32_t ret;
    LOG_MSG_VERBOSE("deinit");

    UNUSED(fd);

    ret = qsee_spi_close(SPI_DEV);
    LOG_MSG_VERBOSE("close SPI %s.", ret ? "fail":"succ");

    return ret;
}

static int32_t fp_spi_open(const uint8_t *spi_path)
{
    UNUSED(spi_path);
    return 1;
}

static int32_t fp_spi_close(int32_t fd)
{
    UNUSED(fd);
    return 0;
}

int32_t silfp_spi_getif(spi_if_t *spi)
{
    if (spi == NULL) {
        return -1;
    }

    memset(spi, 0, sizeof(*spi));

    spi->get_page_len = fp_get_page_len;
    spi->spi_read  = fp_spi_read;
    spi->spi_write = fp_spi_write;
    spi->init      = fp_spi_init;
    spi->deinit    = fp_spi_deinit;
    spi->open      = fp_spi_open;
    spi->close     = fp_spi_close;

    return 0;
}

#endif /* !SECURE_QSEE_CB */
