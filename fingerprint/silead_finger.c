/******************************************************************************
 * @file   silead_finger.c
 * @brief  Contains fingerprint operate functions.
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
 * David Wang  2018/5/15   0.1.1      Support get finger status
 * Jack Zhang  2018/5/17   0.1.2      Change test process to simplify app use
 * David Wang  2018/6/1    0.1.3      Support CaptureImage sub command & auth retry
 * Rich Li     2018/6/7    0.1.4      Support dump image
 * Jack Zhang  2018/6/12   0.1.5      Add ESD protect under idle/nav mode
 * Jack Zhang  2018/6/15   0.1.6      Read OTP during init.
 *
 *****************************************************************************/

#define FILE_TAG "silead_finger"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <endian.h>
#include <errno.h>

#include "silead_finger.h"

#include "silead_impl.h"
#include "silead_error.h"
#include "silead_stats.h"

#include "silead_finger_nav.h"
#include "silead_finger_internal.h"
#include "silead_fingerext.h"
#include "silead_ext_cb.h"

fingerprint_notify_t m_notify_func;

pthread_t m_tid_workerthread;
worker_state_t m_worker_state;
uint8_t m_work_cancel;

pthread_mutex_t m_lock;
pthread_cond_t m_worker_cond;

uint8_t m_screen_status;

uint32_t m_gid;
uint64_t m_auth_op_id;
uint64_t m_auth_id;

int32_t m_need_send_cancel_notice;

int32_t m_enroll_pause = 0;
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
volatile static int32_t m_finger_ready = 0;
#endif

uint32_t m_pre_enroll_time = 0; // seconds

#define ENROLL_TIME_OUT_MAX 600 // 10 minutes

//=====================================================================
// Function declaration
static void _finger_send_error_notice(int32_t error);
static void _finger_send_enroll_notice(uint32_t fid, uint32_t remaining);
static void _finger_send_removed_notice(uint32_t fid, uint32_t gid, uint32_t remaining);
static void _finger_send_acquired_notice(fingerprint_acquired_info_t acquired);
static void _finger_send_authenticated_notice(uint32_t fid, uint32_t gid);

static worker_state_t _finger_get_work_state_unlock(void);
static void _finger_set_work_state_unlock(worker_state_t state);

static int _finger_enroll_command();
static int _finger_auth_command();
static int _finger_lock_mode_commond();

int32_t silfp_ext_cb_command();

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
int silfp_clear_finger_ready_flag();
inline int silfp_is_finger_ready();
int silfp_wait_for_finger_ready();
#endif

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
void silfp_send_enumerated_notice();
void silfp_send_monitor_power_notice(double battery);

void silfp_monitor_power_start();
void silfp_monitor_power_reset();
void silfp_monitor_power_end();
#endif
//=====================================================================

static inline const char *get_work_state_string(int state)
{
    const char *stateString[] = {
        "STATE_IDLE",
        "STATE_ENROLL",
        "STATE_SCAN",
        "STATE_NAV",
        "STATE_EXIT",
        "STATE_TEST",
        "STATE_WAIT",
        "STATE_EXT_CB",
        "STATE_LOCK",
        "STATE_BREAK",
    };

    if (state >= 0 && state < STATE_MAX) {
        return stateString[state];
    }

    return "UNKNOWN";
}

static uint32_t _finger_get_time()
{
    uint32_t times;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    times = tv.tv_sec;

    return times;
}

static int32_t _finger_enroll_pause(int32_t pause)
{
    LOG_MSG_VERBOSE("pause=%d, m_enroll_pause=%d", pause, m_enroll_pause);
    if (pause && (STATE_ENROLL == silfp_finger_get_work_state())) {
        m_enroll_pause = 1;
        pthread_mutex_lock(&m_lock);
        pthread_cond_signal(&m_worker_cond);
        pthread_mutex_unlock(&m_lock);
    } else if ((!pause) && m_enroll_pause) {
        m_enroll_pause = 0;
        pthread_mutex_lock(&m_lock);
        pthread_cond_signal(&m_worker_cond);
        pthread_mutex_unlock(&m_lock);
    }
    return 0;
}

uint64_t silfp_finger_pre_enroll()
{
    uint64_t challenge;
    challenge = sl_fp_load_enroll_challenge();

    LOG_MSG_VERBOSE("challenge = %" PRIu64, challenge);

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
    m_pre_enroll_time = _finger_get_time();
#endif

    return challenge;
}

int silfp_finger_enroll(const hw_auth_token_t *hat, uint32_t __unused gid, uint32_t __unused timeout_sec)
{
    int ret;

    LOG_MSG_VERBOSE("enroll(gid=%u, timeout_sec=%u)", gid, timeout_sec);

    ret = sl_fp_verify_enroll_challenge((const void *)hat, sizeof(hw_auth_token_t));
    if (ret < 0) {
        return ret;
    }

    silfp_finger_set_work_state(STATE_ENROLL);

    return SL_SUCCESS;
}

