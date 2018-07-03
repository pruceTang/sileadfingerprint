/******************************************************************************
 * @file   silead_const.c
 * @brief  Contains fingerprint const definitions.
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

#ifndef __SILEAD_CONST_H__
#define __SILEAD_CONST_H__

#define TPL_MAX_ST 10
#define TPLID_S		128  /* Finger ID, range start */
#define TPLID_E		255  /* Finger ID, range end */
#define ID_VALID(x) ((x) >= TPLID_S && (x) <= TPLID_E)

#define TPL_MAX	5
#define TPL_UPD_STORE_THRESHOLD 5

#define ROUND16(x)  (((x)+15) & ~15)

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#define SPI_BITS_MASK  0x000000FF
#define SPI_DELAY_MASK 0x0000FFFF
#define SPI_MODE_MASK  0x000000FF

#define SIL_FAKE  0x0511

#endif /* __SILEAD_CONST_H__ */
