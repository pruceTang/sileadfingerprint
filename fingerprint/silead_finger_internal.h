/******************************************************************************
 * @file   silead_finger_internal.h
 * @brief  Contains fingerprint operate functions header file.
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

#ifndef __SILEAD_FINGER_INTERNAL_H__
#define __SILEAD_FINGER_INTERNAL_H__

typedef enum worker_state {
    STATE_IDLE = 0,
    STATE_ENROLL,
    STATE_SCAN,
    STATE_NAV,
    STATE_EXIT,
    STATE_TEST,
    STATE_WAIT,
    STATE_EXT_CB,
    STATE_LOCK,
    STATE_BREAK,
    STATE_MAX,
} worker_state_t;

void silfp_finger_set_work_state_no_signal(worker_state_t state);
void silfp_finger_set_work_state(worker_state_t state);
worker_state_t silfp_finger_get_work_state(void);
int silfp_finger_is_canceled();
void silfp_finger_wait_work_condition(int32_t timeout_sec);

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
void silfp_send_touch_down_notice();
void silfp_send_touch_up_notice();
void silfp_send_hardware_notice();
#endif

#endif // __SILEAD_FINGER_INTERNAL_H__