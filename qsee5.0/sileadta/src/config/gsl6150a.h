/******************************************************************************
 * @file   gsl6150a.h
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

#ifndef __SILEAD_FP_GSL6150A_H__
#define __SILEAD_FP_GSL6150A_H__

#include "silead_config.h"

const cf_register_t reg_normal_gsl6150a[] = {
    {0x00000098, 0x20044710},
    {0x000000E4, 0x00000005},
    {0x000000E8, 0x00000043},
    {0xFF010008, 0x00000039},
    {0xFF000004, 0x00000003},
    {0xFF000034, 0x00020206},
    {0xFF00004C, 0x00001A03},
    {0xFF000064, 0x00111311},
    {0xFF00007C, 0x0120012D},
    {0x01000400, 0x00000100},
    {0x01000404, 0x00020003},
    {0x01000408, 0x000C000D},
    {0x0100040C, 0x00E7000D},
    {0x01000410, 0x00FD0003},
    {0xFF0800AC, 0x001A006F},
    {0xFF0800B0, 0x00370050},
    {0xFF080044, 0x10060006},
    {0xFF0800C8, 0x005E0043},
    {0xFF0800B4, 0x00150005},
    {0xFF0800B8, 0x00370027},
    {0xFF0800BC, 0x00240004},
    {0xFF0800D0, 0x0010009F},
    {0xFF0800E4, 0x2002C008},
    {0xFF080030, 0x003D0003},
    {0xFF08015C, 0x105E0043},
    {0xFF08084C, 0x00000010},
    {0xFF080000, 0x20226242},
    {0xFF08007C, 0x008000C0},
    {0xFF080200, 0x38495039},
    {0xFF080204, 0x38495039},
    {0xFF080208, 0x38495039},
    {0xFF08020C, 0x38495039},
    {0xFF080210, 0x38495039},
    {0xFF000020, 0x00010201},
    {0xFF00002C, 0x00000205},
    {0xFF08000C, 0x0012729C},
};

const cf_register_t reg_fg_detect_gsl6150a[] = {
    {0xFF000020, 0x00010201},
    {0xFF00002C, 0x00000205},
    {0xFF08000C, 0x00127272},
};

const cf_register_t reg_partial_gsl6150a[] = {
    {0xFF0A0010, 0x00000001},
    {0xFF080054, 0x00000100},
    {0xFF080150, 0x00380005},
    {0xFF080154, 0x00190002},
};

const cf_register_t reg_full_gsl6150a[] = {
    {0xFF0A0010, 0x00000001},
    {0xFF080054, 0x00000500},
    {0xFF080150, 0x00500007},
    {0xFF080154, 0x00010000},
};

const cf_register_t reg_frm_start_gsl6150a[] = {
    {0xFF080024, 0x00000000},
};

const cf_register_t reg_ds_start_gsl6150a[] = {
    {0xFF080024, 0x03000000},
};

const cf_register_t reg_stop_gsl6150a[] = {
};

const cf_register_t reg_downint_gsl6150a[] = {
    {0x000000BF, 0x00000000},
    {0xFF080034, 0x10001000},
    {0xFF080038, 0x0F000020},
    {0xFF080124, 0x00000018},
    {0xFF08012C, 0x00000180},
    {0xFF080160, 0x2A010501},
    {0xFF010004, 0x00000A80},
    {0xFF000020, 0x00010201},
    {0xFF00002C, 0x00000205},
    {0xFF08000C, 0x00129097},
    {0xFF000080, 0x00000701},
    {0x00000090, 0x0000800B},
    {0xFF080030, 0x011C0001},
};

const cf_register_t reg_vkeyint_gsl6150a[] = {
    {0x000000BF, 0x00000000},
    {0xFF080034, 0x10001000},
    {0xFF080038, 0x0F000020},
    {0xFF080124, 0x00000018},
    {0xFF08012C, 0x00000180},
    {0xFF080160, 0x2A010501},
    {0xFF010004, 0x00000A80},
    {0xFF000020, 0x00010201},
    {0xFF00002C, 0x00000205},
    {0xFF08000C, 0x00129097},
    {0xFF000080, 0x00000701},
    {0x00000090, 0x0000800B},
    {0xFF080030, 0x011C0001},
};

const cf_register_t reg_upint_gsl6150a[] = {
    {0x000000BF, 0x00000000},
    {0xFF080034, 0x12001000},
    {0xFF080038, 0x0F000020},
    {0xFF080124, 0x00000018},
    {0xFF08012C, 0x00000180},
    {0xFF080160, 0x2A010501},
    {0xFF010004, 0x00000A80},
    {0xFF000020, 0x00010201},
    {0xFF00002C, 0x00000205},
    {0xFF08000C, 0x00129090},
    {0xFF000080, 0x00000701},
    {0x00000090, 0x0000800B},
    {0xFF080030, 0x011C0001},
};

const cf_register_t reg_nav_gsl6150a[] = {
    {0xFF0A0010, 0x00000001},
    {0xFF080150, 0x00500007},
    {0xFF080154, 0x00010000},
    {0xFF080054, 0x00000140},
    {0xFF080814, 0x00000000},
};

const cf_register_t reg_nav_hg_gsl6150a[] = {
    {0xFF0A0010, 0x00000001},
    {0xFF080150, 0x00480007},
    {0xFF080154, 0x00090000},
    {0xFF080054, 0x00000100},
    {0xFF080814, 0x00000000},
};

const cf_register_t reg_nav_ds_gsl6150a[] = {
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
    {0xFF08000C, 0x000385A5},
};

const cf_register_t reg_nav_hg_ds_gsl6150a[] = {
};

const cf_register_t reg_snr_gsl6150a[] = {
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

const cf_register_t reg_deadpix1_gsl6150a[] = {
    {0xFF000020, 0x00010101},
    {0xFF00002C, 0x00000101},
    {0xFF08000C, 0x00127272},
};

const cf_register_t reg_deadpix2_gsl6150a[] = {
    {0xFF000020, 0x00010101},
    {0xFF00002C, 0x00000101},
    {0xFF08000C, 0x00127272},
};

const cf_register_t reg_deadpix3_gsl6150a[] = {
    {0xFF000020, 0x00010101},
    {0xFF00002C, 0x00000101},
    {0xFF08000C, 0x00127272},
};

const cf_register_t reg_hg_coverage_gsl6150a[] = {
    {0xFF000024, 0x14320000},
    {0xFF08000C, 0x00127272},
    {0xFF080114, 0x08080801},
    {0xFF080118, 0x0000003C},
    {0xFF08011C, 0x000000FC},
    {0xFF080850, 0x8E002000},
    {0xFF080854, 0xB0000040},
};

const cf_register_t reg_hwagc_read_gsl6150a[] = {
    {0xFF0A0010, 0x00000001},
    {0xFF080054, 0x00000500},
    {0xFF080150, 0x00500007},
    {0xFF080154, 0x00010000},
    {0xFF080840, 0x05100402},
    {0xFF080834, 0x00000006},
    {0xFF080838, 0x0000000E},
};

const cf_register_t reg_hwcov_start_gsl6150a[] = {
    {0xFF080024, 0x01000000},
};

const cf_register_t reg_hwagc_start_gsl6150a[] = {
    {0xFF080024, 0x00000001},
};

const cf_register_t reg_base_ori_gsl6150a[] = {
    {0xFF0A0010, 0x00000001},
    {0xFF080054, 0x00000500},
    {0xFF080150, 0x00500007},
    {0xFF080154, 0x00010000},
    {0xFF080814, 0xC1000100},
    {0xFF000020, 0x00010201},
    {0xFF00002C, 0x00000205},
    {0xFF08000C, 0x00129090},
};

const cf_register_t reg_auto_adjust_gsl6150a[] = {
    {0xFF080000, 0x20227242},
    {0xFF080110, 0x2A010501},
    {0xFF080100, 0x00000018},
    {0xFF080108, 0x00000000},
    {0xFF08010C, 0x00000180},
    {0xFF080054, 0x00000008},
};

const int32_t FineTune_param_gsl6150a[] = {
    0,          128,        750,        10002,      0,          128,        1,          230,
    220,        250,        180,        220,        200,        254,        150,        250,
    58,         158,        58,         158,        30,         40,         10,         10,
    15,         20,         5,          10,         10,         20,         90,         80,
    95,         90,         75,         15,         0,          254,        8,          4,
    6,          15,         30,         30519,      30263,      26167,      25911,      21815,
    21559,      17463,      17462,      17206,      13110,      13109,      12854,      12853,
    8758,       8757,       8742,       8741,       8740,       8739,       4646,       4645,
    4644,       4643,       4628,       4627,       4626,       4612,       4625,       4610,
    4609,       0,          180,        40,         24,         19,         120,        1,
    0,          0,          0,          1,          20,         0,          0,          3,
    2,          17171974,   0,          0,          0,          58,         158,        58,
    158,        0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          7,          16,
    14,         13,         11,         9,          9,          7,          6,          6,
    6,          6,          6,          7,          9,          13,         18,         16,
    0,          8,          8,          30,         10,         0,          5,          20,
    15,         1,
};

const int32_t Navi_param_gsl6150a[] = {
    0,          3,          8,          3,          8,          9,          128,        3,
    2,          8,          64,         27,         1,          2,          0,          2,
    0,          1,          32,         128,        750,        10002,      130,        150,
    120,        140,        110,        150,        120,        140,        10,         15,
    64,         80,         4,          1,          8,          40,         32,         40,
    0,          0,          0,          0,          30,         25,         1,          1,
    200,        8,          4,          100,        60,         40,         7,          40,
    90,         0,          0,          1,          3,          100,        100,        30,
    0,          3,          30,         60,         40,         4,          7,          80,
    50,         0,          7,          17,         10,         0,          120,        120,
    2,          8,          1,          8,          8,          6,          6,          5,
    0,          1000,       3,          1,          5,          255,        -1,         15,
    2,          60,         20,         20,
};

const int32_t Cover_param_gsl6150a[] = {
    64,         80,         2,          2,          10,
};

const int32_t Base_param_gsl6150a[] = {
    16,         25600,      5,          250,        5,          8,          64,         80,
    20,         20,         1,          0,          60,         60,         20,         0,
    240,        256,        768,        256,        1024,       320,        1536,       384,
    2048,       10,         100,        1,          32,         40,
};

const int32_t Reduce_param_gsl6150a[] = {
    32,         0,          1,          24,         0,          40,         0,          1,
};

const cf_dev_ver_t dev_ver_gsl6150a[] = {
    {0x6152B002, 0x6152B002, 0x00000000},
};

cf_set_t cfg_gsl6150a = {
    .dev = {
        (cf_dev_ver_t *)dev_ver_gsl6150a,
        ARRAY_SIZE(dev_ver_gsl6150a),
        0,
    },
    .mask = {0xFFFFFFFF, 0xFFFFFFF1, 0xFFFFFFFF},
    .common = {
        .id = 0x00000000,
        .sid = 0x00000000,
        .vid = 0x00000000,
        .w = 64,
        .h = 80,
        .wp = 32,
        .hp = 32,
        .w_hwagc = 16,
        .h_hwagc = 16,
        .rw = 0,
        .wdpi = 508,
        .hdpi = 508,
        .fg_loop = 1,
        .gain = {0x0012009C, 0x00010201, 0x00000205, 0x00000000, 0},
        .gain_reg = {0xFF08000C, 0xFF000020, 0xFF00002C, 0xFF000024, 0},
    },
    .spi = {
        .ms_frm = 5,
        .retry = 100,
        .reinit = 0,
    },
    .nav = {
        .enable = 0,
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
            {0x0012009C, 0x00010201, 0x00000205, 0x00000000, 0},
            {0x00127272, 0x00060301, 0x00000303, 0x00000000, 0},
            {0x0012909C, 0x00010201, 0x00000205, 0x14320000, 0},
            {0x0012909C, 0x00010201, 0x00000205, 0x14320000, 0},
        },
        .vk_timeout = 350,
        .dclick_gap = 250,
        .longpress = 400,
    },
    .pb = {
        .param = {
            {
                (int32_t *)FineTune_param_gsl6150a,
                ARRAY_SIZE(FineTune_param_gsl6150a),
                0,
            },
            {
                (int32_t *)Navi_param_gsl6150a,
                ARRAY_SIZE(Navi_param_gsl6150a),
                0,
            },
            {
                (int32_t *)Cover_param_gsl6150a,
                ARRAY_SIZE(Cover_param_gsl6150a),
                0,
            },
            {
                (int32_t *)Base_param_gsl6150a,
                ARRAY_SIZE(Base_param_gsl6150a),
                0,
            },
            {
                (int32_t *)Reduce_param_gsl6150a,
                ARRAY_SIZE(Reduce_param_gsl6150a),
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
            .hwcov_tune = 16,
            .exp_size = 8,
        },
        .threshold = {
            .alg_select = 3,
            .enrolNum = 12,
            .max_templates_num = 54,
            .templates_size = 270000,
            .identify_far_threshold = 17,
            .update_far_threshold = 23,
            .enroll_quality_threshold = 0,
            .enroll_coverage_threshold = 0,
            .quality_threshold = 0,
            .coverage_threshold = 0,
            .skin_threshold = 0,
            .artificial_threshold = 0,
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
        .fd_threshold = 128,
        .deadpx_hard_threshold = 140,
        .deadpx_norm_threshold = 0,
        .scut = 0,
        .detev_ww = 40,
        .detev_hh = 28,
        .deteline_h = 255,
        .deteline_w = 255,
        .deadpx_max = 10,
        .badline_max = 1,
        .finger_detect_mode = 1,
        .deadpx_cut = 0x02020202,
    },
    .mmi = {
        .dac_min = 0x00000000,
        .dac_max = 0x000000F0,
        .grey_range_left = 145,
        .grey_range_right = 155,
        .max_tune_time = 10,
        .nav_base_frame_num = 0,
        .auto_adjust_w = 16,
        .auto_adjust_h = 2,
    },
    .esd = {
        .irq_check = 1,
        .irq_reg = 0x0000009A,
        .irq_val = 0x00000004,
        .int_reg = 0xFF080160,
        .int_val = 0x2A010501,
        .int_beacon = 0xFF080030,
    },
    .cfg = {
        ADD_CFG(normal_gsl6150a),
        ADD_CFG(fg_detect_gsl6150a),
        ADD_CFG(partial_gsl6150a),
        ADD_CFG(full_gsl6150a),
        ADD_CFG(frm_start_gsl6150a),
        ADD_CFG(ds_start_gsl6150a),
        ADD_CFG(stop_gsl6150a),
        ADD_CFG(downint_gsl6150a),
        ADD_CFG(vkeyint_gsl6150a),
        ADD_CFG(upint_gsl6150a),
        ADD_CFG(nav_gsl6150a),
        ADD_CFG(nav_hg_gsl6150a),
        ADD_CFG(nav_ds_gsl6150a),
        ADD_CFG(nav_hg_ds_gsl6150a),
        ADD_CFG(snr_gsl6150a),
        ADD_CFG(deadpix1_gsl6150a),
        ADD_CFG(deadpix2_gsl6150a),
        ADD_CFG(deadpix3_gsl6150a),
        ADD_CFG(hg_coverage_gsl6150a),
        ADD_CFG(hwagc_read_gsl6150a),
        ADD_CFG(hwcov_start_gsl6150a),
        ADD_CFG(hwagc_start_gsl6150a),
        ADD_CFG(base_ori_gsl6150a),
        ADD_CFG(auto_adjust_gsl6150a),
    },
};

#endif /* __SILEAD_FP_GSL6150A_H__ */
