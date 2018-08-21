/******************************************************************************
 * @file   silead_impl.c
 * @brief  Contains CA implements.
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
 * John Zhang  2018/5/15   0.1.2      Support load/save config
 * Jack Zhang  2018/5/17   0.1.3      Change test process to simplify app use
 * Rich Li     2018/5/28   0.1.4      Add get enroll number command ID.
 * Davie Wang  2018/6/1    0.1.5      Add capture image sub command ID.
 * David Wang  2018/6/5    0.1.6      Support wakelock & pwdn
 * Rich Li     2018/6/7    0.1.7      Support dump image
 * Jack Zhang  2018/6/15   0.1.8      Add read OTP I/F.
 * Rich Li     2018/7/2    0.1.9      Add algo set param command ID.
 *
 *****************************************************************************/

#define FILE_TAG "silead_impl"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "silead_version.h"
#include "silead_impl.h"
#include "silead_dev.h"
#include "silead_fp.h"
#include "silead_error.h"
#include "silead_config.h"
#include "silead_config_dump.h"
#include "silead_xml.h"
#include "silead_storage.h"
#include "silead_device.h"
#include "silead_bmp.h"
#include "silead_stats.h"
#include "silead_util.h"
#include "silead_finger_internal.h"

#define FP_FEATURE_STORE_NORMAL_MASK 0x0001
#define FP_FEATURE_STORE_AUTH_UPDATE_MASK 0x0002
#define FP_FEATURE_NEED_REINIT_AFTER_IRQ_MASK 0x0004
#define FP_FEATURE_NEED_CALIBRATE_MASK 0x0008
#define FP_FEATURE_NEED_CALIBRATE2_WITH_NOFINGER_MASK 0x0010
#define FP_FEATURE_NEED_CALIBRATE2_MASK 0x0020
#define FP_FEATURE_NEED_FINGER_LOOP_MASK 0x0040
#define FP_FEATURE_NEED_IRQ_PWDN_MASK 0x0080
#define FP_FEATURE_NEED_SHUTDOWN_MASK 0x0100

#ifdef SIL_DUMP_IMAGE
#define TEST_DUMP_DATA_TYPE_IMG 0x51163731
#define TEST_DUMP_DATA_TYPE_NAV 0x51163732
#define TEST_DUMP_DATA_TYPE_RAW 0x51163733
#define TEST_DUMP_DATA_TYPE_FT  0x51163734
static const char dump_prefix[][16] = {
    "auth_succ",
    "auth_fail",
    "enroll_succ",
    "enroll_fail",
    "nav_succ",
    "nav_fail",
    "shot_succ",
    "shot_fail",
    "raw",
    "ft",
    "auth_orig",
    "enroll_orig",
};
#endif /* SIL_DUMP_IMAGE */

#define FP_USER_SEC_SERIAL_NAME "user_sec_serial"

#define FP_CONFIG_CHG_BIT   0x0001
#define FP_CONFIG_UP_BIT    0x0002
#define FP_CONFIG_DOWN_BIT  0x0004
#define FP_CONFIG_UPDATE_MASK (FP_CONFIG_CHG_BIT|FP_CONFIG_UP_BIT|FP_CONFIG_DOWN_BIT)
#define FP_CONFIG_NEED_UPDATE(x) (!!(FP_CONFIG_UPDATE_MASK == ((x)&FP_CONFIG_UPDATE_MASK)))
#ifndef FP_CONFIG_PATH
#define FP_CONFIG_PATH "/persist/silead/"
#endif /* !FP_CONFIG_PATH */
#define FP_CONFIG_NAME "fpconfig.dat"
#define FP_CONFIG_FILE (FP_CONFIG_PATH FP_CONFIG_NAME)
#define FP_OPTIC_CAL_NAME1 "fpcal1.dat"
#define FP_OPTIC_CAL_FILE1 (FP_CONFIG_PATH FP_OPTIC_CAL_NAME1)
#define FP_OPTIC_CAL_NAME2 "fpcal2.dat"
#define FP_OPTIC_CAL_FILE2 (FP_CONFIG_PATH FP_OPTIC_CAL_NAME2)
#define FP_OPTIC_CAL_NAME3 "fpcal3.dat"
#define FP_OPTIC_CAL_FILE3 (FP_CONFIG_PATH FP_OPTIC_CAL_NAME3)
#define FP_OPTIC_CAL_NAME4 "fpcal4.dat"
#define FP_OPTIC_CAL_FILE4 (FP_CONFIG_PATH FP_OPTIC_CAL_NAME4)
#define FP_OPTIC_CAL_NAME5 "fpcal5.dat"
#define FP_OPTIC_CAL_FILE5 (FP_CONFIG_PATH FP_OPTIC_CAL_NAME5)
#define FP_OPTIC_DEADPX_NAME "fpdeadpx.dat"
#define FP_OPTIC_DEADPX_FILE (FP_CONFIG_PATH FP_OPTIC_DEADPX_NAME)

/******add by pruce_tang_20180616 for interface match  --start*****/
#define FP_CONFIG_SCREEN_HBM_PATH "XXXXXXXXXX1"
#define FP_CONFIG_SCREEN_HBM_PATH2 "XXXXXXXXXX2"
#define FP_CONFIG_SCREEN_BRIGHTNESS_PATH "XXXXXXXXXX3"
#define FP_CONFIG_SCREEN_BRIGHTNESS_PATH2 "XXXXXXXXXX4"
/******add by pruce_tang_20180616 for interface match  --end*****/

#define BUF_SIZE   132*1024

static int32_t m_dev_fd = 0;
static const silead_fp_handle_t *m_fp_impl_handler = NULL;

static uint32_t m_storage_normal = 0;
static uint32_t m_tpl_max_size = 0;
static uint32_t m_tpl_update_support = 0;
static uint32_t m_need_reinit_after_irq = 0;
static uint32_t m_need_calibrate = 0;
static uint32_t m_need_calibrate2_with_nofinger = 0;
static uint32_t m_need_calibrate2 = 0;
static uint32_t m_need_finger_loop = 0;
static uint32_t m_config_update = 0;
static uint32_t m_need_irq_pwdn = 0;

static uint32_t m_finger_down = 0;

static uint32_t m_shutdown_by_avdd = 0;

static int32_t _sl_fp_save_config(void);
static int32_t _sl_fp_calibrate_step(uint32_t step, uint32_t init);

/**tp info struct for underglass fingerprint,add by pruce_tang_20180710 start **/
fingerprint_tp_info_t m_pre_tp_touch_info;
fingerprint_tp_info_t m_later_tp_touch_info;

void sl_fp_capture_pre()
{
}

#ifdef SIL_DUMP_IMAGE
static char* m_test_dump_buffer = NULL;
static uint32_t m_test_dump_buffer_size = 0;

static int32_t _fp_dump_init()
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    uint32_t size = 0;
    int32_t dump_data_support = 1;

#ifdef FP_DATA_DUMP_DYNAMIC
    dump_data_support = silfp_util_get_dump_level();
#endif /* FP_DATA_DUMP_DYNAMIC */

    if (dump_data_support > 0) {
        ret = 0;
        if (m_test_dump_buffer == NULL) {
            ret = -SL_ERROR_TA_OPEN_FAILED;
            if (m_fp_impl_handler != NULL) {
                if (m_fp_impl_handler->fp_test_get_image_info != NULL) {
                    ret = m_fp_impl_handler->fp_test_get_image_info(NULL, NULL, &size, NULL, NULL);
                } else {
                    LOG_MSG_DEBUG("No implement fp_test_get_image_info");
                }
            }

            if (size == 0) {
                LOG_MSG_DEBUG("get the config from ta is invalid");
                ret = -SL_ERROR_CONFIG_INVALID;
            }

            if (ret >= 0) {
                m_test_dump_buffer_size = size;
                m_test_dump_buffer = (char *)malloc(size);
                if (m_test_dump_buffer == NULL) {
                    m_test_dump_buffer_size = 0;
                    ret = -SL_ERROR_OUT_OF_MEMORY;
                }
            }
        }
        LOG_MSG_INFO("dump support, note: should not be enabled in release version");
    }

    return ret;
}