int silfp_finger_post_enroll()
{
    sl_fp_set_enroll_challenge(0);

    LOG_MSG_VERBOSE("post_enroll");
    return SL_SUCCESS;
}

int silfp_finger_authenticate(uint64_t operation_id, uint32_t __unused gid)
{
    LOG_MSG_VERBOSE("authenticate(sid=%" PRIu64 ", gid=%d)", operation_id, gid);

    m_auth_op_id = operation_id;
    silfp_finger_set_work_state(STATE_SCAN);

    return SL_SUCCESS;
}

uint64_t silfp_finger_get_auth_id()
{
    m_auth_id = sl_fp_load_auth_id();
    m_auth_id = htobe64(m_auth_id);

    LOG_MSG_VERBOSE("m_auth_id = %" PRIu64, m_auth_id);

    return m_auth_id;
}

int silfp_finger_cancel()
{
    LOG_MSG_VERBOSE("cancel()");

    silfp_finger_set_work_state(STATE_IDLE);

    if (m_need_send_cancel_notice) {
        _finger_send_error_notice(FINGERPRINT_ERROR_CANCELED);
    }

    return SL_SUCCESS;
}

int silfp_finger_remove(uint32_t __unused gid, uint32_t fid)
{
    int ret;
    int i;
    LOG_MSG_VERBOSE("remove(fid=%u, gid=%u)", fid, gid);

    pthread_mutex_lock(&m_lock);
    if (fid == 0) {
        int count = sl_fp_get_db_count();
        if (count > 0) {
            uint32_t *ids = (uint32_t *)malloc(sizeof(uint32_t) * count);
            if (ids) {
                memset(ids, 0, sizeof(uint32_t) * count);
                count = sl_fp_get_finger_prints(ids, count);
                for (i = 0; i < count; i++) {
                    ret = sl_fp_remove_finger(ids[i]);
                    LOG_MSG_DEBUG("removeall(%d, %d)", ids[i], ret);
                    _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                    _finger_send_removed_notice(ids[i], m_gid, count - i -1);
                }
                free(ids);
            }
        }
    } else {
        ret = sl_fp_remove_finger(fid);
        LOG_MSG_DEBUG("remove(%d, %d)", fid, ret);
        if (ret < 0) {
            _finger_send_error_notice(FINGERPRINT_ERROR_UNABLE_TO_REMOVE);
        } else {
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
            int count = sl_fp_get_db_count();
            _finger_send_removed_notice(fid, m_gid, count);
#else
            _finger_send_removed_notice(fid, m_gid, 0);
#endif
        }
    }
    pthread_mutex_unlock(&m_lock);

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
    silfp_send_enumerated_notice();
#endif

    return SL_SUCCESS;
}

int silfp_finger_enumerate6(fingerprint_finger_id_t *results, uint32_t *max_size)
{
    int ret;
    int i = 0;

    LOG_MSG_VERBOSE("enumerate()");

    if (results == NULL || max_size == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }

    pthread_mutex_lock(&m_lock);
    int count = sl_fp_get_db_count();
    if (*max_size == 0) {
        *max_size = count;
        ret = count;
    } else {
        uint32_t *ids = (uint32_t *)malloc(sizeof(uint32_t) * count);
        if (ids) {
            memset(ids, 0 , sizeof(uint32_t) * count);
            count = sl_fp_get_finger_prints(ids, count);
            for (i = 0; i < count && i < (int)(*max_size); i++) {
                results[i].fid = ids[i];
                results[i].gid = m_gid;
            }
            ret = i;
            *max_size = i;
            free(ids);
        } else {
            ret = -SL_ERROR_OUT_OF_MEMORY;
        }
    }
    pthread_mutex_unlock(&m_lock);

    return ret;
}

int silfp_finger_enumerate()
{
    int i;

    LOG_MSG_VERBOSE("enumerate()");

    pthread_mutex_lock(&m_lock);
    int count = sl_fp_get_db_count();
    if (count > 0) {
        uint32_t *ids = (uint32_t *)malloc(sizeof(uint32_t) * count);
        if (ids) {
            memset(ids, 0 , sizeof(uint32_t) * count);
            count = sl_fp_get_finger_prints(ids, count);
            for (i = 0; i < count; i++) {
                _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);

                fingerprint_msg_t msg;
                msg.type = FINGERPRINT_TEMPLATE_ENUMERATING;
                msg.data.enumerated.finger.fid = ids[i];
                msg.data.enumerated.finger.gid = m_gid;
                msg.data.enumerated.remaining_templates = (uint32_t)(count - i - 1);
                if (m_notify_func != NULL) {
                    m_notify_func(&msg);
                }
            }
            free(ids);
        }
    } else {
        fingerprint_msg_t msg;
        msg.type = FINGERPRINT_TEMPLATE_ENUMERATING;
        msg.data.enumerated.finger.fid = 0;
        msg.data.enumerated.finger.gid = 0;
        msg.data.enumerated.remaining_templates = 0;
        if (m_notify_func != NULL) {
            m_notify_func(&msg);
        }
    }
    pthread_mutex_unlock(&m_lock);

    return SL_SUCCESS;
}

