/******************************************************************************
 * @file   silead_fp_memory.c
 * @brief  Contains fingerprint memory functions.
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

#define FILE_TAG "mem_qsee"
#include "silead_logmsg.h"

#ifndef SECURE_QSEE_CUSTOM_MALLOC

#include "silead_base.h"

#ifdef QSEE_WRAP_MEM
void *__wrap_malloc(MALLOC_SIZE size)
#else
void *malloc(MALLOC_SIZE size)
#endif /* QSEE_WRAP_MEM */
{
    return qsee_malloc(size);
}

#ifdef QSEE_WRAP_MEM
void __wrap_free(void *p)
#else
void free(void *p)
#endif /* QSEE_WRAP_MEM */
{
    if (p != NULL) {
        qsee_free(p);
    }
}

void * __attribute__((weak)) qsee_realloc(void *__ptr, MALLOC_SIZE __size);

#ifdef QSEE_WRAP_MEM
void *__wrap_realloc(void *__ptr, MALLOC_SIZE __size)
#else
void *_realloc(void *__ptr, MALLOC_SIZE __size)
#endif /* QSEE_WRAP_MEM */
{
    void *p = NULL;

    if (qsee_realloc) {
        return qsee_realloc(__ptr,__size);
    } else if ( __size ) {
        p = qsee_malloc(__size);
        //memcpy(p,__ptr,__size);
    }
    if (__ptr != NULL) {
        qsee_free(__ptr);
    }
    return p;
}

#ifdef SIL_IDIV0
void __aeabi_idiv0(void)
{
    //Todo, divided by zero
}

void __aeabi_uidiv0(void)
{
    //Todo, divided by zero
}
#endif /* SIL_IDIV0 */
#endif /* !SECURE_QSEE_CUSTOM_MALLOC */