int32_t sl_fp_dump_data(e_mode_dump_img_t type)
{
    int32_t ret= -1;
    uint32_t w = 0;
    uint32_t h = 0;
    uint32_t imgsize = 0;
    uint32_t mode = TEST_DUMP_DATA_TYPE_IMG;

    ret = _fp_dump_init();
    if (ret < 0) {
        return ret;
    }

    if (m_test_dump_buffer == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    if (type == DUMP_IMG_NAV_SUCC || type == DUMP_IMG_NAV_FAIL) {
        mode = TEST_DUMP_DATA_TYPE_NAV;
    } else if (type == DUMP_IMG_RAW || type == DUMP_IMG_AUTH_ORIG || type == DUMP_IMG_ENROLL_ORIG) {
        mode = TEST_DUMP_DATA_TYPE_RAW;
    } else if (type == DUMP_IMG_FT) {
        mode = TEST_DUMP_DATA_TYPE_FT;
    }

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_dump_data != NULL) {
            ret = m_fp_impl_handler->fp_test_dump_data(mode, m_test_dump_buffer, m_test_dump_buffer_size, &imgsize, &w, &h);
        } else {
            LOG_MSG_DEBUG("No implement fp_test_dump_data");
        }
    }
    if (ret >= 0 || imgsize > 0) {
        silfp_bmp_save(m_test_dump_buffer, (type < DUMP_IMG_MAX) ? dump_prefix[type] : dump_prefix[DUMP_IMG_SHOT_FAIL], imgsize, w, h);
    }
    return ret;
}

static int32_t _fp_dump_deinit()
{
    if (m_test_dump_buffer != NULL) {
        free(m_test_dump_buffer);
    }
    m_test_dump_buffer = NULL;
    m_test_dump_buffer_size = 0;

    return 0;
}
#endif /* SIL_DUMP_IMAGE */

static int32_t _fp_update_all_log_level()
{
#ifdef SIL_DEBUG_ALL_LOG
    sl_fp_set_log_mode(2, 0, 2);
    silfp_stats_set_enabled(1); // open time_count
#else
    sl_fp_set_log_mode(0, 2, 0);
#endif

    return 0;
}

inline int32_t sl_fp_need_cancel_notice(void)
{
    return silfp_util_need_cancel_notice();
}

inline void sl_fp_cancel(void)
{
    silfp_dev_cancel();
}

void sl_fp_sync_finger_status_optic(int32_t down)
{
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
    if ((!m_finger_down && down) || (m_finger_down && !down)) {
        m_finger_down = down;
        silfp_dev_sync_finger_status_optic(down);
    }
#endif
}

int32_t sl_fp_get_screen_status(uint8_t *status)
{
    if (m_dev_fd <= 0) {
        LOG_MSG_DEBUG("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    return silfp_dev_get_screen_status(m_dev_fd, status);
}

inline int32_t sl_fp_set_screen_cb(screen_cb listen, void *param)
{
    return silfp_dev_set_screen_cb(listen, param);
}

static int32_t _fp_need_reinit_after_irq(void)
{
    return m_need_reinit_after_irq;
}

static inline int32_t _fp_wait_finger_status(int32_t fd, int32_t status, int32_t uncancelable)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    int16_t retry = 8;
    int32_t s_charge = 0; // get_sys_charging_state(); // Fix Me.

    LOG_MSG_VERBOSE("wait finger status %d", status);

    do {
        //sl_fp_download_normal();
        if (m_need_irq_pwdn) {
            sl_fp_chip_pwdn();
        } else {
        sl_fp_download_normal();
            if (m_fp_impl_handler != NULL) {
                if (m_fp_impl_handler->fp_wait_finger_status != NULL) {
                    silfp_dev_enable(m_dev_fd);
                    ret = m_fp_impl_handler->fp_wait_finger_status(status|((s_charge << 8)&0xFF00));
                    silfp_dev_disable(m_dev_fd);
                } else {
                    LOG_MSG_DEBUG("No implement fp_wait_finger_status");
                }
            }
            retry --;
            if ((ret == -SL_ERROR_INT_INVALID) && (retry > 0)) {
                /* Reset 300ms */
                silfp_dev_pwdn(fd, SIFP_PWDN_FLASH);
                usleep(1000*300);
                continue;
            }

            if (ret < 0) {
                return ret;
            }
        }

        if (silfp_finger_is_canceled() && !uncancelable) {
            ret = -SL_ERROR_CANCELED;
            break;
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
        } else if ((m_finger_down && status == IRQ_DOWN ) || (!m_finger_down && status == IRQ_UP)) {
            ret = 0;
#endif
        } else {
            ret = silfp_dev_wait_finger_status(fd, uncancelable);
        }

        if (_fp_need_reinit_after_irq() && (status == IRQ_DOWN || status == IRQ_NAV) && (ret != -SL_ERROR_CANCELED)) {
            sl_fp_download_normal();
        } else if (!_fp_need_reinit_after_irq() && (ret != -SL_ERROR_CANCELED) && (m_fp_impl_handler != NULL) && (m_fp_impl_handler->fp_chk_esd != NULL)) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_chk_esd();
            silfp_dev_disable(m_dev_fd);
            if ((ret == -SL_ERROR_DETECTED_ESD) && (retry > 0)) {
                /* Reset 300ms */
                silfp_dev_pwdn(fd, SIFP_PWDN_FLASH);
                usleep(1000*300);
                continue;
            }
        }
    } while ((ret == -SL_ERROR_DETECTED_ESD) && (retry > 0));

    if ((ret != -SL_ERROR_CANCELED) && (ret != -SL_ERROR_DETECTED_ESD)) {
        if (status == IRQ_DOWN || status == IRQ_NAV) {
            m_config_update |= FP_CONFIG_DOWN_BIT;
        } else if (status == IRQ_UP) {
            m_config_update |= FP_CONFIG_UP_BIT;
        }
        _sl_fp_save_config();
    }

    return ret;
}

int32_t sl_fp_wait_finger_down(void)
{
    int32_t ret = _fp_wait_finger_status(m_dev_fd, IRQ_DOWN, 0);

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
    if (ret >= 0) {
        silfp_send_touch_down_notice();
    }
#endif

    return ret;
}

int32_t sl_fp_wait_finger_up(void)
{
    int32_t ret = _fp_wait_finger_status(m_dev_fd, IRQ_UP, 0);

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
    silfp_send_touch_up_notice();
#endif

    return ret;
}

static int32_t sl_fp_wait_finger_up_uncancelable(void)
{
    int32_t ret = _fp_wait_finger_status(m_dev_fd, IRQ_UP, 1);

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
    silfp_send_touch_up_notice();
#endif

    return ret;
}

int32_t sl_fp_wait_finger_nav(void)
{
    return _fp_wait_finger_status(m_dev_fd, IRQ_NAV, 0);
}

int32_t sl_fp_capture_image(int32_t __unused times)
{
    int32_t ret;

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_capture_image != NULL) {
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
            if (!times) {
                sl_fp_capture_pre();
            }
#endif
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_capture_image();
            silfp_dev_disable(m_dev_fd);
        } else {
            LOG_MSG_DEBUG("No implement fp_capture_image");
        }
    }

#ifdef SIL_DUMP_IMAGE
    if (ret < 0) {
        sl_fp_dump_data(DUMP_IMG_SHOT_FAIL);
    }
#endif /* SIL_DUMP_IMAGE */

    return ret;
}

int32_t sl_fp_nav_capture_image(void)
{
    int32_t ret;

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_nav_capture_image != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_nav_capture_image();
            silfp_dev_disable(m_dev_fd);
        } else {
            LOG_MSG_DEBUG("No implement fp_nav_capture_image");
        }
    }

#ifdef SIL_DUMP_IMAGE
    sl_fp_dump_data((ret >=0) ? DUMP_IMG_NAV_SUCC : DUMP_IMG_NAV_FAIL);
#endif /* SIL_DUMP_IMAGE */

    return ret;
}