int silfp_finger_set_active_group(uint32_t gid, const char *store_path)
{
    int err = FINGERPRINT_ERROR;

    LOG_MSG_VERBOSE("set_active_group(%d, %s)", gid, store_path);

    pthread_mutex_lock(&m_lock);
    m_gid = gid;

    err = sl_fp_set_gid(gid);
    if (err >= 0) {
        err = sl_fp_load_user_db((char *)store_path);
    }
    pthread_mutex_unlock(&m_lock);

    if (err >= 0) {
        err = SL_SUCCESS;
    }
    LOG_MSG_DEBUG("err(%d)", err);

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
    silfp_send_enumerated_notice();
#endif

    return err;
}

int silfp_finger_set_notify_callback(fingerprint_notify_t notify)
{
    LOG_MSG_VERBOSE("set notify");
    if(!notify) {
        return SL_ERROR_BAD_PARAMS;
    }

    m_notify_func = notify;

    return SL_SUCCESS;
}

static void _finger_send_error_notice(int32_t error)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_ERROR;
    msg.data.error = (fingerprint_error_t)error;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

static void _finger_send_enroll_notice(uint32_t fid, uint32_t remaining)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_TEMPLATE_ENROLLING;
    msg.data.enroll.finger.fid = fid;
    msg.data.enroll.finger.gid = m_gid;
    msg.data.enroll.samples_remaining = remaining;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

static void _finger_send_removed_notice(uint32_t fid, uint32_t gid, uint32_t remaining)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_TEMPLATE_REMOVED;
    msg.data.removed.finger.fid = fid;
    msg.data.removed.finger.gid = gid;
    msg.data.removed.remaining_templates = remaining;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

static void _finger_send_acquired_notice(fingerprint_acquired_info_t acquired)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    LOG_MSG_VERBOSE("acquired=%d", acquired);

    msg.type = FINGERPRINT_ACQUIRED;
    msg.data.acquired.acquired_info = acquired;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

