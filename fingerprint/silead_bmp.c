/******************************************************************************
 * @file   silead_bmp.c
 * @brief  Contains Bitmap file operate functions.
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
 * Rich Li     2018/6/7    0.1.1      Support dump image
 *
 *****************************************************************************/

#define FILE_TAG "silead_img"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include "silead_error.h"
#include "silead_bmp.h"

#ifndef CHECK_IMAGE_MAGIC_SUPPORT
#define CHECK_IMAGE_MAGIC_SUPPORT 0
#endif

#define BMP_MAGIC1 0x0511
#define BMP_MAGIC2 0x1EAD

typedef struct __attribute__((packed)) _bitmap_file_header {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct __attribute__((packed)) _bitmap_info_header {
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;

typedef struct __attribute__((packed)) _bitmap_verify {
    uint32_t rand1;
    uint32_t rand2;
    uint32_t checksum;
} BITMAPVERIFY;

// bmp head + bmp info head + color table
#if CHECK_IMAGE_MAGIC_SUPPORT
#define IMG_DATA_OFFSET (256*4 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPVERIFY))
#else
#define IMG_DATA_OFFSET (256*4 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))
#endif

static unsigned char m_color_map[256*4];
static int32_t m_color_inited = 0;

static int32_t _bmp_calc_checksum(const void *in, const uint32_t len, uint32_t *checksum)
{
    uint32_t i;
    unsigned long int sum = 0;
    const unsigned char *p;

    if (in == NULL || checksum == NULL || len == 0) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    p = (const unsigned char *)in;
    for (i = 0; i < len; i++) {
        sum += *p++;
    }

    while (sum >> 16) {
        sum = (sum >> 16) + (sum & 0xFFFF);
    }

    *checksum = ~sum;

    return SL_SUCCESS;
}

uint32_t silfp_bmp_get_size(const uint32_t w, const uint32_t h)
{
    uint32_t len = IMG_DATA_OFFSET;
    uint32_t fixed_w = (w + 3) / 4 * 4;

    len += fixed_w*h;
    return len;
}

int32_t silfp_bmp_get_img(void *dest, const uint32_t size, const void *buf, const uint32_t w, const uint32_t h)
{
    uint32_t i, j;

    uint32_t fixed_w;
    const unsigned char *p;
    unsigned char *pdest = (unsigned char *)dest;
    uint32_t checksum = 0;

    BITMAPFILEHEADER bf = {
        .bfType = 0x4d42,
        .bfSize = IMG_DATA_OFFSET + w*h,
        .bfReserved1 = BMP_MAGIC1,
        .bfReserved2 = BMP_MAGIC2,
        .bfOffBits = IMG_DATA_OFFSET,
    };
    BITMAPINFOHEADER bi = {
        .biSize = sizeof(bi),
        .biWidth = w,
        .biHeight = h,
        .biPlanes = 1,
        .biBitCount = 8,
        .biCompression = 0,
        .biSizeImage = w*h,
        .biXPelsPerMeter = 0,
        .biYPelsPerMeter = 0,
        .biClrUsed = 256,
        .biClrImportant = 256,
    };

    if (dest == NULL || size < silfp_bmp_get_size(w, h) || buf == NULL || w == 0 || h == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    BITMAPVERIFY bv;
    bv.rand1 = rand();
    bv.rand2 = rand();

    _bmp_calc_checksum(buf, w*h, &checksum);
    bv.checksum = checksum;

    if (!m_color_inited) {
        for (i = 0; i < 256; i ++) {
            m_color_map[i*4] = m_color_map[i*4+1] = m_color_map[i*4+2] = i;
            m_color_map[i*4+3] = 0;
        }
        m_color_inited = 1;
    }

    memcpy(pdest, &bf, sizeof(bf));
    pdest += sizeof(bf);

    memcpy(pdest, &bi, sizeof(bi));
    pdest += sizeof(bi);

    memcpy(pdest, &m_color_map, sizeof(m_color_map));
    pdest += sizeof(m_color_map);

#if CHECK_IMAGE_MAGIC_SUPPORT
    memcpy(pdest, &bv, sizeof(bv));
    pdest += sizeof(BITMAPVERIFY);
#endif

    fixed_w = (w + 3) / 4 * 4;
    p = (const unsigned char *)buf;
    p += (w * (h - 1));
    for (i = 0; i < h; i++) {
        memcpy(pdest, p, w);
        pdest += w;
        p -= w;

        if (fixed_w > w) {
            memset(pdest, 0, fixed_w - w);
            pdest += (fixed_w - w);
        }
    }

    return (pdest - (unsigned char *)dest);
}

int32_t silfp_bmp_get_data(void *dest, const uint32_t w, const uint32_t h, const void *buf, const uint32_t size)
{
    uint32_t fixed_w;
    uint32_t i, j;
    uint32_t checksum = 0xFFFFFFFF;

    const unsigned char *p = (const unsigned char *)buf;
    unsigned char *pdest = (unsigned char *)dest;

    if (pdest == NULL || w == 0 || h == 0 || p == NULL || size < IMG_DATA_OFFSET) {
        return -SL_ERROR_BAD_PARAMS;
    }

    BITMAPFILEHEADER *bf = (BITMAPFILEHEADER *)p;
    BITMAPINFOHEADER *bi = (BITMAPINFOHEADER *)(p + sizeof(BITMAPFILEHEADER));
    p += bf->bfOffBits;

#if CHECK_IMAGE_MAGIC_SUPPORT
    BITMAPVERIFY *bv = (BITMAPVERIFY *)(p - sizeof(BITMAPVERIFY));
    if (bf->bfReserved1 != BMP_MAGIC1 || bf->bfReserved2 != BMP_MAGIC2 || bf->bfOffBits != IMG_DATA_OFFSET) {
        return -SL_ERROR_BAD_PARAMS;
    }
#endif

    if (bf->bfType != 0x4d42 || bi->biWidth != w || bi->biHeight != h || bi->biBitCount != 8) {
        LOG_MSG_ERROR("bf->bfType=%x, bi->biWidth=%d, bi->biHeight=%d, bi->biBitCount=%d, w=%d, h=%d", bf->bfType, bi->biWidth, bi->biHeight, bi->biBitCount, w, h);
        return -SL_ERROR_BAD_PARAMS;
    }

    fixed_w = (w + 3) / 4 * 4;
    if (fixed_w * h + bf->bfOffBits > size) {
        return -SL_ERROR_BAD_PARAMS;
    }

    p += (fixed_w * (h - 1));
    for (i = 0; i < h; i++) {
        memcpy(pdest, p, w);
        pdest += w;
        p -= fixed_w;
    }

#if CHECK_IMAGE_MAGIC_SUPPORT
    _bmp_calc_checksum(dest, w*h, &checksum);
    if (checksum != bv->checksum) {
        return -SL_ERROR_BAD_PARAMS;
    }
#endif

    return (pdest - (unsigned char *)dest);
}

static int32_t _bmp_get_save_file_path(char *path, const char *prefix, const uint32_t len)
{
    static int32_t index = 0;
    time_t timep;
    struct tm *p;

    time(&timep);
    p = localtime(&timep);

    if (path != NULL && len > 0) {
        if (prefix) {
            snprintf(path, len, "/data/system/silead/%04d-%s-%04d%02d%02d-%02d%02d%02d.bmp",
                     index++, prefix, (1900 + p->tm_year), ( 1 + p->tm_mon), p->tm_mday,
                     p->tm_hour, p->tm_min, p->tm_sec);
        } else {
            snprintf(path, len, "/data/system/users/0/fpdata/%04d.bmp", index++);
        }
    }

    return index;
}

int32_t silfp_bmp_save(const void *buf, const char *prefix, const uint32_t size, const uint32_t w, const uint32_t h)
{
    int32_t ret;
    void *pbuff = NULL;
    const unsigned char *p;
    int32_t len;
    int32_t fd = -1;
    char path[512];

    if (buf == NULL || size < w * h) {
        return -SL_ERROR_BAD_PARAMS;
    }

    len = silfp_bmp_get_size(w, h);
    pbuff = malloc(len);
    if (pbuff == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    do {
        len = silfp_bmp_get_img(pbuff, len, buf, w, h);
        if (len <= 0) {
            break;
        }

        _bmp_get_save_file_path(path, prefix, sizeof(path));
        fd = open(path, O_RDWR | O_CREAT, 0644);
        if (fd < 0) {
            break;
        }

        p = (const unsigned char *)pbuff;
        do {
            ret = write(fd, p, len);
            if (ret > 0)  {
                len -= ret;
                p += ret;
            } else {
                break;
            }
        } while(len > 0);
    } while (0);

    if (fd >= 0) {
        close(fd);
    }

    if (pbuff != NULL) {
        free(pbuff);
        pbuff = NULL;
    }
    return 0;
}