int32_t sl_fp_auth_start(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_auth_start != NULL) {
            ret = m_fp_impl_handler->fp_auth_start();
        } else {
            LOG_MSG_DEBUG("No implement fp_auth_start");
        }
    }

    return ret;
}

int32_t sl_fp_auth_step(uint64_t op_id, uint32_t step, uint32_t *fid)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_auth_step != NULL) {
            ret = m_fp_impl_handler->fp_auth_step(op_id, step, fid);
        } else {
            LOG_MSG_DEBUG("No implement fp_auth_step");
        }
    }

    return ret;
}

int32_t sl_fp_auth_normal_tpl_upd(void)
{
    int32_t ret = 0;
    void *buf = NULL;
    uint32_t len = m_tpl_max_size;
    uint32_t fid;

    if (m_storage_normal && m_tpl_update_support) {
        do {
            buf = malloc(len);
            if (buf == NULL) {
                ret = -SL_ERROR_OUT_OF_MEMORY;
                break;
            }

            ret = -SL_ERROR_TA_OPEN_FAILED;
            if (m_fp_impl_handler != NULL) {
                if (m_fp_impl_handler->fp_update_template != NULL) {
                    ret = m_fp_impl_handler->fp_update_template(buf, &len, &fid);
                } else {
                    LOG_MSG_DEBUG("No implement fp_update_template");
                }
            }
        } while (0);

        if (ret >= 0 && len > 0 && len <= m_tpl_max_size) {
            silfp_storage_update(fid, buf, len);
        }
        free(buf);
        buf = NULL;
    }
    return ret;
}

int32_t sl_fp_auth_end(void)
{
    int32_t ret;

    sl_fp_auth_normal_tpl_upd();

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_auth_end != NULL) {
            ret = m_fp_impl_handler->fp_auth_end();
        } else {
            LOG_MSG_DEBUG("No implement fp_auth_end");
        }
    }

    return ret;
}

int32_t sl_fp_enroll_start(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_enroll_start != NULL) {
            ret = m_fp_impl_handler->fp_enroll_start();
        } else {
            LOG_MSG_DEBUG("No implement fp_enroll_start");
        }
    }

    return ret;
}

int32_t sl_fp_enroll_step(uint32_t *remaining)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_enroll_step != NULL) {
            ret = m_fp_impl_handler->fp_enroll_step(remaining);
        } else {
            LOG_MSG_DEBUG("No implement fp_enroll_step");
        }
    }

    return ret;
}

int32_t sl_fp_enroll_normal_tpl_save(uint32_t *fid)
{
    int32_t ret = 0;
    void *buf = NULL;
    uint32_t len = m_tpl_max_size;

    if (m_storage_normal) {
        do {
            buf = malloc(len);
            if (buf == NULL) {
                ret = -SL_ERROR_OUT_OF_MEMORY;
                break;
            }

            ret = -SL_ERROR_TA_OPEN_FAILED;
            if (m_fp_impl_handler != NULL) {
                if (m_fp_impl_handler->fp_save_template != NULL) {
                    ret = m_fp_impl_handler->fp_save_template(buf, &len);
                } else {
                    LOG_MSG_DEBUG("No implement fp_save_template");
                }
            }
        } while (0);

        if (ret >= 0 && len > 0 && len <= m_tpl_max_size) {
            ret = silfp_storage_save(buf, len, fid);
        } else {
            ret = -SL_ERROR_STO_OP_FAILED;
        }

        if (buf != NULL) {
            free (buf);
        }
    }
    return ret;
}

int32_t sl_fp_enroll_end(uint32_t *fid)
{
    int32_t ret;
    int32_t status;

    ret = sl_fp_enroll_normal_tpl_save(fid);
    if (ret >= 0) {
        status = 0; // save ok or no normal storage
    } else {
        status = -1; // save failed
    }

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_enroll_end != NULL) { // clean enroll env
            ret = m_fp_impl_handler->fp_enroll_end(status, fid);
        } else {
            LOG_MSG_DEBUG("No implement fp_enroll_end");
        }
    }

    if (ret >= 0 && status < 0) {
        ret = -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

int32_t sl_fp_nav_support(uint32_t *type)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_nav_support != NULL) {
            ret = m_fp_impl_handler->fp_nav_support(type);
        } else {
            LOG_MSG_DEBUG("No implement fp_nav_support");
        }
    }

    return ret;
}

int32_t sl_fp_nav_start(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_nav_start != NULL) {
            if (m_config_update & FP_CONFIG_CHG_BIT) {
                sl_fp_download_normal();
            }
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_nav_start();
            silfp_dev_disable(m_dev_fd);
        } else {
            LOG_MSG_DEBUG("No implement fp_nav_start");
        }
    }

    return ret;
}

int32_t sl_fp_nav_step(uint32_t *key)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_nav_step != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_nav_step(key);
            silfp_dev_disable(m_dev_fd);
        } else {
            LOG_MSG_DEBUG("No implement fp_nav_step");
        }
    }

#ifdef SIL_DUMP_IMAGE
    sl_fp_dump_data((ret >=0) ? DUMP_IMG_NAV_SUCC : DUMP_IMG_NAV_FAIL);
#endif /* SIL_DUMP_IMAGE */

    return ret;
}

int32_t sl_fp_nav_end(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_nav_end != NULL) {
            ret = m_fp_impl_handler->fp_nav_end();
        } else {
            LOG_MSG_DEBUG("No implement fp_nav_end");
        }
    }

    return ret;
}

int32_t sl_fp_send_key(uint32_t key)
{
    if (m_dev_fd <= 0) {
        LOG_MSG_DEBUG("FP Device Open Failed");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    return silfp_dev_send_key(m_dev_fd, key);
}

static int32_t _fp_init2()
{
    int32_t ret;
    uint32_t feature = 0;
    void *device_info = NULL;
    uint32_t len = 0;
    uint32_t algoVer, taVer;

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_init2 != NULL) {
            ret = silfp_device_info_get(&device_info);
            if (ret > 0) {
                len = ret;
            }
            ret = m_fp_impl_handler->fp_init2(device_info, len, &feature, &m_tpl_max_size);
            silfp_device_info_release(device_info);
        } else {
            LOG_MSG_DEBUG("No implement fp_init2");
        }
    }

    if (ret >= 0) {
        m_storage_normal = !!(feature & FP_FEATURE_STORE_NORMAL_MASK);
        m_tpl_update_support = !!(feature & FP_FEATURE_STORE_AUTH_UPDATE_MASK);
        m_need_reinit_after_irq = !!(feature & FP_FEATURE_NEED_REINIT_AFTER_IRQ_MASK);
        m_need_calibrate = !!(feature & FP_FEATURE_NEED_CALIBRATE_MASK);
        m_need_calibrate2_with_nofinger = !!(feature & FP_FEATURE_NEED_CALIBRATE2_WITH_NOFINGER_MASK);
        m_need_calibrate2 = !!(feature & FP_FEATURE_NEED_CALIBRATE2_MASK);
        m_need_finger_loop = !!(feature & FP_FEATURE_NEED_FINGER_LOOP_MASK);
        m_need_irq_pwdn = !!(feature & FP_FEATURE_NEED_IRQ_PWDN_MASK);
        m_shutdown_by_avdd = !!(feature & FP_FEATURE_NEED_SHUTDOWN_MASK);
    }

    // maybe some ta miss the value, should not happend, just in case
    if (m_storage_normal && m_tpl_max_size == 0) {
        m_tpl_max_size = (500 * 1024);
    }

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_version != NULL) {
            if (m_fp_impl_handler->fp_get_version(&algoVer, &taVer) >= 0) {
                LOG_MSG_INFO("ta version:v%d, algo version: v%d", taVer, algoVer);
            }
        } else {
            LOG_MSG_DEBUG("No implement fp_get_version");
        }
    }
    LOG_MSG_VERBOSE("m_storage_normal=%d, m_tpl_max_size=%d, m_need_reinit_after_irq=%d", m_storage_normal, m_tpl_max_size, m_need_reinit_after_irq);
    LOG_MSG_VERBOSE("calibrate=%d, calibrate2_nofinger=%d, calibrate2=%d", m_need_calibrate, m_need_calibrate2_with_nofinger, m_need_calibrate2);
    LOG_MSG_VERBOSE("finger loop=%d, avdd=%d", m_need_finger_loop, m_shutdown_by_avdd);

    return ret;
}

