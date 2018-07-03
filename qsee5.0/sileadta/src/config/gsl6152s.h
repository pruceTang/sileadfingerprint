/******************************************************************************
 * @file   gsl6152s.h
 * @brief  Contains Chip configurations.
 *
 *
 * Copyright (c) 2016-2018 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *****************************************************************************/

/* Auto-generated file; do not edit. */

#ifndef __SILEAD_FP_GSL6152S_H__
#define __SILEAD_FP_GSL6152S_H__

#include "silead_config.h"

const cf_register_t reg_normal_gsl6152s[] = {
    {0x00000098, 0x00044710},
    {0x000000A0, 0x00000001},
    {0x00000098, 0x20644700},
    {0x000000A0, 0x00000000},
    {0xFF0A0010, 0x00000001},
    {0xFF0A00C0, 0x13114304},
    {0xFF0A00C4, 0x00000000},
    {0xFF0A0090, 0x00004026},
    {0xFF0A0094, 0x01000001},
    {0xFF0A00EC, 0x00000000},
    {0x000000E4, 0x00000005},
    {0x000000E8, 0x00000043},
    {0x00000094, 0x01020000},
    {0xFF00004C, 0x00009103},
    {0xFF010008, 0x00000039},
    {0xFF080000, 0x14226202},
    {0xFF080030, 0x003F1233},
    {0xFF0800D4, 0xFFFF0143},
    {0xFF080820, 0x001D0000},
    {0x01000400, 0x00000280},
    {0x01000404, 0x00000020},
    {0x01000408, 0x00200020},
    {0x0100040C, 0x02400020},
    {0x01000410, 0x02600020},
};

const cf_register_t reg_fg_detect_gsl6152s[] = {
    {0xFF000020, 0x00030301},
    {0xFF00002C, 0x00000307},
    {0xFF08000C, 0x00038C8C},
};

const cf_register_t reg_partial_gsl6152s[] = {
    {0xFF0A0010, 0x00000001},
    {0xFF080000, 0x14226202},
    {0xFF080054, 0x00000100},
    {0xFF080150, 0x00480007},
    {0xFF080154, 0x00090000},
    {0xFF080814, 0x00000000},
};

const cf_register_t reg_full_gsl6152s[] = {
    {0xFF0A0010, 0x00000001},
    {0xFF080054, 0x00000500},
    {0xFF080150, 0x00500007},
    {0xFF080154, 0x00010000},
    {0xFF080814, 0xC1000100},
};

const cf_register_t reg_frm_start_gsl6152s[] = {
    {0xFF080024, 0x00000000},
};

const cf_register_t reg_ds_start_gsl6152s[] = {
    {0xFF080024, 0x03000000},
};

const cf_register_t reg_stop_gsl6152s[] = {
};

const cf_register_t reg_downint_gsl6152s[] = {
    {0xFF080124, 0x00000008},
    {0xFF08012C, 0x00000080},
    {0x000000BF, 0x00000000},
    {0xFF010004, 0x00000930},
    {0xFF0A0010, 0x00000701},
    {0xFF000020, 0x00060301},
    {0xFF00002C, 0x00000303},
    {0xFF08000C, 0x00038C8C},
    {0x00000090, 0x0000800B},
    {0xFF0800D4, 0xFFFF010A},
    {0xFF080030, 0x011F3111},
};

const cf_register_t reg_vkeyint_gsl6152s[] = {
    {0xFF080124, 0x00000042},
    {0xFF080128, 0x20000100},
    {0xFF08012C, 0x00040000},
    {0xFF080164, 0x00000080},
    {0x000000BF, 0x00000000},
    {0xFF010004, 0x00000530},
    {0xFF000020, 0x00060301},
    {0xFF00002C, 0x00000303},
    {0xFF08082C, 0x05000000},
    {0xFF080040, 0x00000300},
    {0xFF080160, 0x48010701},
    {0xFF080034, 0x10002000},
    {0xFF080038, 0x07000040},
    {0xFF0A0010, 0x00000701},
    {0x00000090, 0x0000800B},
    {0xFF000020, 0x00060301},
    {0xFF00002C, 0x00000303},
    {0xFF08000C, 0x00038C8C},
    {0xFF0800D4, 0xFFFF010A},
    {0xFF080030, 0x011F3111},
};

