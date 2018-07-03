/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef TLSPI_H_
#define TLSPI_H_

#include "public/tci.h"

/**
 * Termination codes
 */
#define EXIT_ERROR ((uint32_t)(-1))

typedef struct {
    tciCommandHeader_t  header;     /**< Command header */
    uint32_t len;
    uint32_t v_addr;
    uint32_t v_addr2;
    uint32_t data;
    uint32_t data2;
    uint32_t data3;
    uint32_t data4;
} sil_cmd_t;

typedef struct {
    tciResponseHeader_t header;     /**< Response header */
    uint32_t len;
    uint32_t data;
    uint32_t data2;
    uint32_t data3;
    uint32_t data4;
    int32_t status;
} sil_rsp_t;

typedef struct {
    union {
        sil_cmd_t    cmd;
        sil_rsp_t    rsp;
    };
} tciSilMessage_t;

#endif // TLSPI_H_