static int32_t _sl_fp_save_config(void)
{
    char *buf = NULL;
    uint32_t len;
    int32_t ret = SL_SUCCESS;

    if (!FP_CONFIG_NEED_UPDATE(m_config_update)) {
        return ret;
    } else {
        m_config_update = 0;
    }

    if (m_need_calibrate || m_need_calibrate2) {
        len = 128*128;
        buf = malloc(len);
        if (!buf) {
            LOG_MSG_ERROR("Malloc fail!!!");
            return -SL_ERROR_OUT_OF_MEMORY;
        }

        if (m_fp_impl_handler->fp_get_config != NULL) {
            ret = m_fp_impl_handler->fp_get_config(buf, &len);
        } else {
            LOG_MSG_DEBUG("No implement fp_get_config");
        }

        if (ret >= 0 && len > 4) {
            ret = silfp_storage_save_config(FP_CONFIG_PATH, FP_CONFIG_NAME, buf, len);
        }
        LOG_MSG_DEBUG("save config, ret=%d",ret);
        free(buf);
    }
    return ret;
}

int32_t sl_fp_calibrate(void)
{
    int32_t ret;
#ifndef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
    char *buf = NULL;
    int32_t len = 0;

    if ( m_need_calibrate ) {
        len = silfp_storage_get_file_size(FP_CONFIG_FILE);
        LOG_MSG_DEBUG("get %s size %d",FP_CONFIG_FILE, len);
        if ( len < 10 ) {
            len = 4;
        }
        buf = malloc(len);
        if (!buf) {
            LOG_MSG_ERROR("Malloc fail!!!");
            return -SL_ERROR_OUT_OF_MEMORY;
        }
        memset(buf,0,len);

        if ( len > 4 ) {
            ret = silfp_storage_load_config(FP_CONFIG_PATH, FP_CONFIG_NAME, buf, len);
            if ( ret < 0 ) {
                len = 4;
            }
        }
    }

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_need_calibrate) {
            if (m_fp_impl_handler->fp_calibrate != NULL) {
                silfp_dev_enable(m_dev_fd);
                ret = m_fp_impl_handler->fp_calibrate(buf, len);
                silfp_dev_disable(m_dev_fd);
            } else {
                LOG_MSG_DEBUG("No implement fp_calibrate");
            }
        }

        if (ret == -SL_ERROR_CONFIG_INVALID) {
            m_config_update = FP_CONFIG_CHG_BIT;
            if (m_need_calibrate2_with_nofinger) {
                ret = sl_fp_wait_finger_up();
            } else if (m_need_calibrate2) {
                ret = sl_fp_download_normal();
            }
        }

        if (m_need_calibrate2) {
            if (m_fp_impl_handler->fp_calibrate2 != NULL) {
                silfp_dev_enable(m_dev_fd);
                ret = m_fp_impl_handler->fp_calibrate2();
                silfp_dev_disable(m_dev_fd);
            } else {
                LOG_MSG_DEBUG("No implement fp_calibrate2");
            }
        }

        if (m_need_calibrate || m_need_calibrate2) {
            sl_fp_download_normal();
        }
    }

    if (buf) {
        free(buf);
    }
#else
    ret = _sl_fp_calibrate_step(1, 1);
    ret |= _sl_fp_calibrate_step(2, 1);
    ret |= _sl_fp_calibrate_step(3, 1);
    ret |= _sl_fp_calibrate_step(4, 1);
    ret |= _sl_fp_calibrate_step(5, 1);
#endif /* SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC */

    return ret;
}

static int32_t _fp_update_cfg(const uint32_t chipid, const uint32_t subid, const uint32_t virtualid)
{
    int32_t ret = 0;
    int32_t len = 0;
    void *buffer = NULL;

    cf_set_t *pconfig = NULL;
    const char *name = NULL;

    pconfig = silfp_cfg_malloc();
    if (pconfig != NULL) {
        pconfig->common.id = chipid;
        pconfig->common.sid = subid;
        pconfig->common.vid = virtualid;

        do {
            ret = silfp_xml_get_sysparams(pconfig);
            if (ret < 0) {
                ret = 0;
                break;
            }
            //silfp_cfg_dump_data(pconfig, NULL);

            len = silfp_cfg_get_update_length(pconfig);
            if (len <= 0) {
                ret = 0;
                break;
            }

            buffer = malloc(len);
            if (buffer == NULL) {
                LOG_MSG_ERROR("allocation failed");
                ret = -SL_ERROR_OUT_OF_MEMORY;
                break;
            }

            if(len != silfp_cfg_get_update_buffer(buffer, len, pconfig)) {
                ret= -SL_ERROR_BAD_PARAMS;
                break;
            }

            ret = -SL_ERROR_TA_OPEN_FAILED;
            if (m_fp_impl_handler != NULL) {
                if (m_fp_impl_handler->fp_update_cfg != NULL) {
                    ret = m_fp_impl_handler->fp_update_cfg(buffer, len);
                } else {
                    LOG_MSG_DEBUG("No implement fp_update_cfg");
                }
            }
        } while(0);

        if (buffer != NULL) {
            free(buffer);
            buffer = NULL;
        }
        silfp_cfg_free(pconfig);
        pconfig = NULL;
    }

    return ret;
}

static int32_t _fp_init()
{
    int32_t ret;
    FP_DEV_CONF dev_init;
    uint32_t chipid = 0;
    uint32_t subid = 0;
    uint32_t vid = 0;
    uint32_t update_cfg = 0;
    char name[32];

    if (m_dev_fd <= 0) {
        m_dev_fd = open(FP_SENSOR_DEVICE, O_RDWR);
    }
    if (m_dev_fd <= 0) {
        LOG_MSG_ERROR("FP Device Open Failed (%d:%s)", errno, strerror(errno));
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    memset(&dev_init, 0, sizeof(dev_init));
    ret = silfp_dev_init(m_dev_fd, &dev_init);
    if (ret < 0) {
        LOG_MSG_ERROR("init dev fail");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    if (silfp_dev_hw_reset(m_dev_fd, 0) < 0) {
        LOG_MSG_ERROR("reset fingerprint chip fail");
    }

    if (m_fp_impl_handler == NULL) {
        m_fp_impl_handler = silfp_get_impl_handler(dev_init.ta[0]?dev_init.ta:NULL);
    }

    _fp_update_all_log_level();
    silfp_dev_enable(m_dev_fd);

    do {
        ret = -SL_ERROR_TA_OPEN_FAILED;
        if (m_fp_impl_handler != NULL) {
            if (m_fp_impl_handler->fp_init != NULL) {
                ret = m_fp_impl_handler->fp_init(&dev_init, &chipid, &subid, &vid, &update_cfg);
                LOG_MSG_VERBOSE("chipid=%x,%x,%x %d (%d)", chipid, subid, vid, update_cfg, ret);
            } else {
                LOG_MSG_DEBUG("No implement fp_init");
            }
        }

        if (ret < 0) {
            break;
        }

        if (update_cfg) {
            _fp_update_cfg(chipid, subid, vid);
        }

        ret = _fp_init2();
    } while (0);

    silfp_dev_disable(m_dev_fd);

    if (ret >= 0) {
        snprintf(name, sizeof(name),"gsl%04x\n",(chipid >> 16));
        sl_fp_create_proc_node(name);
    }

    return ret;
}

int32_t sl_fp_download_normal()
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (m_dev_fd <= 0) {
        LOG_MSG_ERROR("FP Device Open Failed");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    if (silfp_dev_hw_reset(m_dev_fd, 0) < 0) {
        LOG_MSG_ERROR("reset fingerprint chip fail");
    }

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_download_normal != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_download_normal();
            silfp_dev_disable(m_dev_fd);
        } else {
            LOG_MSG_DEBUG("No implement fp_download_normal");
        }
    }
    return ret;
}

