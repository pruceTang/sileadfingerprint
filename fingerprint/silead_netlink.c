/******************************************************************************
 * @file   silead_netlink.c
 * @brief  Contains netlink communication functions.
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
 * David Wang  2018/5/28   0.1.1      Support poll/read if netlink id invalid
 *
 *****************************************************************************/

#define FILE_TAG "silead_nl"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <poll.h>
#include <errno.h>

#include "silead_netlink.h"
#include "silead_msg.h"

static pthread_t nl_rcv_thread;
static int32_t m_socket_id = 0;
static uint8_t m_nl_quit;

static screen_cb m_screen_cb;
static void* m_screen_cb_param;

#define INFINITETIME -1

static void _relay_msg(unsigned char msg)
{
    switch(msg) {
    case SIFP_NETLINK_START:
        LOG_MSG_VERBOSE("recv START msg");
        break;
    case SIFP_NETLINK_IRQ:
        LOG_MSG_DEBUG("recv IRQ msg");
#ifndef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
        silfp_msg_send(SIFP_MSG_IRQ);
#endif
        break;
    case SIFP_NETLINK_SCREEN_ON:
        LOG_MSG_DEBUG("recv SCREEN ON msg");
        if (m_screen_cb != NULL) {
            m_screen_cb(1, m_screen_cb_param);
        }
        break;
    case SIFP_NETLINK_SCREEN_OFF:
        LOG_MSG_DEBUG("recv SCREEN OFF msg");
        if (m_screen_cb != NULL) {
            m_screen_cb(0, m_screen_cb_param);
        }
        break;
    case SIFP_NETLINK_DISCONNECT:
        LOG_MSG_VERBOSE("recv DISCONNECT msg");
        m_nl_quit = 1;
        break;
    default:
        LOG_MSG_ERROR( "Unknown netlink msg %d", msg);
        break;
    }
}

static void *_thr_netlink_recv(void *handle)
{
    int32_t ret;
    struct nlmsghdr *nlh = NULL;
    struct sockaddr_nl local, dest;
    struct msghdr msg;
    struct iovec iov;
    unsigned char value;

    do {
        m_socket_id = socket(AF_NETLINK, SOCK_RAW, (uint32_t)(unsigned long)handle);
        if (m_socket_id < 0) {
            LOG_MSG_ERROR("socket failed (%d:%s)", errno, strerror(errno));
            break;
        }

        memset(&local, 0, sizeof(struct sockaddr_nl));
        local.nl_family = AF_NETLINK;
        local.nl_pid = getpid(); /*local process id*/
        local.nl_groups = 0;

        ret = bind(m_socket_id, (struct sockaddr*)&local, sizeof(struct sockaddr_nl));
        if (ret != 0) {
            LOG_MSG_ERROR("bind failed (%d:%s)", errno, strerror(errno));
            break;
        }

        /* send init message */
        memset(&dest, 0, sizeof(struct sockaddr_nl));
        dest.nl_family = AF_NETLINK;
        dest.nl_pid = 0; /*destination is kernel so set to 0*/
        dest.nl_groups = 0;

        nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_NL_MSG_LEN));
        if (NULL == nlh) {
            LOG_MSG_ERROR("Alloc NLH fail");
            break;
        }
        nlh->nlmsg_len = NLMSG_SPACE(MAX_NL_MSG_LEN);
        nlh->nlmsg_pid = getpid();
        nlh->nlmsg_flags = 0;
        strcpy(NLMSG_DATA(nlh), "FP");

        iov.iov_base = (void*) nlh;
        iov.iov_len = nlh->nlmsg_len;

        memset(&msg, 0, sizeof(struct msghdr));
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_name = (void*) &dest;
        msg.msg_namelen = sizeof(struct sockaddr_nl);

        // send pid to kernel
        if (sendmsg(m_socket_id, &msg, 0) < 0) {
            break;
        }

        LOG_MSG_VERBOSE("Netlink recv thread running %d .....", getpid());

        memset(nlh, 0, NLMSG_SPACE(MAX_NL_MSG_LEN));
        while (!m_nl_quit) {
            ret = recvmsg(m_socket_id, &msg, 0);
            if (ret <= 0) {
                LOG_MSG_ERROR( "recvmsg failed, ret %d", ret);
                continue;
            }

            value = *((char *) NLMSG_DATA(nlh));
            _relay_msg(value);
        }
    } while(0);

    if (nlh) {
        free(nlh);
        nlh = NULL;
    }

    return NULL;
}

static void *_thr_read(void *handle)
{
    int32_t ret;
    struct pollfd fds;
    unsigned char value;

    do {
        if (!handle) {
            LOG_MSG_ERROR("invalid drv handl!");
            break;
        }

        memset(&fds, 0, sizeof(struct pollfd));
        fds.fd = (int32_t)(long)handle;
        fds.events = POLLIN;

        LOG_MSG_VERBOSE("Read thread running %d .....", getpid());

        while (!m_nl_quit) {
            ret = poll(&fds, sizeof(fds)/sizeof(struct pollfd), INFINITETIME);
            if (ret <= 0) {
                LOG_MSG_ERROR( "poll failed, ret %d", ret);
                continue;
            }

            ret = read(fds.fd,&value,1);
            if (ret <= 0) {
                LOG_MSG_ERROR( "read failed, ret %d", ret);
                continue;
            }
            _relay_msg(value);
        }
    } while(0);

    return NULL;
}

int32_t silfp_nl_init(uint8_t nl_id, int32_t fd)
{
    LOG_MSG_VERBOSE("init: nl_id = %d, fd = %d", nl_id, fd);

    m_nl_quit = 0;
    m_screen_cb = NULL;
    m_screen_cb_param = NULL;

    if ( nl_id > 0 ) {
        if (pthread_create(&nl_rcv_thread, NULL, _thr_netlink_recv, (void *)(unsigned long)(nl_id)) != 0) {
            LOG_MSG_ERROR( "pthread_create failed");
        }
    } else {
        if (pthread_create(&nl_rcv_thread, NULL, _thr_read, (void *)(unsigned long)(fd)) != 0) {
            LOG_MSG_ERROR( "pthread_create failed");
        }
    }
    return 0;
}

int32_t silfp_nl_deinit(void)
{
    void *ret;

    if (pthread_join(nl_rcv_thread, &ret) != 0 ) {
        LOG_MSG_ERROR( "pthread_join failed");
    }

    if (m_socket_id > 0 ) {
        close(m_socket_id);
    }

    m_screen_cb = NULL;
    m_screen_cb_param = NULL;

    LOG_MSG_VERBOSE("deinit");
    return 0;
}

int32_t silfp_nl_set_screen_cb(screen_cb listen, void *param)
{
    m_screen_cb = listen;
    m_screen_cb_param = param;

    return 0;
}