const cf_register_t reg_upint_gsl6152s[] = {
    {0xFF080124, 0x00000008},
    {0xFF08012C, 0x00000080},
    {0x000000BF, 0x00000000},
    {0xFF010004, 0x00000798},
    {0xFF080034, 0x12000380},
    {0xFF0A0010, 0x00000701},
    {0xFF000020, 0x00060301},
    {0xFF00002C, 0x00000303},
    {0xFF08000C, 0x00038C8C},
    {0x00000090, 0x0000800B},
    {0xFF0800D4, 0xFFFF010A},
    {0xFF080030, 0x011F3111},
};

const cf_register_t reg_nav_gsl6152s[] = {
    {0xFF0A0010, 0x00000001},
    {0xFF080150, 0x00500007},
    {0xFF080154, 0x00010000},
    {0xFF080054, 0x00000140},
    {0xFF080814, 0x00000000},
};

const cf_register_t reg_nav_hg_gsl6152s[] = {
    {0xFF0A0010, 0x00000001},
    {0xFF080150, 0x00480007},
    {0xFF080154, 0x00090000},
    {0xFF080054, 0x00000100},
    {0xFF080814, 0x00000000},
};

const cf_register_t reg_nav_ds_gsl6152s[] = {
    {0xFF0A0010, 0x00000001},
    {0xFF080150, 0x00500007},
    {0xFF080154, 0x00010000},
    {0xFF080054, 0x00000140},
    {0xFF080814, 0x00000000},
    {0xFF080040, 0x00010400},
    {0xFF080034, 0x10002800},
    {0xFF080038, 0x0F000050},
    {0xFF080840, 0x070A0701},
    {0xFF080834, 0x000000FF},
    {0xFF080838, 0x000000FF},
    {0xFF08082C, 0x05000001},
    {0xFF000024, 0x63330000},
    {0xFF08000C, 0x00038CA5},
};

const cf_register_t reg_nav_hg_ds_gsl6152s[] = {
};

const cf_register_t reg_snr_gsl6152s[] = {
    {0xFF0A00C0, 0x23114304},
    {0xFF080188, 0x0000C002},
    {0xFF080040, 0x00010400},
    {0xFF080034, 0x10002800},
    {0xFF080038, 0x0F000050},
    {0xFF080840, 0x070A0701},
    {0xFF080834, 0x000000FF},
    {0xFF080838, 0x000000FF},
    {0xFF08082C, 0x05000001},
    {0xFF000024, 0x63330000},
    {0xFF08000C, 0x00038C8C},
};

const cf_register_t reg_deadpix1_gsl6152s[] = {
    {0xFF000020, 0x00020201},
    {0xFF00002C, 0x00000101},
    {0xFF00006C, 0x01000311},
    {0xFF08000C, 0x00030080},
};

const cf_register_t reg_deadpix2_gsl6152s[] = {
    {0xFF000020, 0x00020201},
    {0xFF00002C, 0x00000100},
    {0xFF00006C, 0x01000311},
    {0xFF08000C, 0x00030088},
};

const cf_register_t reg_deadpix3_gsl6152s[] = {
    {0xFF000020, 0x00020201},
    {0xFF00002C, 0x00000100},
    {0xFF00006C, 0x00000311},
    {0xFF08000C, 0x00030075},
};

const cf_register_t reg_hg_coverage_gsl6152s[] = {
    {0xFF080040, 0x00010400},
    {0xFF080034, 0x10002800},
    {0xFF080038, 0x0F000050},
    {0xFF080840, 0x070A0701},
    {0xFF080834, 0x000000FF},
    {0xFF080838, 0x000000FF},
    {0xFF08082C, 0x05000001},
    {0xFF000024, 0x63330000},
    {0xFF08000C, 0x00038C8C},
};

const cf_register_t reg_hwagc_read_gsl6152s[] = {
    {0xFF0A0010, 0x00000001},
    {0xFF08082C, 0x05000000},
    {0xFF080840, 0x04140204},
    {0xFF080834, 0x00000003},
    {0xFF080838, 0x0000000F},
    {0xFF080000, 0x14226202},
};

const cf_register_t reg_hwcov_start_gsl6152s[] = {
    {0xFF080024, 0x01000000},
};

const cf_register_t reg_hwagc_start_gsl6152s[] = {
    {0xFF080024, 0x00000001},
};

const cf_register_t reg_base_ori_gsl6152s[] = {
};

const cf_register_t reg_auto_adjust_gsl6152s[] = {
};

