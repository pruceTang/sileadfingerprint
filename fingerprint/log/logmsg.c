/******************************************************************************
 * @file   logmsg.c
 * @brief  Contains log message control.
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

#include "logmsg.h"
#include <cutils/properties.h>

#define LOG_NAMESPACE ("log.tag." LOG_TAG)

static int toLevel(const char* value)
{
    switch (value[0]) {
        case 'V':
        case 'v':
            return 0;  // VERBOSE
        case 'D':
        case 'd':
            return 1;  // DEBUG
        case 'I':
        case 'i':
            return 2;  // INFO
        case 'W':
        case 'w':
            return 3;  // WARNING
        case 'E':
        case 'e':
            return 4;  // ERROR
        case 'A':
        case 'a':
            return 0;  // ALL
        case 'S':
        case 's':
            return 100; // SUPPRESS
    }
    return 2;
}

int isLoggable(int level)
{
    int len;
    char buf[PROPERTY_VALUE_MAX];

    len = property_get(LOG_NAMESPACE, buf, "");
    int logLevel = toLevel(buf);
    return (level >= logLevel) ? 1 : 0;
}