static void _finger_send_authenticated_notice(uint32_t fid, uint32_t gid)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_AUTHENTICATED;
    msg.data.authenticated.finger.fid = fid;
    msg.data.authenticated.finger.gid = gid;

    if (fid != 0) {
        hw_auth_token_t hat;
        memset(&hat, 0, sizeof(hat));
        hat.challenge = m_auth_op_id;
        hat.authenticator_type = htobe32(HW_AUTH_FINGERPRINT);
        hat.version = HW_AUTH_TOKEN_VERSION;
        hat.authenticator_id = m_auth_id;
        hat.user_id = m_auth_id;

        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        hat.timestamp = htobe64((uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);

        sl_fp_get_hw_auth_obj(&hat, sizeof(hat));
        memcpy(&(msg.data.authenticated.hat), &hat, sizeof(hat));

        LOG_MSG_VERBOSE("challenge=%" PRIu64 " m_auth_op_id=%" PRIu64, hat.challenge, m_auth_op_id);
    }

    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

void silfp_finger_wait_work_condition(int32_t timeout_sec)
{
    struct timeval now;
    struct timespec outtime;

    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + timeout_sec;
    outtime.tv_nsec = now.tv_usec * 1000;

    pthread_mutex_lock(&m_lock);
    pthread_cond_timedwait(&m_worker_cond, &m_lock, &outtime);
    pthread_mutex_unlock(&m_lock);
}

void* _finger_commandloop(void __unused *arg)
{
    worker_state_t state;

    LOG_MSG_VERBOSE("enter");

    sl_fp_calibrate();

    while (silfp_finger_get_work_state() != STATE_EXIT) {
        LOG_MSG_INFO("work %s", get_work_state_string(silfp_finger_get_work_state()));
        pthread_mutex_lock(&m_lock);
        state = _finger_get_work_state_unlock();
        if (STATE_WAIT == state || STATE_BREAK == state || (state == STATE_IDLE && (!silfp_nav_check_support() || !m_screen_status) && !silfp_nav_force_support())) {
            LOG_MSG_INFO("wait----------");
            if (STATE_WAIT != state) {
                sl_fp_chip_pwdn();
            }
            pthread_cond_wait(&m_worker_cond, &m_lock);
        }
        m_work_cancel = 0;
        pthread_mutex_unlock(&m_lock);

        state = silfp_finger_get_work_state();
        if (state == STATE_ENROLL) {
            _finger_enroll_command();
        } else if (state == STATE_SCAN) {
            _finger_auth_command();
        } else if (state == STATE_NAV) {
            silfp_nav_command();
        } else if (state == STATE_TEST) {
            silfp_ext_request_commond();
        } else if (state == STATE_EXT_CB) {
            silfp_ext_cb_command();
        } else if (state == STATE_LOCK) {
            _finger_lock_mode_commond();
        }

        pthread_mutex_lock(&m_lock);
        if (!m_work_cancel) {
            if (silfp_nav_check_support() && ( m_screen_status || silfp_nav_force_support()) && _finger_get_work_state_unlock() == STATE_IDLE) {
                _finger_set_work_state_unlock(STATE_NAV);
            }
        }
        pthread_mutex_unlock(&m_lock);
    }

    LOG_MSG_VERBOSE("exit");

    return 0;
}

static inline worker_state_t _finger_get_work_state_unlock(void)
{
    return m_worker_state;
}

worker_state_t silfp_finger_get_work_state(void)
{
    worker_state_t state = STATE_IDLE;

    pthread_mutex_lock(&m_lock);
    state = _finger_get_work_state_unlock();
    pthread_mutex_unlock(&m_lock);

    return state;
}

static void _finger_set_work_state_unlock(worker_state_t state)
{
    LOG_MSG_VERBOSE("set to %s************", get_work_state_string(state));
    m_worker_state = state;
}

void silfp_finger_set_work_state_no_signal(worker_state_t state)
{
    pthread_mutex_lock(&m_lock);
    _finger_set_work_state_unlock(state);
    pthread_mutex_unlock(&m_lock);
}

void silfp_finger_set_work_state(worker_state_t state)
{
    pthread_mutex_lock(&m_lock);

    m_work_cancel = 1;
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
    silfp_clear_finger_ready_flag();
#endif
    sl_fp_cancel();

    if ((_finger_get_work_state_unlock() != STATE_EXIT ) && (_finger_get_work_state_unlock() != STATE_BREAK)) {
        _finger_set_work_state_unlock(state);
    }
    pthread_cond_signal(&m_worker_cond);
    pthread_mutex_unlock(&m_lock);

    //if (STATE_IDLE == silfp_finger_get_work_state()) {
        _finger_enroll_pause(0);
    //}
}

static void _finger_screen_state_callback(int32_t screenon, void __unused *param)
{
    pthread_mutex_lock(&m_lock);

    m_screen_status = screenon;
    LOG_MSG_VERBOSE("m_screen_status: %d", m_screen_status);

    worker_state_t state = _finger_get_work_state_unlock();
    if (silfp_nav_check_support() && state != STATE_EXIT && state != STATE_BREAK) {
        LOG_MSG_VERBOSE("work status: %s", get_work_state_string(state));
        if (state == STATE_IDLE && m_screen_status) {
            LOG_MSG_VERBOSE("should run navi, signal only*************");
            pthread_cond_signal(&m_worker_cond);
        } else if (state == STATE_NAV && !m_screen_status) {
            LOG_MSG_VERBOSE("should goto idle, cancel navi************");
            _finger_set_work_state_unlock(STATE_IDLE);
            m_work_cancel = 1;
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
            silfp_clear_finger_ready_flag();
#endif
            sl_fp_cancel();
        }
    }
    pthread_mutex_unlock(&m_lock);
}

inline int silfp_finger_is_canceled()
{
    return m_work_cancel;
}

int silfp_finger_init()
{
    int ret;

    LOG_MSG_VERBOSE("init");

    m_notify_func = NULL;

    m_tid_workerthread = 0;
    m_worker_state = STATE_IDLE;
    m_work_cancel = 0;
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
    silfp_clear_finger_ready_flag();
#endif

    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_worker_cond, NULL);

    m_screen_status = 0;

    m_gid = 0;
    m_auth_op_id = 0;
    m_auth_id = 0;

    m_enroll_pause = 0;

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
    m_need_send_cancel_notice = 0;
#else
    m_need_send_cancel_notice = sl_fp_need_cancel_notice();
#endif

    LOG_MSG_INFO("need cancel notice (%d)", m_need_send_cancel_notice);

    ret = sl_fp_init();
    if (ret >= 0) {
        sl_fp_get_otp();
        silfp_nav_init();

#ifndef SL_FP_FEATURE_OPPO_CUSTOMIZE
        sl_fp_get_screen_status(&m_screen_status);
        sl_fp_set_screen_cb(_finger_screen_state_callback, NULL);
#else
        m_screen_status = 0;
#endif

        silfp_ext_start();
        silfp_ext_cb_init();

        LOG_MSG_VERBOSE("screen status:%d", m_screen_status);

        ret = pthread_create(&m_tid_workerthread, NULL, _finger_commandloop, NULL);
        if(ret != 0) {
            LOG_MSG_ERROR("can't create thread (%d:%s)", ret, strerror(ret));
            ret = -1;
        }
    }

    return ret;
}

int silfp_finger_deinit()
{
    silfp_finger_set_work_state(STATE_EXIT);
    pthread_join(m_tid_workerthread, NULL);

    silfp_ext_cb_deinit();
    silfp_ext_stop();
    sl_fp_close();
    silfp_nav_deinit();

    m_notify_func = NULL;

    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_worker_cond);

    LOG_MSG_VERBOSE("destory");

    return 0;
}