int32_t sl_fp_init()
{
    int32_t ret;
    int32_t i;
    int32_t count = 2; // try 5 times

    m_dev_fd = -1;

#ifdef GIT_SHA1_ID
    LOG_MSG_INFO("fp hal version: %s, tag: %s", FP_HAL_VERSION, GIT_SHA1_ID);
#else
    LOG_MSG_INFO("fp hal version: %s", FP_HAL_VERSION);
#endif

    slifp_device_print_version();

    for (i = 0; i < count; i++) {
        ret = _fp_init();
        if (ret >= 0) {
            break;
        }
    }

    return ret;
}

int32_t sl_fp_close(void)
{
    silfp_dev_deinit(m_dev_fd);

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_deinit != NULL) {
            m_fp_impl_handler->fp_deinit();
        } else {
            LOG_MSG_DEBUG("No implement fp_deinit");
        }
    }
    m_fp_impl_handler = NULL;

    if (m_storage_normal) {
        silfp_storage_release();
    }

    if(m_dev_fd > 0) {
        close(m_dev_fd);
        m_dev_fd = 0;
    }

#ifdef SIL_DUMP_IMAGE
    _fp_dump_deinit();
#endif /* SIL_DUMP_IMAGE */

    LOG_MSG_VERBOSE("close");
    return SL_SUCCESS;
}

int32_t sl_fp_set_gid(uint32_t gid)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    uint32_t serial = 0xFDCA;//silfp_device_get_user_key(gid, FP_USER_SEC_SERIAL_NAME);
    LOG_MSG_DEBUG("serial = 0x%x", serial);

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_set_gid != NULL) {
            ret = m_fp_impl_handler->fp_set_gid(gid, serial);
        } else {
            LOG_MSG_DEBUG("No implement fp_set_gid");
        }
    }

    return ret;
}

int32_t sl_fp_load_user_db(const char* db_path)
{
    int32_t ret;
    void *buf = NULL;
    uint32_t buf_len = m_tpl_max_size;
    uint32_t len;

    if (m_storage_normal) {
        buf = malloc(buf_len);
        if (buf == NULL) {
            return -SL_ERROR_OUT_OF_MEMORY;
        }
        silfp_storage_set_tpl_path(db_path);
    }

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_load_user_db != NULL) {
            ret = m_fp_impl_handler->fp_load_user_db(db_path);
        } else {
            LOG_MSG_DEBUG("No implement fp_load_user_db");
        }
    }

    if (m_storage_normal) {
        uint32_t id[TPL_MAX_ST];
        int32_t num, i, cnt = 0;

        num = silfp_storage_get_idlist(id, 1);
        for (i = 0; i < num; i ++) {
            memset(buf, 0, buf_len);
            ret = silfp_storage_load(id[i], buf, buf_len);
            if (ret >= 0) {
                len = ret;
                if (len <= buf_len) {
                    ret = -SL_ERROR_TA_OPEN_FAILED;
                    if (m_fp_impl_handler != NULL) {
                        if (m_fp_impl_handler->fp_load_template != NULL) {
                            ret = m_fp_impl_handler->fp_load_template(id[i], buf, len);
                        } else {
                            LOG_MSG_DEBUG("No implement fp_load_template");
                        }
                    }
                } else {
                    ret = -SL_ERROR_TEMPLATE_INVALID;
                }

                if (ret == -SL_ERROR_TEMPLATE_INVALID) {
                    silfp_storage_inc_fail_count(id[i]);
                } else if (ret >= 0) {
                    cnt++;
                }
            }
        }

        ret = cnt;
        free(buf);
    }

    return ret;
}

int32_t sl_fp_remove_finger(uint32_t fid)
{
    int32_t ret;

    if (m_storage_normal) {
        ret = silfp_storage_remove(fid);
        if (ret < 0) {
            return ret;
        }
    }

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_remove_finger != NULL) {
            ret = m_fp_impl_handler->fp_remove_finger(fid);
        } else {
            LOG_MSG_DEBUG("No implement fp_remove_finger");
        }
    }

    return ret;
}

int32_t sl_fp_get_db_count(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_db_count != NULL) {
            ret = m_fp_impl_handler->fp_get_db_count();
        } else {
            LOG_MSG_DEBUG("No implement fp_get_db_count");
        }
    }

    return ret;
}

int32_t sl_fp_get_finger_prints(uint32_t *ids, uint32_t count)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_finger_prints != NULL) {
            ret = m_fp_impl_handler->fp_get_finger_prints(ids, count);
        } else {
            LOG_MSG_DEBUG("No implement fp_get_finger_prints");
        }
    }

    return ret;
}

int64_t sl_fp_load_enroll_challenge(void)
{
    int64_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_load_enroll_challenge != NULL) {
            ret = m_fp_impl_handler->fp_load_enroll_challenge();
        } else {
            LOG_MSG_DEBUG("No implement fp_load_enroll_challenge");
        }
    }

    return ret;
}

int32_t sl_fp_set_enroll_challenge(uint64_t challenge)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_set_enroll_challenge != NULL) {
            ret = m_fp_impl_handler->fp_set_enroll_challenge(challenge);
        } else {
            LOG_MSG_DEBUG("No implement fp_set_enroll_challenge");
        }
    }

    return ret;
}

int32_t sl_fp_verify_enroll_challenge(const void* hat, uint32_t size)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_verify_enroll_challenge != NULL) {
            ret = m_fp_impl_handler->fp_verify_enroll_challenge(hat, size);
        } else {
            LOG_MSG_DEBUG("No implement fp_verify_enroll_challenge");
        }
    }

    return ret;
}

int64_t sl_fp_load_auth_id(void)
{
    int64_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_load_auth_id != NULL) {
            ret = m_fp_impl_handler->fp_load_auth_id();
        } else {
            LOG_MSG_DEBUG("No implement fp_load_auth_id");
        }
    }

    return ret;
}

int32_t sl_fp_get_hw_auth_obj(void * buffer, uint32_t length)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_hw_auth_obj != NULL) {
            ret = m_fp_impl_handler->fp_get_hw_auth_obj(buffer, length);
        } else {
            LOG_MSG_DEBUG("No implement fp_get_hw_auth_obj");
        }
    }

    return ret;
}

int32_t sl_fp_set_log_mode(uint8_t krm, uint8_t tam, uint8_t agm)
{
    int32_t ret;

    if (m_dev_fd <= 0) {
        LOG_MSG_DEBUG("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    LOG_MSG_VERBOSE("krm = %d, tam = %d, agm = %d", krm, tam, agm);

    silfp_dev_set_log_level(m_dev_fd, &krm);

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_set_log_mode != NULL) {
            ret = m_fp_impl_handler->fp_set_log_mode(tam, agm);
        } else {
            LOG_MSG_DEBUG("No implement fp_set_log_mode");
        }
    }

    return ret;
}

int32_t sl_fp_set_nav_type(uint32_t mode)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_set_nav_type != NULL) {
            ret = m_fp_impl_handler->fp_set_nav_type(mode);
        } else {
            LOG_MSG_DEBUG("No implement fp_set_nav_type");
        }
    }

    return ret;
}

static int32_t _fp_get_finger_status(uint32_t *status)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    uint32_t addition = 0;

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_finger_status != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_get_finger_status(status, &addition);
            silfp_dev_disable(m_dev_fd);
        } else {
            LOG_MSG_DEBUG("No implement fp_get_finger_status");
        }
        if (ret == -SL_ERROR_DETECTED_ESD) {
            /* Reset 300ms */
            silfp_dev_pwdn(m_dev_fd, SIFP_PWDN_FLASH);
            usleep(1000*300);
            sl_fp_download_normal();
        }
    }

    if (ret >= 0) {
        if (status) {
            m_config_update |= (*status)?FP_CONFIG_DOWN_BIT:FP_CONFIG_UP_BIT;
        }
        if (addition & 0x40000000) { // update finish
            m_need_finger_loop = 0;
        }
        if (addition & 0x80000000) { // need save
            LOG_MSG_DEBUG("addition = 0x%x", addition);
            m_config_update = FP_CONFIG_CHG_BIT;
        }
        _sl_fp_save_config();
    }

    return ret;
}