const int32_t FineTune_param_gsl6152s[] = {
    32,         128,        750,        10002,      0,          128,        1,          230,
    220,        250,        180,        220,        200,        254,        150,        250,
    78,         178,        78,         178,        100,        180,        10,         10,
    15,         20,         1,          10,         3,          20,         97,         80,
    99,         90,         75,         15,         0,          254,        8,          4,
    6,          15,         30,         30519,      30263,      26167,      25911,      21815,
    21559,      17463,      17462,      17206,      13110,      13109,      12854,      12853,
    8758,       8757,       8742,       8741,       8740,       8739,       4646,       4645,
    4644,       4643,       4628,       4627,       4626,       4612,       4625,       4610,
    4609,       0,          180,        40,         24,         8,          120,        1,
    0,          0,          0,          1,          20,         0,          0,          3,
    2,          17171974,   0,          0,          0,          78,         178,        78,
    178,        0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          7,          16,
    14,         13,         11,         9,          9,          7,          6,          6,
    6,          6,          6,          7,          9,          13,         18,         16,
    0,          8,          8,          30,         10,         0,          5,          20,
    15,         8,
};

const int32_t Navi_param_gsl6152s[] = {
    0,          3,          8,          3,          8,          9,          180,        3,
    2,          8,          64,         27,         1,          2,          0,          2,
    0,          1,          128,        128,        750,        10002,      180,        220,
    160,        240,        140,        220,        160,        200,        10,         15,
    64,         80,         4,          1,          8,          40,         32,         40,
    0,          0,          0,          0,          30,         25,         1,          1,
    200,        8,          4,          100,        40,         40,         7,          40,
    90,         0,          0,          1,          3,          100,        100,        30,
    0,          3,          30,         80,         20,         4,          7,          80,
    50,         0,          7,          17,         10,         0,          120,        120,
    2,          8,          1,          8,          8,          6,          6,          5,
    0,          1000,       3,          1,          5,          255,        -1,
};

const int32_t Cover_param_gsl6152s[] = {
    64,         80,         2,          2,          10,
};

const int32_t Base_param_gsl6152s[] = {
    16,         25600,      5,          250,        5,          8,          64,         80,
    21,         20,         1,          0,          60,         60,         20,         0,
    240,        256,        768,        256,        1024,       320,        1536,       384,
    2048,       10,         100,        0,
};

const int32_t Reduce_param_gsl6152s[] = {
    0,          0,          1,          17,         0,          40,         80,         0,
};

const cf_dev_ver_t dev_ver_gsl6152s[] = {
    {0x6152A000, 0x0516B010, 0x00000000},
};

