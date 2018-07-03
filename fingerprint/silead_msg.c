/******************************************************************************
 * @file   silead_msg.c
 * @brief  Contains pipe communication functions.
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
 * Luke Ma     2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_msg"
#include "log/logmsg.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>

#include "silead_msg.h"

#define STR_CANCEL "c"
#define STR_IRQ "i"
#define MSG_LEN 1

static int32_t s_fdWakeupRead = -1;
static int32_t s_fdWakeupWrite = -1;

struct pollfd mPollFds;

int32_t silfp_msg_init(void)
{
    int32_t ret;
    int32_t filedes[2];

    LOG_MSG_VERBOSE("init");

    ret = pipe(filedes);
    if (ret >= 0) {
        s_fdWakeupRead = filedes[0];
        s_fdWakeupWrite = filedes[1];

        fcntl(s_fdWakeupRead, F_SETFL, O_NONBLOCK | O_NOATIME);
        fcntl(s_fdWakeupWrite, F_SETFL, O_NONBLOCK);

        mPollFds.fd = s_fdWakeupRead;
        mPollFds.events = POLLIN;
        mPollFds.revents = 0;
    } else {
        LOG_MSG_ERROR("pipe failed (%d:%s)", errno, strerror(errno));
    }

    return 0;
}

void silfp_msg_deinit(void)
{
    close(s_fdWakeupRead);
    s_fdWakeupRead = -1;
    close(s_fdWakeupWrite);
    s_fdWakeupWrite = -1;

    LOG_MSG_VERBOSE("deinit");
}

void silfp_msg_send(int32_t type)
{
    int32_t ret;
    char *buf = NULL;

    if (SIFP_MSG_IRQ == type) {
        buf = STR_IRQ;
    } else if (SIFP_MSG_CANCEL == type) {
        buf = STR_CANCEL;
    } else {
        LOG_MSG_ERROR("invalid type=%d", type);
        return;
    }

    do {
        ret = write(s_fdWakeupWrite, buf, MSG_LEN);
        LOG_MSG_VERBOSE("write buf=%s, ret=%d", buf, ret);
    } while (ret < 0 && errno == EINTR);
}

static int32_t _msg_read_msg(int32_t uncancelable)
{
    ssize_t count;
    char buff[16] = {0};

    do {
        count = read(s_fdWakeupRead, buff, MSG_LEN);
        LOG_MSG_VERBOSE("buff=%s, count=%zd", buff, count);
    } while (count < 0 && errno == EINTR);

    if (count > 0) {
        if (memcmp(buff, STR_CANCEL, MSG_LEN) == 0) {
            if (!uncancelable) {
                return SIFP_MSG_CANCEL;
            }
        } else if (memcmp(buff, STR_IRQ, MSG_LEN) == 0) {
            return SIFP_MSG_IRQ;
        }
    }
    return SIFP_MSG_UNKNOW;
}

void silfp_msg_clean(void)
{
    char buff[1024] = {0};
    int32_t ret;

    do {
        ret = read(s_fdWakeupRead, &buff, sizeof(buff));
        LOG_MSG_VERBOSE("buff=%s, ret=%d", buff, ret);
    } while (ret > 0 || (ret < 0 && errno == EINTR));
}

int32_t silfp_msg_recv(int32_t uncancelable)
{
    int32_t n = 0;
    int32_t ret;

    for (;;) {
        LOG_MSG_VERBOSE("poll");

        n = poll(&mPollFds, 1, -1);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            LOG_MSG_ERROR("poll error (%d)", errno);
        }

        if (mPollFds.revents & POLLIN) {
            mPollFds.revents = 0;
            ret = _msg_read_msg(uncancelable);
            if (ret >= 0) {
                return ret;
            }
        }
    }

    return SIFP_MSG_UNKNOW;
}