int32_t sl_fp_get_finger_down(void)
{
    int32_t ret = -1;
    uint32_t status = 0;

    if (m_need_finger_loop) {
        sl_fp_download_normal();

        while (!silfp_finger_is_canceled()) {
            ret = _fp_get_finger_status(&status);
            if (ret < 0) {
                break;
            }

            if (status > 0) {
                break;
            } else {
                ret = -SL_ERROR_CANCELED;
            }
        }
    } else {
        ret = sl_fp_wait_finger_down();
    }

    return ret;
}

int32_t sl_fp_get_finger_up(void)
{
    int32_t ret = -1;
    uint32_t status = 0;

    if (m_need_finger_loop) {
        sl_fp_download_normal();

        while (!silfp_finger_is_canceled()) {
            ret = _fp_get_finger_status(&status);
            if (ret < 0) {
                break;
            }

            if (status == 0) {
                break;
            } else {
                ret = -SL_ERROR_CANCELED;
            }
        }
    } else {
        ret = sl_fp_wait_finger_up();
    }

    return ret;
}

int32_t sl_fp_get_finger_up_uncancelable(void)
{
    int32_t ret;
    uint32_t status;

    if (m_need_finger_loop) {
        sl_fp_download_normal();

        while (1) {
            ret = _fp_get_finger_status(&status);
            if (ret < 0) {
                continue;
            }

            if (status == 0) {
                break;
            }
        }
    } else {
        ret = sl_fp_wait_finger_up_uncancelable();
    }

    return ret;
}

int32_t sl_fp_get_enroll_num(uint32_t *num)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_enroll_num != NULL) {
            ret = m_fp_impl_handler->fp_get_enroll_num(num);
        } else {
            LOG_MSG_DEBUG("No implement fp_get_enroll_num");
        }
    }

    return ret;
}

inline int32_t sl_fp_get_dev_ver(char *version, uint32_t len)
{
    int32_t ret = -1;

    if (m_dev_fd > 0) {
        ret = silfp_dev_get_ver(m_dev_fd, version, len);
    }

    return ret;
}

int32_t sl_fp_get_ta_ver(uint32_t *algoVer, uint32_t *taVer)
{
    int32_t ret;

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_version != NULL) {
            ret = m_fp_impl_handler->fp_get_version(algoVer, taVer);
        } else {
            LOG_MSG_DEBUG("No implement fp_get_version");
        }
    }

    return ret;
}

int32_t sl_fp_get_chipid(uint32_t *chipId, uint32_t *subId)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    sl_fp_download_normal();

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_chip_id != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_get_chip_id(chipId, subId);
            silfp_dev_disable(m_dev_fd);
        } else {
            LOG_MSG_DEBUG("No implement fp_get_chip_id");
        }
    }

    return ret;
}

int32_t sl_fp_ci_chk_finger(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_ci_chk_finger != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_ci_chk_finger();
            silfp_dev_disable(m_dev_fd);
        } else {
            LOG_MSG_DEBUG("No implement fp_ci_chk_finger");
        }
    }

    return ret;
}

int32_t sl_fp_ci_adj_gain(int32_t enroll)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_ci_adj_gain != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_ci_adj_gain(enroll);
            silfp_dev_disable(m_dev_fd);
        } else {
            LOG_MSG_DEBUG("No implement fp_ci_adj_gain");
        }
    }

    return ret;
}

int32_t sl_fp_ci_shot(int32_t enroll)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_ci_shot != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_ci_shot(enroll);
            silfp_dev_disable(m_dev_fd);
        } else {
            LOG_MSG_DEBUG("No implement fp_ci_shot");
        }
    }

    return ret;
}

int32_t sl_fp_alg_set_param(uint32_t idx, void *buffer, uint32_t *plen, uint32_t *result)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_alg_set_param != NULL) {
            if (idx & 0xFF00) {
                silfp_dev_enable(m_dev_fd);
            }
            ret = m_fp_impl_handler->fp_alg_set_param(idx, buffer, plen, result);
            if (idx & 0xFF00) {
                silfp_dev_disable(m_dev_fd);
            }
        } else {
            LOG_MSG_DEBUG("No implement fp_alg_set_param");
        }
    }

    return ret;
}

static char *_sl_fp_get_cal_file(uint32_t step)
{
    switch(step&0xF) {
    case 1:
        return FP_OPTIC_CAL_FILE1;
    case 2:
        return FP_OPTIC_CAL_FILE2;
    case 3:
        return FP_OPTIC_CAL_FILE3;
    case 4:
        return FP_OPTIC_CAL_FILE4;
    case 5:
        return FP_OPTIC_CAL_FILE5;
    default:
        return NULL;
    }
}
static char *_sl_fp_get_cal_name(uint32_t step)
{
    switch(step&0xF) {
    case 1:
        return FP_OPTIC_CAL_NAME1;
    case 2:
        return FP_OPTIC_CAL_NAME2;
    case 3:
        return FP_OPTIC_CAL_NAME3;
    case 4:
        return FP_OPTIC_CAL_NAME4;
    case 5:
        return FP_OPTIC_CAL_NAME5;
    default:
        return NULL;
    }
}
static int32_t _sl_fp_calibrate_step(uint32_t step, uint32_t init)
{
    int32_t  ret = -SL_ERROR_TA_OPEN_FAILED;
    uint32_t step_ta = step;
    char     *buf = NULL;
    int32_t  len = BUF_SIZE;

    if ((step >= 1) && (step <= 5)) {
        if (init) {
            len = silfp_storage_get_file_size(_sl_fp_get_cal_file(step));
            LOG_MSG_DEBUG("get %s size %d",_sl_fp_get_cal_file(step), len);

            if ( len < 10 ) {
                len = BUF_SIZE;
                step_ta |= 0x20; // Indicate date is invalid.
            } else {
                len += 64; // Allocate for head structure.
            }
        }
        buf = malloc(len);
        if (!buf) {
            LOG_MSG_ERROR("Malloc fail!!!");
            return -SL_ERROR_OUT_OF_MEMORY;
        }
        memset(buf,0,len);

        if ( init && (len != BUF_SIZE) ) {
            ret = silfp_storage_load_config(FP_CONFIG_PATH, _sl_fp_get_cal_name(step), buf, len-64);
            if ( ret == len-64 ) {
                step_ta |= 0x10; // Indicate date is valid.
            } else {
                step_ta |= 0x20; // Indicate date is invalid.
            }
        }
    }

    if ((step >= 1) && (step <= 3)) {
        sl_fp_download_normal();
    }
    LOG_MSG_DEBUG("fp_calibrate_optic step %d", step_ta);

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_calibrate_optic != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_calibrate_optic(step_ta, (uint8_t *)buf, (uint32_t *)&len);
            silfp_dev_disable(m_dev_fd);
            if ((((ret >= 0) && !(step_ta&0xF0)) || ((ret > 10) && (step_ta&0xF0))) && ((step >= 1 && step <= 5))) {
                silfp_storage_save_config(FP_CONFIG_PATH, _sl_fp_get_cal_name(step), buf, len);
                if ((step >= 1) && (step <= 3)) {
                    silfp_storage_remove_file(_sl_fp_get_cal_file(4));
                    silfp_storage_remove_file(_sl_fp_get_cal_file(5));
                }
            }
        } else {
            LOG_MSG_DEBUG("No implement fp_calibrate_optic");
        }
    }

    if (buf) {
        free(buf);
        buf = NULL;
    }

#ifdef SIL_DUMP_IMAGE
    if ((ret >= 0) &&/* !(step_ta&0xF0) && */((step >= 1) && (step <= 3))) {
        sl_fp_dump_data(DUMP_IMG_RAW);
    }
#endif /* SIL_DUMP_IMAGE */

    return ret;
}

