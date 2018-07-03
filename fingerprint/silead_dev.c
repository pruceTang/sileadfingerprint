/******************************************************************************
 * @file   silead_dev.c
 * @brief  Contains /dev/silead_fp operate functions.
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
 * David Wang  2018/5/28   0.1.1      Support poll/read if netlink id invalid
 * David Wang  2018/6/5    0.1.2      Support wakelock & pwdn
 *
 *****************************************************************************/

#define FILE_TAG "silead_dev"
#include "log/logmsg.h"

#include <string.h>
#include <errno.h>

#include "silead_msg.h"
#include "silead_netlink.h"
#include "silead_dev.h"
#include "silead_error.h"
#include "silead_key.h"

#define ENOENT 2

static int32_t m_res_inited = 0;
static int32_t m_need_spi_op = 1;

int32_t silfp_dev_get_ver(int32_t fd, char *ver, uint32_t len)
{
    int32_t ret = -SL_ERROR_DEV_IOCTL_FAILED;
    if (ver && len >= 10) {
        memset(ver, 0, len);
        ret = ioctl(fd, SIFP_IOC_GET_VER, ver);
        if (ret < 0) {
            ret = -SL_ERROR_DEV_IOCTL_FAILED;
        }
    }

    return ret;
}

int32_t silfp_dev_enable(int32_t fd)
{
    int32_t ret = 0;

    if (m_need_spi_op) {
        ret = ioctl(fd, SIFP_IOC_ACQ_SPI, NULL);
        if (ret < 0) {
            if (ENOENT == errno) {
                LOG_MSG_INFO("dev_spi_op not supported!");
                m_need_spi_op = 0;
                ret = 0;
            } else {
                ret = -SL_ERROR_DEV_IOCTL_FAILED;
            }
        }
    }

    return ret;
}

int32_t silfp_dev_disable(int32_t fd)
{
    int32_t ret = 0;

    if (m_need_spi_op) {
        ret = ioctl(fd, SIFP_IOC_RLS_SPI, NULL);
        if (ret < 0) {
            ret = -SL_ERROR_DEV_IOCTL_FAILED;
        }
    }

    return ret;
}

int32_t silfp_dev_init(int32_t fd, FP_DEV_CONF *t)
{
    int32_t ret;
    char ver[11];

    LOG_MSG_VERBOSE("init");

    if (m_res_inited) {
        return 1;
    }

    if (silfp_dev_get_ver(fd, ver, sizeof(ver)) >= 0) {
        LOG_MSG_INFO("dev_ver version: %s", ver);
    }

    ret = ioctl(fd, SIFP_IOC_INIT, t);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    if (ret >= 0 && !m_res_inited && t != NULL) {
        silfp_msg_init();
        silfp_nl_init(t->nl_id, fd);
        m_res_inited = 1;
    }

    return ret;
}

int32_t silfp_dev_deinit(int32_t fd)
{
    int32_t ret = ioctl(fd, SIFP_IOC_DEINIT, NULL);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    silfp_dev_disable(fd);
    silfp_nl_deinit();
    silfp_msg_deinit();
    m_res_inited = 0;

    LOG_MSG_VERBOSE("deinit");

    return ret;
}

int32_t silfp_dev_hw_reset(int32_t fd, uint8_t delayms)
{
    int32_t ret = ioctl(fd, SIFP_IOC_RESET, &delayms);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

int32_t silfp_dev_enable_irq(int32_t fd)
{
    int32_t ret = ioctl(fd, SIFP_IOC_ENABLE_IRQ, NULL);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

int32_t silfp_dev_disable_irq(int32_t fd)
{
    int32_t ret = ioctl(fd, SIFP_IOC_DISABLE_IRQ, NULL);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

int32_t silfp_dev_get_screen_status(int32_t fd, uint8_t *status)
{
    int32_t ret = ioctl(fd, SIFP_IOC_SCR_STATUS, status);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

int32_t silfp_dev_set_screen_cb(screen_cb listen, void *param)
{
    return silfp_nl_set_screen_cb(listen, param);
}

int32_t silfp_dev_wait_finger_status(int32_t fd, int32_t uncancelable)
{
    int32_t ret;
    int32_t type;

    silfp_msg_clean();

    if (silfp_dev_enable_irq(fd) < 0) {
        LOG_MSG_ERROR("IOC_ENABLE_IRQ fail");
    }

    type = silfp_msg_recv(uncancelable);
    LOG_MSG_VERBOSE("silfp_msg_recv: %d", type);

    if (silfp_dev_disable_irq(fd) < 0) {
        LOG_MSG_ERROR("IOC_DISABLE_IRQ fail");
    }

    if (type == SIFP_MSG_CANCEL) {
        return -SL_ERROR_CANCELED;
    }

    return SL_SUCCESS;
}

void silfp_dev_cancel(void)
{
    if (m_res_inited) {
        silfp_msg_send(SIFP_MSG_CANCEL);
    }
}

void silfp_dev_sync_finger_status_optic(int32_t __unused down)
{
    if (m_res_inited) {
        silfp_msg_send(SIFP_MSG_IRQ);
    }
}

int32_t silfp_dev_send_key(int32_t fd, uint32_t key)
{
    int32_t ret;

    struct fp_dev_key_t k = {
        .key = key,
        .value = NAV_KEY_FLAG_CLICK,
    };

    LOG_MSG_DEBUG("should report key = %s", silead_key_get_des((int32_t)key));

    ret = ioctl(fd, SIFP_IOC_KEY_EVENT, &k);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

int32_t silfp_dev_set_log_level(int32_t fd, uint8_t *lvl)
{
    int32_t ret = 0;

    if (lvl) {
        ret = ioctl(fd, SIFP_IOC_DBG_LEVEL, lvl);
        if (ret < 0) {
            ret = -SL_ERROR_DEV_IOCTL_FAILED;
        }
    }
    return ret;
}

int32_t silfp_dev_wakelock(int32_t fd, uint8_t lock)
{
    int32_t ret = ioctl(fd, SIFP_IOC_WAKELOCK, &lock);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

int32_t silfp_dev_pwdn(int32_t fd, uint8_t avdd_op)
{
    int32_t ret = ioctl(fd, SIFP_IOC_PWDN, &avdd_op);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

int32_t silfp_dev_create_proc_node(int32_t fd, char *chipname)
{
    char name[PROC_VND_ID_LEN];
    int32_t ret;

    if (!chipname) {
        return -SL_ERROR_DEV_IOCTL_FAILED;
    }
    memset(name, 0x0, sizeof(name));
    strncpy(name, chipname, sizeof(name)-1);
    ret = ioctl(fd, SIFP_IOC_PROC_NODE, name);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}