static int32_t _finger_should_acquired_notice(int32_t value)
{
    int32_t warning = 0;
    switch (value) {
    case -SL_ERROR_EROLL_DUPLICATE:
    case -SL_ERROR_MOVE_TOO_FAST:
    case -SL_ERROR_DETECT_NO_FINGER:
    case -SL_ERROR_SAME_AREA:
    case -SL_ERROR_QUALITY_FAILED:
    case -SL_ERROR_COVERAREA_FAILED:
    case -SL_ERROR_QUALITY_COVERAREA_FAILED:
    case -SL_ERROR_FAKE_FINGER:
    case -SL_ERROR_GAIN_IMPROVE_TIMEOUT:
    case -SL_ERROR_SPI_TIMEOUT:
        warning = 1;
        break;
    }
    return warning;
}

static int _silfp_finger_capture_image(int32_t __unused type, int32_t __unused times)
{
    int ret = SL_SUCCESS;

    if (!times) {
        sileadHypnusSetAction();
    }
/**tp info struct for underglass fingerprint,add by pruce_tang_20180710 start **/
	sl_init_tp_touch_info();
    silfp_get_tp_touch_info(0);

    ret = sl_fp_ci_chk_finger();
    if (ret >= 0) {
        do {
            ret = sl_fp_ci_adj_gain();
        } while(ret > 0);
    }

#ifdef SIL_DUMP_IMAGE
    sl_fp_dump_data(type ? DUMP_IMG_AUTH_ORIG : DUMP_IMG_ENROLL_ORIG);
#endif
/**tp info struct for underglass fingerprint,add by pruce_tang_20180710 start **/
    silfp_get_tp_touch_info(1);
    if (ret >= 0) {
        ret = sl_fp_ci_shot();
    }

#ifdef SIL_DUMP_IMAGE
    if (ret < 0) {
        sl_fp_dump_data(DUMP_IMG_SHOT_FAIL);
    }
#endif /* SIL_DUMP_IMAGE */

    return ret;
}

static int _finger_enroll_command()
{
    int ret = SL_SUCCESS;
    uint32_t remaining;
    uint32_t fid;
    int status = -1;
    uint32_t enroll_time;

    LOG_MSG_INFO("enroll-------------");
    enroll_time = _finger_get_time();
    if (enroll_time < m_pre_enroll_time || enroll_time - m_pre_enroll_time >= ENROLL_TIME_OUT_MAX) {
        _finger_send_error_notice(FINGERPRINT_ERROR_TIMEOUT);
        silfp_finger_set_work_state(STATE_IDLE);
        LOG_MSG_DEBUG("enroll timeout, change to idle");
        return 0;
    }

    ret = sl_fp_enroll_start();
    if (ret >= 0) {
        do {
            if (m_enroll_pause) {
                LOG_MSG_VERBOSE("enroll pause, wait continue enroll");
                pthread_mutex_lock(&m_lock);
                pthread_cond_wait(&m_worker_cond, &m_lock);
                pthread_mutex_unlock(&m_lock);

                if (m_work_cancel) {
                    break;
                }
            }

            ret = sl_fp_get_finger_down();
            if (m_enroll_pause) {
                continue;
            }

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
            if(ret >= 0) {
                ret = silfp_wait_for_finger_ready();
                if (m_enroll_pause) {
                    continue;
                }
            }
#endif

            if (ret >= 0) {
#ifndef SIL_DUMP_IMAGE
                ret = sl_fp_capture_image(0);
#else
                ret = _silfp_finger_capture_image(0, 0);
#endif
            }
            if (ret >= 0) {
                ret = sl_fp_enroll_step(&remaining);
#ifdef SIL_DUMP_IMAGE
                sl_fp_dump_data((ret >=0) ? DUMP_IMG_ENROLL_SUCC : DUMP_IMG_ENROLL_FAIL);
#endif /* SIL_DUMP_IMAGE */
            }

            if (ret < 0) {
                LOG_MSG_DEBUG("enroll: err(%d)", -ret);
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
                switch (ret) {
                    case -SL_ERROR_CANCELED: {
                        break;
                    }
                    case -SL_ERROR_EROLL_DUPLICATE: {
                        _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                        _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_ALREADY_ENROLLED);
                        break;
                    }
                    case -SL_ERROR_SAME_AREA: {
                        _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                        _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_TOO_SIMILAR);
                        break;
                    }
                    case -SL_ERROR_COVERAREA_FAILED: {
                        _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                        _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_PARTIAL);
                        break;
                    }
                    case -SL_ERROR_DETECT_NO_FINGER:
                    case -SL_ERROR_MOVE_TOO_FAST:{
                        _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                        _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_TOO_FAST);
                        break;
                    }						
                    case -SL_ERROR_FAKE_FINGER:
                    case -SL_ERROR_GAIN_IMPROVE_TIMEOUT:
                    case -SL_ERROR_SPI_TIMEOUT:
                    case -SL_ERROR_QUALITY_COVERAREA_FAILED:
                    case -SL_ERROR_QUALITY_FAILED: {
                        _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                        _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                        break;
                    }
                    default: {
                        _finger_send_error_notice(-ret);
                        break;
                    }
                }
