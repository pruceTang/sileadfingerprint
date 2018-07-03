/******************************************************************************
 * @file   silead_log_ctrl.h
 * @brief  Contains log control API definitions.
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

#ifndef __SILEAD_FP_LOG_CTRL_H__
#define __SILEAD_FP_LOG_CTRL_H__

void silfp_log_ta_set_level(int32_t level);
void silfp_log_algo_set_level(const uint8_t level);

#endif /* __SILEAD_FP_LOG_CTRL_H__ */
