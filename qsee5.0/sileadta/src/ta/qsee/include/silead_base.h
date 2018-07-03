/******************************************************************************
 * @file   silead_base.h
 * @brief  Contains base header file.
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

#ifndef __TEE_FP_BASE_H__
#define __TEE_FP_BASE_H__

#ifndef QSEE_V4
#define BASE_TYPE_DEF
#endif /* !QSEE_V4 */
#define SECURE_OS

#ifdef BASE_TYPE_DEF
typedef unsigned __int64 uint64_t;
typedef signed __int64  int64_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef uint16 uint16_t;
typedef int16  int16_t;
typedef uint8  uint8_t;
typedef int8   int8_t;
typedef unsigned int UINTNT;
#endif

#include "comdef.h"
#include "qsee_services.h"
#include "qsee_heap.h"
#include "qsee_cipher.h"
#include "qsee_hash.h"
#include "qsee_fs.h"
#include "qsee_heap.h"
#include <stringl.h>

#include "silead_const.h"

#if defined(AARCH64_SUPPORT)
typedef unsigned long UINTNT;
typedef unsigned long MALLOC_SIZE;
#else
typedef uint32_t MALLOC_SIZE;
#endif /* AARCH64_SUPPORT */

void *malloc(MALLOC_SIZE size);
void free(void *p);

#endif /* __TEE_FP_BASE_H__ */