#else
                if (ret == -SL_ERROR_CANCELED) { // canceled
                    break;
                } else if (_finger_should_acquired_notice(ret)) { // warning
                    _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                    _finger_send_acquired_notice((fingerprint_acquired_info_t)(-ret));
                } else { // error
                    _finger_send_error_notice(-ret);
                }
#endif
            } else {
                if (remaining > 0) {
                    _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                    _finger_send_enroll_notice(0, remaining);
                } else if (remaining == 0) { // enroll finish
                    break;
                }
            }

            if (ret != -SL_ERROR_CANCELED && !m_work_cancel) {
                sl_fp_get_finger_up();
            }
        } while (!m_work_cancel);
    }

    if (sl_fp_enroll_end(&fid) >= 0) { // return < 0, if not finish enroll or save tpl failed
        LOG_MSG_DEBUG("enroll: fid(%d) cancel:%d", fid, m_work_cancel);
        if (ret != -SL_ERROR_CANCELED && !m_work_cancel) {
            _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
            _finger_send_enroll_notice(fid, 0);

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
            silfp_send_enumerated_notice();
#endif

            silfp_finger_set_work_state_no_signal(STATE_IDLE);
            status = 0;
        }
    }
    LOG_MSG_DEBUG("enroll finish-------------");

    if (sl_fp_calibrate_step(4) > 0) {
        sl_fp_calibrate_step(5);
    }

    if (status >= 0) {
        sl_fp_get_finger_up();
    }

    return ret;
}

static int _finger_auth_command()
{
    int ret = SL_SUCCESS;
    int fid = -1;
    int status = -1;
    int i = 0;
    const int retry_max = 3;

    LOG_MSG_DEBUG("authenticate-------------");

    ret = sl_fp_auth_start();
    if (ret >= 0) {
        do {
            ret = sl_fp_wait_finger_down();
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
            if(ret >= 0) {
                ret = silfp_wait_for_finger_ready();
            }
#endif
            if (ret >= 0) {
                for (i = 0; i < retry_max; i++) {
                    silfp_stats_start();
#ifndef SIL_DUMP_IMAGE
                    ret = sl_fp_capture_image(i);
#else
                    ret = _silfp_finger_capture_image(1, i);
#endif
                    if (ret >= 0) {
                        silfp_stats_capture_image();
                        ret = sl_fp_auth_step(m_auth_op_id, i, (uint32_t *)&fid);
#ifdef SIL_DUMP_IMAGE
                        sl_fp_dump_data((ret >=0) ? DUMP_IMG_AUTH_SUCC : DUMP_IMG_AUTH_FAIL);
#endif /* SIL_DUMP_IMAGE */
                    }

                    if (ret >= 0) { // matched
                        break;
                    }
                }
            }

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
            silfp_monitor_power_reset();
#endif
            if (ret == -SL_ERROR_AUTH_MISMATCH) { // mismatch
                silfp_stats_auth_mismatch();
                LOG_MSG_INFO("authenticate: mismatch");
                _finger_send_authenticated_notice(0, 0);
            } else if (ret == -SL_ERROR_COVERAREA_FAILED || ret == -SL_ERROR_QUALITY_FAILED || ret == -SL_ERROR_QUALITY_COVERAREA_FAILED || ret == -SL_ERROR_FAKE_FINGER) { // mis touch
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
                silfp_monitor_power_start();

                if (m_screen_status) {
                    LOG_MSG_DEBUG("authenticate: mis-touch, screen on");
                    switch (ret) {
                    case -SL_ERROR_COVERAREA_FAILED: {
                        _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_PARTIAL);
                        break;
                    }
                    case -SL_ERROR_QUALITY_COVERAREA_FAILED:
                    case -SL_ERROR_QUALITY_FAILED: {
                        _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                        break;
                    }
                    default: {
                        _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                        break;
                    }
                    }
                    _finger_send_authenticated_notice(0, 0);
                } else {
                    LOG_MSG_DEBUG("authenticate: mis-touch, screen off");
                    _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_INSUFFICIENT);
                }
#else
                if (m_screen_status) {
                    LOG_MSG_DEBUG("authenticate: mis-touch, screen on");
                    _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                    _finger_send_authenticated_notice(0, 0);
                } else {
                    LOG_MSG_DEBUG("authenticate: mis-touch, screen off");
                    _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_INSUFFICIENT);
                }
#endif

            } else if (ret < 0) {
                LOG_MSG_INFO("authenticate: err(%d)", -ret);
                if (ret != -SL_ERROR_CANCELED) {
                    _finger_send_acquired_notice((fingerprint_acquired_info_t)(-ret));
                } else { // cancel
                    break;
                }
            } else {
                LOG_MSG_DEBUG("authenticate: fid(%d) cancel:%d", fid, m_work_cancel);
                if (!m_work_cancel) {
                    silfp_stats_auth_match();
                    _finger_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                    _finger_send_authenticated_notice(fid, 0);

                    silfp_finger_set_work_state_no_signal(STATE_IDLE);
                    status = 0;
                }
                break;
            }

            if (ret != -SL_ERROR_CANCELED && !m_work_cancel) {
                sl_fp_wait_finger_up();
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
                silfp_monitor_power_end();
#endif
            }
        } while (!m_work_cancel);
    }

    LOG_MSG_DEBUG("authenticate finish-------------");
    sl_fp_auth_end();

    if (sl_fp_calibrate_step(4) > 0) {
        sl_fp_calibrate_step(5);
    }

    if (status >= 0) {
        sl_fp_wait_finger_up();
    }

    return ret;
}

