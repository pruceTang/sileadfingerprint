/******************************************************************************
 * @file   silead_version.h
 * @brief  Contains CA version definitions.
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
 * Jack Zhang  2018/4/2    0.1.0      Init version
 * Tiven Xie   2018/5/10   0.1.1      Update CA version to 4.15.1
 * Tiven Xie   2018/5/17   0.1.2      Update CA version to 4.15.2
 *
 *****************************************************************************/

#ifndef __SILEAD_VERSION_H__
#define __SILEAD_VERSION_H__

// n:non security t:trustonic q:qsee g:global platform
#define SECURITY_TYPE_NONE_VALUE "n"
#define SECURITY_TYPE_TEE_VALUE "t"
#define SECURITY_TYPE_QSEE_VALUE "q"
#define SECURITY_TYPE_GP_VALUE "g"

#define VERSION_FP_MAJOR      "4"
#define VERSION_FP_MINOR      "15"
#define VERSION_FP_REVISION   "6"

#ifndef BUILD_DATE
#define BUILD_DATE "XXXXXXXXXXXX"
#endif

#ifndef SECURITY_TYPE_NOSEC
#undef SECURITY_TYPE_NONE_VALUE
#define SECURITY_TYPE_NONE_VALUE
#endif

#ifndef SECURITY_TYPE_TEE
#undef SECURITY_TYPE_TEE_VALUE
#define SECURITY_TYPE_TEE_VALUE
#endif

#ifndef SECURITY_TYPE_QSEE
#undef SECURITY_TYPE_QSEE_VALUE
#define SECURITY_TYPE_QSEE_VALUE
#endif

#ifndef SECURITY_TYPE_GP
#undef SECURITY_TYPE_GP_VALUE
#define SECURITY_TYPE_GP_VALUE
#endif

#define VERSION_SEC_TYPE SECURITY_TYPE_NONE_VALUE SECURITY_TYPE_TEE_VALUE SECURITY_TYPE_QSEE_VALUE SECURITY_TYPE_GP_VALUE

#ifdef PLATFORM_VERSION
#define FP_HAL_VERSION (VERSION_FP_MAJOR "." VERSION_FP_MINOR "." VERSION_FP_REVISION "(" BUILD_DATE "-" PLATFORM_VERSION "-" VERSION_SEC_TYPE ")")
#else
#define FP_HAL_VERSION (VERSION_FP_MAJOR "." VERSION_FP_MINOR "." VERSION_FP_REVISION "(" BUILD_DATE "-" VERSION_SEC_TYPE ")")
#endif

#endif /* __SILEAD_VERSION_H__ */