static int32_t _sl_fp_test_sram(uint32_t *result, uint32_t *deadpx, uint32_t *deadpx_center, uint8_t init)
{
    int32_t  ret = -SL_ERROR_TA_OPEN_FAILED;
    uint32_t cmd_ta = 0;
    char     *buf = NULL;
    int32_t  len = BUF_SIZE;
    uint32_t num = 0;

    if (!result || !deadpx || !deadpx_center) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if (init) {
        len = silfp_storage_get_file_size(FP_OPTIC_DEADPX_FILE);
        LOG_MSG_DEBUG("get %s size %d",FP_OPTIC_DEADPX_FILE, len);

        if ( len < 10 ) {
            len = BUF_SIZE;
        }
    }
    buf = malloc(len);
    if (!buf) {
        LOG_MSG_ERROR("Malloc fail!!!");
        return -SL_ERROR_OUT_OF_MEMORY;
    }
    memset(buf,0,len);

    if ( init && (len != BUF_SIZE)) {
        ret = silfp_storage_load_config(FP_CONFIG_PATH, FP_OPTIC_DEADPX_NAME, buf, len);
    }

    sl_fp_download_normal();

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_cmd != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_test_cmd(cmd_ta, (uint8_t *)buf, (uint32_t *)&len, &num);
            silfp_dev_disable(m_dev_fd);
            if (ret >= 0) {
                silfp_storage_save_config(FP_CONFIG_PATH, FP_OPTIC_DEADPX_NAME, buf, len);
                *deadpx = (num>>16) & 0xFFFF;
                *deadpx_center = num & 0xFFFF;
                *result = ret;
            }
        } else {
            LOG_MSG_DEBUG("No implement fp_test_cmd");
        }
    }

    if (buf) {
        free(buf);
        buf = NULL;
    }

    return ret;
}

int32_t sl_fp_calibrate_step(uint32_t step)
{
    return _sl_fp_calibrate_step(step, 0);
}

int32_t sl_fp_optic_test_snr(uint32_t *snr, uint32_t *noise, uint32_t *signal)
{
    int32_t ret = -1;
    uint32_t cmd_ta = 1;
    uint32_t snr_result[4] = {0};
    uint32_t len = sizeof(snr_result);
    uint32_t result = 0;

#ifdef SIL_DUMP_IMAGE
    int32_t ret2 = 0;
    uint32_t dump_len = 0;
    uint8_t *dump_buf = NULL;
    uint32_t dump_result = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t dump_remaining = 0;
    uint8_t step = 0;
#endif

    uint32_t snr_finish_result[4] = {0};
    uint32_t snr_finish_len = sizeof(snr_finish_result);
    uint32_t finish_result = 0;

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_cmd != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_test_cmd(cmd_ta, (uint8_t *)snr_result, &len, &result);
            silfp_dev_disable(m_dev_fd);
            LOG_MSG_DEBUG("ret: %d, snr:%d, noise:%d,  signal:%d", ret, snr_result[0], snr_result[1], snr_result[2]);

#ifdef SIL_DUMP_IMAGE
            cmd_ta = 0xC0000001;
            dump_buf = malloc(BUF_SIZE);
            if (dump_buf != NULL) {
                do {
                    dump_len = BUF_SIZE;
                    dump_buf[0] = (step & 0xFF);
                    step++;
                    ret2 = m_fp_impl_handler->fp_test_cmd(cmd_ta, (uint8_t *)dump_buf, &dump_len, &dump_result);
                    if (ret2 >= 0) {
                        width = (dump_result & 0x0000FFFF);
                        height = (dump_result >> 16 & 0x0000FFFF);
                        LOG_MSG_DEBUG("width = %d, height = %d, size = %d", width, height, dump_len);
                        if (dump_len >= width * height + 1) {
                            dump_remaining = dump_buf[0];
                            LOG_MSG_DEBUG("dump_remaining = %d", dump_remaining);
                            silfp_bmp_save(dump_buf + 1, "snr", width * height, width, height);
                        }
                    } else {
                        LOG_MSG_ERROR("dump error (%d)", ret2);
                    }
                } while (ret2 >= 0 && dump_remaining > 0);
                free(dump_buf);
            } else {
                LOG_MSG_ERROR("malloc dump buf failed");
            }
#endif

            cmd_ta = 0x80000001;
            m_fp_impl_handler->fp_test_cmd(cmd_ta, (uint8_t *)snr_finish_result, &snr_finish_len, &finish_result);
        } else {
            LOG_MSG_DEBUG("No implement fp_test_cmd");
        }
    }

    if (snr) {
        *snr = snr_result[0];
    }
    if (noise) {
        *noise = snr_result[1];
    }
    if (signal) {
        *signal = snr_result[2];
    }

    return ret;
}

int32_t sl_fp_optic_test_factory_quality(uint32_t *result, uint32_t *quality, uint32_t*length)
{
    int32_t ret = -1;
    uint32_t cmd_ta = 2;
    uint32_t qa_result[3] = {0};
    uint32_t len = sizeof(qa_result);
    uint32_t qa_ret = 0;

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_cmd != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_test_cmd(cmd_ta, (uint8_t *)qa_result, &len, &qa_ret);
            silfp_dev_disable(m_dev_fd);
            LOG_MSG_DEBUG("ret: %d, result: %u, quality:%u, length:%u", ret, qa_ret, qa_result[0], qa_result[1]);
        } else {
            LOG_MSG_DEBUG("No implement fp_test_cmd");
        }
    }

#ifdef SIL_DUMP_IMAGE
    sl_fp_dump_data(DUMP_IMG_FT);
#endif

    if (result) {
        *result = qa_ret;
    }
    if (quality) {
        *quality = qa_result[0];
    }
    if (length) {
        *length = qa_result[1];
    }

    return ret;
}

int32_t sl_fp_reset_test()
{
    int32_t ret;

    ret = sl_fp_download_normal();

    return ret;
}

int32_t sl_fp_get_otp(void)
{
    uint32_t otp[3] = {0};
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_otp != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_get_otp(&otp[0], &otp[1], &otp[2]);
            silfp_dev_disable(m_dev_fd);
        } else {
            LOG_MSG_DEBUG("No implement fp_get_otp");
        }
    }

    if (ret >= 0) {
        LOG_MSG_INFO("OTP 0x%08X %08X %08X", otp[0],otp[1],otp[2]);
    }

    return ret;
}

int32_t sl_fp_test_get_image_info(uint32_t *w, uint32_t *h, uint32_t *size, uint32_t *w_ori, uint32_t *h_ori)
{
    int32_t ret;

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_get_image_info != NULL) {
            ret = m_fp_impl_handler->fp_test_get_image_info(w, h, size, w_ori, h_ori);
        } else {
            LOG_MSG_DEBUG("No implement fp_test_get_image_info");
        }
    }

    return ret;
}

int32_t sl_fp_test_image_capture(uint32_t gentpl, void *buffer, uint32_t *len, uint8_t *quality, uint8_t *area, uint8_t *istpl)
{
    int32_t ret;

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_image_capture != NULL) {
            ret = m_fp_impl_handler->fp_test_image_capture(gentpl, buffer, len, quality, area, istpl);
            if (sl_fp_calibrate_step(4) > 0) {
                sl_fp_calibrate_step(5);
            }
        } else {
            LOG_MSG_DEBUG("No implement fp_test_image_capture");
        }
    }

    return ret;
}

int32_t sl_fp_test_frrfar_send_group_image(uint32_t frr, void *buffer, uint32_t *len)
{
    int32_t ret;

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_frrfar_send_group_image != NULL) {
            ret = m_fp_impl_handler->fp_test_frrfar_send_group_image(frr, buffer, len);
        } else {
            LOG_MSG_DEBUG("No implement fp_test_frrfar_send_group_image");
        }
    }

    return ret;
}

int32_t sl_fp_test_image_finish(void)
{
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_image_finish != NULL) {
            m_fp_impl_handler->fp_test_image_finish();
        } else {
            LOG_MSG_DEBUG("No implement fp_test_image_finish");
        }
    }

    return SL_SUCCESS;
}

int32_t sl_fp_test_deadpx(uint32_t *result, uint32_t *deadpx, uint32_t *badline)
{
#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
    return _sl_fp_test_sram(result, deadpx, badline, 0);
#else
    int32_t ret;

    sl_fp_download_normal();

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_deadpx != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_test_deadpx(result, deadpx, badline);
            silfp_dev_disable(m_dev_fd);
        } else {
            LOG_MSG_DEBUG("No implement fp_test_deadpx");
        }
    }

    return ret;