static int _finger_lock_mode_commond()
{
    int ret = SL_SUCCESS;

    LOG_MSG_INFO("lock-------------");

    do {
        ret = sl_fp_wait_finger_down();

        if (ret != -SL_ERROR_CANCELED && !m_work_cancel) {
            sl_fp_wait_finger_up();
        }
    } while (!m_work_cancel);

    LOG_MSG_DEBUG("lock finish-------------");

    return ret;
}

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
void silfp_send_engineering_image_quality_notice(int32_t successed, int32_t image_quality)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_ENGINEERING_INFO;
    msg.data.engineering.type=FINGERPRINT_IMAGE_QUALITY;
    msg.data.engineering.quality.successed = successed;
    msg.data.engineering.quality.image_quality = image_quality;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

void silfp_send_enumerated_notice()
{
    int ret;
    uint32_t count = MAX_ID_LIST_SIZE;
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    ret = silfp_finger_enumerate6(msg.data.enumerated_oppo.finger, &count);
    if(ret >= 0) {
        msg.type = FINGERPRINT_TEMPLATE_ENUMERATING;
        msg.data.enumerated_oppo.samples_remaining = count;
        msg.data.enumerated_oppo.gid = m_gid;
        if (m_notify_func != NULL) {
            m_notify_func(&msg);
        }
    }
}

void silfp_send_touch_down_notice()
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_TOUCH_DOWN;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

void silfp_send_touch_up_notice()
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_TOUCH_UP;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

void silfp_send_hardware_notice()
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_HARDWARE;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

void silfp_send_monitor_power_notice(double battery)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_MONITOR;
    msg.data.monitor.type = FINGERPRINT_POWER_MONITOR;
    msg.data.monitor.data.power.battery = battery;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}


uint64_t m_monitor_power_start = 0;
uint64_t m_is_mistouch_mode = 0;
void silfp_monitor_power_start()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    m_monitor_power_start = (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    m_is_mistouch_mode = 1;
}

void silfp_monitor_power_reset()
{
    m_monitor_power_start = 0;
    m_is_mistouch_mode = 0;
}

void silfp_monitor_power_end()
{
    if (m_is_mistouch_mode) {
        silfp_send_monitor_power_notice(3.14159);
    }
}

int silfp_get_enroll_total_times()
{
    uint32_t num;
    if (sl_fp_get_enroll_num(&num) < 0) {
        num = 0;
    }
    LOG_MSG_DEBUG("enroll count: %d", num);
    return num;
}

int silfp_pause_enroll()
{
    LOG_MSG_DEBUG("enroll pause");
    return _finger_enroll_pause(1);
}

int silfp_continue_enroll()
{
    LOG_MSG_DEBUG("enroll continue");
    return _finger_enroll_pause(0);
}

int silfp_pause_identify()
{
    LOG_MSG_DEBUG("pause identify not implement");
    return 0;
}

int silfp_continue_identify()
{
    LOG_MSG_DEBUG("pause identify not implement");
    return 0;
}

int silfp_get_alikey_status()
{
    LOG_MSG_DEBUG("pause identify not implement");
    return 0;
}

int silfp_cleanup()
{
    LOG_MSG_DEBUG("clean up");
    return silfp_finger_cancel();
}

int silfp_set_touch_event_listener()
{
    LOG_MSG_DEBUG("set touch event");
    silfp_finger_set_work_state(STATE_LOCK);
    return 0;
}

int silfp_set_screen_state(uint32_t sreenstate)
{
    LOG_MSG_DEBUG("set screen state %d", sreenstate);
    if (FINGERPRINT_SCREEN_OFF == sreenstate) {
        _finger_screen_state_callback(0, NULL);
    } else {
        _finger_screen_state_callback(1, NULL);
    }
    return 0;
}

int silfp_dynamically_config_log(uint32_t __unused on)
{
    LOG_MSG_DEBUG("set dynamically config log not implement");
    return 0;
}