cf_set_t cfg_gsl6152s = {
    .dev = {
        (cf_dev_ver_t *)dev_ver_gsl6152s,
        ARRAY_SIZE(dev_ver_gsl6152s),
        0,
    },
    .mask = {0xFFFFFF00, 0xFFFFFF10, 0xFFFFFFFF},
    .common = {
        .id = 0x00000000,
        .sid = 0x00000000,
        .vid = 0x00000000,
        .w = 64,
        .h = 80,
        .wp = 32,
        .hp = 32,
        .w_hwagc = 32,
        .h_hwagc = 20,
        .rw = 0,
        .wdpi = 508,
        .hdpi = 508,
        .fg_loop = 0,
        .gain = {0x0003009C, 0x00020201, 0x00000203, 0x00000000, 0},
        .gain_reg = {0xFF08000C, 0xFF000020, 0xFF00002C, 0xFF000024, 0},
    },
    .spi = {
        .ms_frm = 5,
        .retry = 100,
        .reinit = 0,
    },
    .nav = {
        .enable = 1,
        .mode = 0,
        .type = 0,
        .con_frame_get_num = 1,
        .w = 32,
        .h = 40,
        .wh = 32,
        .hh = 32,
        .w_ds = 32,
        .h_ds = 40,
        .w_hg_ds = 0,
        .h_hg_ds = 0,
        .gain = {
            {0x000300A5, 0x00010201, 0x00000103, 0x00000000, 0},
            {0x00030085, 0x00060301, 0x00000303, 0x00000000, 0},
            {0x000386A5, 0x00010201, 0x00000103, 0x63330000, 0},
            {0x00038690, 0x00010201, 0x00000103, 0x63330000, 0},
        },
        .vk_timeout = 350,
        .dclick_gap = 250,
        .longpress = 400,
    },
    .pb = {
        .param = {
            {
                (int32_t *)FineTune_param_gsl6152s,
                ARRAY_SIZE(FineTune_param_gsl6152s),
                0,
            },
            {
                (int32_t *)Navi_param_gsl6152s,
                ARRAY_SIZE(Navi_param_gsl6152s),
                0,
            },
            {
                (int32_t *)Cover_param_gsl6152s,
                ARRAY_SIZE(Cover_param_gsl6152s),
                0,
            },
            {
                (int32_t *)Base_param_gsl6152s,
                ARRAY_SIZE(Base_param_gsl6152s),
                0,
            },
            {
                (int32_t *)Reduce_param_gsl6152s,
                ARRAY_SIZE(Reduce_param_gsl6152s),
                0,
            },
        },
        .agc = {
            .skip_fd = 1,
            .fd_threshold = 128,
            .skip_small = 0,
            .max = 5,
            .max_small = 9,
            .hwagc_enable = 1,
            .hwcov_wake = 12,
            .hwcov_tune = 45,
            .exp_size = 8,
        },
        .threshold = {
            .alg_select = 3,
            .enrolNum = 14,
            .max_templates_num = 54,
            .templates_size = 270000,
            .identify_far_threshold = 17,
            .update_far_threshold = 23,
            .enroll_quality_threshold = 30,
            .enroll_coverage_threshold = 75,
            .quality_threshold = 30,
            .coverage_threshold = 75,
            .skin_threshold = 3000,
            .artificial_threshold = 2525,
            .samearea_detect = 3,
            .samearea_threshold = 17,
            .samearea_dist = 43,
            .samearea_start = 10,
            .samearea_check_once_num = 1,
            .samearea_check_num_total = 6,
            .dy_fast = 4,
            .segment = 0,
            .water_finger_detect = 0,
            .shake_coe = 10,
            .noise_coe = 140,
            .gray_prec = 25,
            .water_detect_threshold = 1500,
        },
    },
    .test = {
        .fd_threshold = 180,
        .deadpx_hard_threshold = 140,
        .deadpx_norm_threshold = 10,
        .scut = 0,
        .detev_ww = 40,
        .detev_hh = 28,
        .deteline_h = 255,
        .deteline_w = 255,
        .deadpx_max = 10,
        .badline_max = 1,
        .finger_detect_mode = 0,
        .deadpx_cut = 0x00000000,
    },
    .mmi = {
        .dac_min = 0x00000070,
        .dac_max = 0x000000FF,
        .grey_range_left = 90,
        .grey_range_right = 110,
        .max_tune_time = 7,
        .nav_base_frame_num = 0,
        .auto_adjust_w = 0,
        .auto_adjust_h = 0,
    },
    .esd = {
        .irq_check = 1,
        .irq_reg = 0x0000009A,
        .irq_val = 0x00000064,
        .int_reg = 0xFF0800D4,
        .int_val = 0xFFFF010A,
        .int_beacon = 0xFF080030,
    },
    .cfg = {
        ADD_CFG(normal_gsl6152s),
        ADD_CFG(fg_detect_gsl6152s),
        ADD_CFG(partial_gsl6152s),
        ADD_CFG(full_gsl6152s),
        ADD_CFG(frm_start_gsl6152s),
        ADD_CFG(ds_start_gsl6152s),
        ADD_CFG(stop_gsl6152s),
        ADD_CFG(downint_gsl6152s),
        ADD_CFG(vkeyint_gsl6152s),
        ADD_CFG(upint_gsl6152s),
        ADD_CFG(nav_gsl6152s),
        ADD_CFG(nav_hg_gsl6152s),
        ADD_CFG(nav_ds_gsl6152s),
        ADD_CFG(nav_hg_ds_gsl6152s),
        ADD_CFG(snr_gsl6152s),
        ADD_CFG(deadpix1_gsl6152s),
        ADD_CFG(deadpix2_gsl6152s),
        ADD_CFG(deadpix3_gsl6152s),
        ADD_CFG(hg_coverage_gsl6152s),
        ADD_CFG(hwagc_read_gsl6152s),
        ADD_CFG(hwcov_start_gsl6152s),
        ADD_CFG(hwagc_start_gsl6152s),
        ADD_CFG(base_ori_gsl6152s),
        ADD_CFG(auto_adjust_gsl6152s),
    },
};

#endif /* __SILEAD_FP_GSL6152S_H__ */