#endif /* SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC */
}

int32_t sl_fp_test_speed(void *buffer, uint32_t *len)
{
    int32_t ret;

    ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_speed != NULL) {
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_test_speed(buffer, len);
            silfp_dev_disable(m_dev_fd);
        } else {
            LOG_MSG_DEBUG("No implement fp_test_speed");
        }
    }

    return ret;
}

int32_t sl_fp_wakelock(uint8_t lock)
{
    int32_t ret = -SL_ERROR_DEV_OPEN_FAILED;

    if (m_dev_fd > 0) {
        ret = silfp_dev_wakelock(m_dev_fd, lock);
    }

    return ret;
}

int32_t sl_fp_chip_pwdn(void)
{
    int32_t ret = -SL_ERROR_DEV_OPEN_FAILED;

    if (m_dev_fd > 0) {
        ret = silfp_dev_pwdn(m_dev_fd, (m_shutdown_by_avdd ? SIFP_PWDN_POWEROFF : SIFP_PWDN_NONE));
        if (m_shutdown_by_avdd && (m_fp_impl_handler->fp_alg_set_param != NULL)) {
            uint8_t onoff = 0;
            uint32_t len = sizeof(onoff);
            uint32_t result = 0;
            silfp_dev_enable(m_dev_fd);
            ret = m_fp_impl_handler->fp_alg_set_param(0, &onoff, &len, &result);
            silfp_dev_disable(m_dev_fd);
        }
    }

    return ret;
}

int32_t sl_fp_create_proc_node(void *chipname)
{
    int32_t ret = -SL_ERROR_DEV_OPEN_FAILED;

    if (m_dev_fd > 0) {
        ret = silfp_dev_create_proc_node(m_dev_fd, chipname);
    }

    return ret;
}
/******add by pruce_tang_20180616 for calibrate interface match  --start*****/
/***********set screen light***********/
int32_t sl_fp_set_hbm_mode(uint32_t mode)
{
    char buff[50];
    int fd = open(FP_CONFIG_SCREEN_HBM_PATH,O_WRONLY);
    if(fd<0) {
        LOG_MSG_ERROR("open brightness node1 failed,try node2!");
        fd = open(FP_CONFIG_SCREEN_HBM_PATH2,O_WRONLY);
        if(fd<0) {
            LOG_MSG_ERROR("open failed,hbm node not exist!");
            return fd;
        }
    }
    sprintf(buff,"%d",(int)mode);
    write(fd,buff,50);
    close(fd);
    return fd;
}

int32_t sl_fp_set_brightness(uint32_t mode)
{
    char buff[50];
    int fd = open(FP_CONFIG_SCREEN_BRIGHTNESS_PATH,O_WRONLY);
    if(fd<0) {
        LOG_MSG_ERROR("open brightness node1 failed,try node2!");
        fd = open(FP_CONFIG_SCREEN_BRIGHTNESS_PATH2,O_WRONLY);
        if(fd<0) {
            LOG_MSG_ERROR("open failed,brightness node not exist!");
            return fd;
        }
    }
    sprintf(buff,"%d",(int)mode);
    write(fd,buff,50);
    close(fd);
    return fd;
}

int32_t sl_fp_get_finger_down_loop(void)
{
    int32_t ret = -1;
    uint32_t status = 0;

    sl_fp_download_normal();

    while (!silfp_finger_is_canceled()) {
        ret = _fp_get_finger_status(&status);
        if (ret < 0) {
            break;
        }

        if (status > 0) {
            break;
        } else {
            ret = -SL_ERROR_CANCELED;
        }
    }

    return ret;
}
/******add by pruce_tang_20180616 for interface match  --end*****/

/**tp info struct for underglass fingerprint,add by pruce_tang_20180710 start **/

#define TP_TOUCH_INFO_FILE_PATH "/data/system/silead/"
#define TP_TOUCH_INFO_FILE_NAME "tp_touch_info.txt"
int sl_set_tp_touch_info(fingerprint_tp_info_t *tp_touch_info,int32_t *data)
{
    tp_touch_info->touch_state = *data;
    tp_touch_info->touch_positon_x = *(data+1);
    tp_touch_info->touch_positon_y = *(data+2);
    tp_touch_info->touch_coverage = *(data+3);
    tp_touch_info->image_state= *(data+4);	
    return 0;
}

int sl_get_tp_touch_info_from_file(const char *file_path,const char *file_name)  
{  
    int32_t read_buff[10]={0};
    int32_t i;
    char open_path[64];
    sprintf(open_path,"%s%s",file_path,file_name);	
    FILE *fpRead=fopen(open_path,"r");  
    if(fpRead==NULL)  
    {  
        LOG_MSG_ERROR("%s%s file not exist!",TP_TOUCH_INFO_FILE_PATH,TP_TOUCH_INFO_FILE_NAME);
        return 0;  
    }  
    for( i=0;i<10;i++)  
    {  
        fscanf(fpRead,"%d ",&read_buff[i]);  
    }  
    fclose(fpWrite);
    sl_set_tp_touch_info(&m_pre_tp_touch_info, &read_buff[0]);
    LOG_MSG_DEBUG("pre_tp_info:touch_state=%d,pos_x=%d,pos_y=%d,coverage=%d,img_state=%d",
		  read_buff[0],read_buff[1],read_buff[2],read_buff[3],read_buff[4]);
    sl_set_tp_touch_info(&m_pre_tp_touch_info, &read_buff[5]);
    LOG_MSG_DEBUG(("pre_tp_info:touch_state=%d,pos_x=%d,pos_y=%d,coverage=%d,img_state=%d",
		  read_buff[5],read_buff[6],read_buff[7],read_buff[8],read_buff[9]);
    return 1;  
} 

int32_t sl_get_tp_touch_info(uint8_t mode)
{
    int32_t ret = -1;
    fingerprint_tp_info_t *tp_touch_info =NULL;
    if(0==mode){
        ret = silfp_dev_get_tp_touch_info(m_dev_fd, &m_pre_tp_touch_info);
	 	m_pre_tp_touch_info.image_state = mode;
        tp_touch_info = &m_pre_tp_touch_info;
    }else if(1==mode){
        ret = silfp_dev_get_tp_touch_info(m_dev_fd, &m_later_tp_touch_info);
	 	m_later_tp_touch_info.image_state = mode;
        tp_touch_info = &m_later_tp_touch_info;		
    }
    		
    if(ret<0){
            LOG_MSG_ERROR("get tp touch info failed!");
            return ret;        
    }

    if (m_fp_impl_handler->fp_alg_set_param != NULL && NULL != tp_touch_info) {
        uint32_t len = sizeof(fingerprint_tp_info_t);
        uint32_t result = 0;		
        silfp_dev_enable(m_dev_fd);
        ret = m_fp_impl_handler->fp_alg_set_param(1, tp_touch_info, &len, &result);
        silfp_dev_disable(m_dev_fd);
    }

    return ret;
}

int sl_get_pre_tp_touch_info(uint32_t state,uint32_t pos_x,uint32_t pos_y,uint32_t coverage)
{
    m_pre_tp_touch_info.touch_state = state;
    m_pre_tp_touch_info.touch_positon_x = pos_x;
    m_pre_tp_touch_info.touch_positon_y = pos_y;
    m_pre_tp_touch_info.touch_coverage = coverage;
    return 0;
}
int sl_set_later_tp_touch_info(uint32_t state,uint32_t pos_x,uint32_t pos_y,uint32_t coverage)
{
    m_later_tp_touch_info.touch_state = state;
    m_later_tp_touch_info.touch_positon_x = pos_x;
    m_later_tp_touch_info.touch_positon_y = pos_y;
    m_later_tp_touch_info.touch_coverage = coverage;
    return 0;
}

int sl_init_tp_touch_info()
{
    memset(&m_later_tp_touch_info, 0, sizeof(m_later_tp_touch_info));
    memset(&m_pre_tp_touch_info, 0, sizeof(m_pre_tp_touch_info));
    return 0;
}
/**tp info struct for underglass fingerprint,add by pruce_tang_20180710 end **/