int silfp_get_engineering_info(uint32_t type)
{
    int ret = 0;
    switch (type) {
        case FINGERPRINT_GET_IMAGE_QUALITYS: {
            ret = silfp_ext_cb_image_quality_get();
            break;
        }
        case FINGERPRINT_SELF_TEST: {
            ret = silfp_ext_cb_spi_test();
            break;
        }
        default: {
            LOG_MSG_DEBUG("test %d not implement", type);
            break;
        }
    }
    return ret;
}
#endif

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
/**tp info struct for underglass fingerprint,add by pruce_tang_20180710 start **/

int silfp_get_tp_touch_info(uint8_t mode)
{
    int ret = -1;
    
    ret = sl_get_tp_touch_info(mode);

    return ret;
}

int silfp_touch_down()
{
    LOG_MSG_DEBUG("touch down");
    sl_fp_sync_finger_status_optic(1);
    return 0;
}

int silfp_touch_up()
{
    LOG_MSG_DEBUG("touch up");
    sl_fp_sync_finger_status_optic(0);
    return 0;
}

int silfp_notify_finger_ready()
{
    LOG_MSG_DEBUG("notify finger ready");
    m_finger_ready = 1;
    return SL_SUCCESS;
}

int silfp_clear_finger_ready_flag()
{
    LOG_MSG_DEBUG("clear finger ready flag");
    m_finger_ready = 0;
    return SL_SUCCESS;
}

inline int silfp_is_finger_ready()
{
    return m_finger_ready;
}

int silfp_wait_for_finger_ready()
{
    int iRetry = 200;
    while((iRetry > 0) && (!silfp_finger_is_canceled ())) {
        if(silfp_is_finger_ready()) {
            silfp_clear_finger_ready_flag();
            return SL_SUCCESS;
        }
        iRetry--;
        usleep(100 * 25); // 2.5ms
    }
    silfp_clear_finger_ready_flag();
    return -SL_ERROR_CANCELED;
}

void _send_fingerprint_cmd_notice(int32_t cmd_id, int8_t *result, uint32_t len)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    LOG_MSG_DEBUG("len = %d", len);

    msg.type = FINGERPRINT_OPTICAL_SENDCMD;
    msg.data.test.cmd_id = cmd_id;
    msg.data.test.result = result;
    msg.data.test.result_len = len;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

void silfp_send_fingerprint_cmd_notice(int32_t cmd_id, int32_t ret)
{
    uint32_t result_len = 8;
    int32_t result[2] = {0};

    result[0] = 100;
    result[1] = ret;

    _send_fingerprint_cmd_notice(cmd_id, (int8_t *)result, result_len);
}

void silfp_send_fingerprint_cmd_3v_notice(int32_t cmd_id, int32_t ret, int32_t v, int32_t v2, int32_t v3)
{
    uint32_t result_len = 32;
    int32_t result[8] = {0};

    result[0] = 100;
    result[1] = ret;
    result[2] = 101;
    result[3] = v;
    result[4] = 102;
    result[5] = v2;
    result[6] = 103;
    result[7] = v3;

    _send_fingerprint_cmd_notice(cmd_id, (int8_t *)result, result_len);
}

int silfp_send_fingerprint_cmd(int32_t cmd_id, int8_t __unused *in_buf, uint32_t __unused size)
{
    int32_t ret = 0;
    switch (cmd_id) {
        case FUN_CALIBRATE_START:
            LOG_MSG_DEBUG("test %d not implement", cmd_id);
            break;
        case FUN_CALIBRATE_CMD1:
        case FUN_CALIBRATE_CMD2:
        case FUN_CALIBRATE_CMD3:
        case FUN_CALIBRATE_CMD4:
        case FUN_CALIBRATE_CMD5:
        case FUN_CALIBRATE_CMD6:
        case FUN_CALIBRATE_CMD7:
        case FUN_CALIBRATE_CMD8:
        case FUN_CALIBRATE_CMD9:
        case FUN_CALIBRATE_CMD10:
            silfp_ext_cb_calibrate_step(cmd_id);
            break;
        case FUN_FINGERPRINT_TEST1:
            silfp_et_cb_optic_test_factory_quality();
            break;
        case FUN_FINGERPRINT_TEST2:
            LOG_MSG_DEBUG("test %d not implement", cmd_id);
            silfp_send_fingerprint_cmd_notice(cmd_id, 0);
            break;
        case FUN_FINGERPRINT_TEST_FINISH:
            LOG_MSG_DEBUG("test %d not implement", cmd_id);
            break;
        case FUN_AGING_TEST:
            silfp_ext_cb_image_capture_loop();
            break;
        case FUN_AGING_TEST_FINISH: {
            silfp_finger_cancel();
            break;
        }
        default: {
            LOG_MSG_DEBUG("default, test %d not implement", cmd_id);
            silfp_send_fingerprint_cmd_notice(cmd_id, 0);
            break;
        }
    }

    return ret;
}
#endif

