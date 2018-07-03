/******************************************************************************
 * @file   silead_config.c
 * @brief  Contains Chip config files operate functions.
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
 * Martin Wu  2018/4/2    0.1.0      Init version
 * Martin Wu  2018/5/7    0.1.1      Support 6150a related configurations.
 * Martin Wu  2018/5/9    0.1.2      Add 6150 deadpix test param
 * Martin Wu  2018/5/10   0.1.3      Add 6150 nav base param
 * Martin Wu  2018/5/11   0.1.4      Add 6150 auto adjust param
 * John Zhang 2018/5/12   0.1.5      Add 6150b config file
 * Martin Wu  2018/5/14   0.1.6      Add finger detect loop mode
 * Martin Wu  2018/6/8    0.1.6      Add ESD related param
 * Martin Wu  2018/6/16   0.1.7      Add OTP related param
 * Martin Wu  2018/6/18   0.1.8      Add cut_size param
 * Martin Wu  2018/6/25   0.1.9      Add optical base & SNR param
 * Martin Wu  2018/6/25   0.2.0      Add optical factory test param
 * Martin Wu  2018/6/26   0.2.1      Add optical middle tone base param
 * Martin Wu  2018/6/27   0.2.2      Add SPI check 0xBF front porch.
 * Martin Wu  2018/6/30   0.2.3      Add distortion & finger_num param.
 *
 *****************************************************************************/

#define FILE_TAG "silead_config"
#define LOG_DBG_VERBOSE 0
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>

#include "silead_config.h"

cf_set_t * silfp_cfg_init(int32_t __unused fd)
{
    LOG_MSG_VERBOSE("init");

    return NULL;
}

typedef struct __attribute__ ((packed)) _sl_data_upd {
    uint32_t len;
    uint32_t updated;
} cf_data_upd_t;

typedef struct __attribute__ ((packed)) _sl_pb_upd {
    cf_pb_agc_t agc;
    cf_algo_param_t threshold;
} cf_pb_upd;

typedef struct __attribute__ ((packed)) _sl_upd_set {
    cf_common_t common;
    cf_spi_t spi;
    cf_nav_t nav;
    cf_pb_upd pb;
    cf_test_t test;
    cf_mmi_t mmi;
    cf_ft_t ft;
    cf_esd_t esd;
    cf_data_upd_t cfg[CFG_MAX];
    cf_data_upd_t param[CFG_PB_PARAM_MAX];
    uint32_t updated;
} cf_upd_set_t;

int32_t silfp_cfg_update(cf_set_t *pcfgs, const e_mode_config_t type, const cf_register_t *preg, const uint32_t len)
{
    if (pcfgs == NULL || type >= CFG_MAX || preg == NULL) {
        return 0;
    }

    if (pcfgs->cfg[type].updated) {
        free(pcfgs->cfg[type].reg);
    }

    pcfgs->cfg[type].reg = (cf_register_t *)preg;
    pcfgs->cfg[type].len = len;
    pcfgs->cfg[type].updated = 1;
    return pcfgs->cfg[type].updated;
}

int32_t silfp_cfg_update_ex(cf_set_t *pcfgs, const e_mode_config_t type, const uint32_t addr, const uint32_t val)
{
    cf_register_t *preg = NULL;
    uint32_t size;

    if (pcfgs == NULL || type >= CFG_MAX ) {
        return -1;
    }

    if (!pcfgs->cfg[type].len || !pcfgs->cfg[type].reg) {
        // Empty, no update needed;
        return 0;
    }

    if (!pcfgs->cfg[type].updated) {
        size = pcfgs->cfg[type].len * sizeof(cf_register_t);
        preg = (cf_register_t*)malloc(size);
        if (preg != NULL) {
            memcpy((char *)preg, (char *)pcfgs->cfg[type].reg, size);
            pcfgs->cfg[type].reg = preg;
            pcfgs->cfg[type].updated = 1;
        } else {
            LOG_MSG_ERROR("cfg_update_ex malloc fail");
            return -1;
        }
    }

    for (size = 0; size < pcfgs->cfg[type].len; size++) {
        if (pcfgs->cfg[type].reg[size].addr == addr) {
            pcfgs->cfg[type].reg[size].val = val;
            break;
        }
    }

    return 0;
}

int32_t silfp_cfg_pb_param_update(cf_set_t *pcfgs, const e_pb_param_t type, const int32_t *val, const uint32_t len)
{
    if (pcfgs == NULL || type >= CFG_PB_PARAM_MAX || val == NULL) {
        return 0;
    }

    if (pcfgs->pb.param[type].updated) {
        free(pcfgs->pb.param[type].val);
    }

    pcfgs->pb.param[type].val = (int32_t *)val;
    pcfgs->pb.param[type].len = len;
    pcfgs->pb.param[type].updated = 1;
    return pcfgs->pb.param[type].updated;
}

int32_t silfp_cfg_dev_ver_update(cf_set_t *pcfgs, const cf_dev_ver_t *ver, const uint32_t len)
{
    if (pcfgs == NULL || ver == NULL) {
        return 0;
    }

    if (pcfgs->dev.updated) {
        free(pcfgs->dev.ver);
    }
    pcfgs->dev.ver = (cf_dev_ver_t *)ver;
    pcfgs->dev.len = len;
    pcfgs->dev.updated = 1;

    return pcfgs->dev.updated;
}

cf_set_t * silfp_cfg_malloc(void)
{
    cf_set_t *item = (cf_set_t *)malloc(sizeof(cf_set_t));
    if (item != NULL) {
        memset(item, 0, sizeof(cf_set_t));
    }
    return item;
}

void silfp_cfg_free(cf_set_t *pcfgs)
{
    silfp_cfg_deinit(pcfgs);
    free(pcfgs);
}

void silfp_cfg_deinit(cf_set_t *pcfgs)
{
    int32_t i = 0;

    if (pcfgs != NULL) {
        for (i = 0; i < CFG_MAX; i++) {
            if (pcfgs->cfg[i].updated) {
                free(pcfgs->cfg[i].reg);
                pcfgs->cfg[i].reg = NULL;
                pcfgs->cfg[i].len = 0;
                pcfgs->cfg[i].updated = 0;

                LOG_MSG_VERBOSE("release cfg %d", i);
            }
        }

        for (i = 0; i < CFG_PB_PARAM_MAX; i++) {
            if (pcfgs->pb.param[i].updated) {
                free(pcfgs->pb.param[i].val);
                pcfgs->pb.param[i].val = NULL;
                pcfgs->pb.param[i].len = 0;
                pcfgs->pb.param[i].updated = 0;

                LOG_MSG_VERBOSE("release pb param cfg %d", i);
            }
        }

        if (pcfgs->dev.updated) {
            free(pcfgs->dev.ver);
            pcfgs->dev.ver = NULL;
            pcfgs->dev.len = 0;
            pcfgs->dev.updated = 0;
        }
    }
    LOG_MSG_VERBOSE("deinit");
}

const char * silfp_cfg_get_config_name(const uint32_t idx)
{
    const char *name[] = {
        "normal",
        "fg_detect",
        "partial",
        "full",
        "frm_start",
        "ds_start",
        "stop",
        "downint",
        "vkeyint",
        "upint",
        "nav",
        "nav_hg",
        "nav_ds",
        "nav_hg_ds",
        "snr",
        "deadpix1",
        "deadpix2",
        "deadpix3",
        "hg_coverage",
        "hwagc_read",
        "hwcov_start",
        "hwagc_start",
        "base_ori",
        "auto_adjust",
    };

    if (idx < CFG_MAX) {
        return name[idx];
    }
    return "unknow";
}

int32_t silfp_cfg_get_update_length(cf_set_t *pcfgs)
{
    int32_t len = 0;
    int32_t i;

    if (pcfgs != NULL) {
        len += sizeof(cf_upd_set_t);

        for (i = 0; i < CFG_MAX; i++) {
            if (pcfgs->cfg[i].updated && pcfgs->cfg[i].len > 0) {
                len += pcfgs->cfg[i].len * sizeof(cf_register_t);
            }
        }

        for (i = 0; i < CFG_PB_PARAM_MAX; i++) {
            if (pcfgs->pb.param[i].updated && pcfgs->pb.param[i].len > 0) {
                len += pcfgs->pb.param[i].len * sizeof(int32_t);
            }
        }
    }

    LOG_MSG_VERBOSE("len = %d", len);
    return len;
}

int32_t silfp_cfg_get_update_buffer(void *buffer, const uint32_t len, const cf_set_t *pcfgs)
{
    uint32_t offset = 0;
    uint32_t size = 0;
    cf_upd_set_t *pcfg_upd;
    int32_t i;

    if (buffer == NULL || pcfgs == NULL || len < sizeof(cf_upd_set_t)) {
        return 0;
    }

    size = sizeof(cf_upd_set_t);

    pcfg_upd = (cf_upd_set_t *)buffer;
    memcpy(&pcfg_upd->common, &pcfgs->common, sizeof(pcfg_upd->common));
    memcpy(&pcfg_upd->spi, &pcfgs->spi, sizeof(pcfg_upd->spi));
    memcpy(&pcfg_upd->nav, &pcfgs->nav, sizeof(pcfg_upd->nav));
    memcpy(&pcfg_upd->pb.agc, &pcfgs->pb.agc, sizeof(pcfg_upd->pb.agc));
    memcpy(&pcfg_upd->pb.threshold, &pcfgs->pb.threshold, sizeof(pcfg_upd->pb.threshold));
    memcpy(&pcfg_upd->test, &pcfgs->test, sizeof(pcfg_upd->test));
    memcpy(&pcfg_upd->mmi, &pcfgs->mmi, sizeof(pcfg_upd->mmi));
    memcpy(&pcfg_upd->ft, &pcfgs->ft, sizeof(pcfg_upd->ft));
    memcpy(&pcfg_upd->esd, &pcfgs->esd, sizeof(pcfg_upd->esd));

    offset += size;

    LOG_MSG_VERBOSE("common.updated = %d", pcfgs->common.updated);
    LOG_MSG_VERBOSE("common.gain.updated = %d", pcfgs->common.gain.updated);
    LOG_MSG_VERBOSE("spi.updated = %d", pcfgs->spi.updated);
    LOG_MSG_VERBOSE("nav.updated = %d", pcfgs->nav.updated);
    for (i = 0; i < CFG_NAV_AGC_MODE_MAX; i++) {
        LOG_MSG_VERBOSE("nav.gain[%d].updated = %d", i, pcfgs->nav.gain[i].updated);
    }
    LOG_MSG_VERBOSE("test.updated = %d", pcfgs->test.updated);
    LOG_MSG_VERBOSE("mmi.updated = %d", pcfgs->mmi.updated);
    LOG_MSG_VERBOSE("ft.updated = %d", pcfgs->ft.updated);
    LOG_MSG_VERBOSE("esd.updated = %d", pcfgs->esd.updated);
    LOG_MSG_VERBOSE("pb.agc.updated = %d", pcfgs->pb.agc.updated);
    LOG_MSG_VERBOSE("pb.threshold.updated = %d", pcfgs->pb.threshold.updated);

    for (i = 0; i < CFG_MAX; i++) {
        pcfg_upd->cfg[i].updated = pcfgs->cfg[i].updated;
        pcfg_upd->cfg[i].len = pcfgs->cfg[i].len;

        if (pcfgs->cfg[i].updated && pcfgs->cfg[i].len > 0) {
            size = pcfgs->cfg[i].len * sizeof(cf_register_t);
            if (offset + size <= len) {
                memcpy((char *)buffer + offset, pcfgs->cfg[i].reg, size);
                offset += size;
            }
        }
    }

    for (i = 0; i < CFG_PB_PARAM_MAX; i++) {
        pcfg_upd->param[i].updated = pcfgs->pb.param[i].updated;
        pcfg_upd->param[i].len = pcfgs->pb.param[i].len;

        if (pcfgs->pb.param[i].updated && pcfgs->pb.param[i].len > 0) {
            size = pcfgs->pb.param[i].len * sizeof(int32_t);
            if (offset + size <= len) {
                memcpy((char *)buffer + offset, pcfgs->pb.param[i].val, size);
                offset += size;
            }
        }
    }

    LOG_MSG_VERBOSE("offset = %d", offset);
    return offset;
}

static int32_t _cfg_get_update_length(cf_upd_set_t *pcfgs)
{
    int32_t len = 0;
    int32_t i;

    if (pcfgs != NULL) {
        len += sizeof(cf_upd_set_t);

        for (i = 0; i < CFG_MAX; i++) {
            if (pcfgs->cfg[i].updated && pcfgs->cfg[i].len > 0) {
                len += pcfgs->cfg[i].len * sizeof(cf_register_t);
            }
        }

        for (i = 0; i < CFG_PB_PARAM_MAX; i++) {
            if (pcfgs->param[i].updated && pcfgs->param[i].len > 0) {
                len += pcfgs->param[i].len * sizeof(int32_t);
            }
        }
    }

    LOG_MSG_VERBOSE("len = %d", len);
    return len;
}

int32_t silfp_cfg_update_config(const void *buffer, const uint32_t len, cf_set_t *psyscfgs)
{
    uint32_t offset = 0;
    uint32_t size = 0;
    int32_t i;
    cf_upd_set_t *pcfg_upd;

    if (buffer == NULL || psyscfgs == NULL || len < sizeof(cf_upd_set_t)) {
        return 0;
    }

    pcfg_upd = (cf_upd_set_t *)buffer;
    if (pcfg_upd->common.id != psyscfgs->common.id || pcfg_upd->common.sid != psyscfgs->common.sid || pcfg_upd->common.vid != psyscfgs->common.vid) {
        return 0;
    }

    if (_cfg_get_update_length(pcfg_upd) != (int32_t)len) {
        return 0;
    }

    // commom update
    LOG_MSG_VERBOSE("common.updated = %d", pcfg_upd->common.updated);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, common, w);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, common, h);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, common, wp);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, common, hp);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, common, w_hwagc);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, common, h_hwagc);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, common, rw);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, common, wdpi);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, common, hdpi);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, common, fg_loop);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, common, wc);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, common, hc);

    // commom.gain update
    LOG_MSG_VERBOSE("common.gain.updated = %d", pcfg_upd->common.gain.updated);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, gain, v0c);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, gain, v20);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, gain, v2c);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, gain, v24);

    // commom.gain_reg update
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, gain_reg, reg0c);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, gain_reg, reg20);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, gain_reg, reg2c);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, gain_reg, reg24);

    // commom.otp update
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, otp, otp0);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, otp, otp1);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, otp, otp2);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, otp, otp3);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, otp, otp4);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, otp, otp5);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, common, otp, otp_a0);

    // spi update
    LOG_MSG_VERBOSE("spi.updated = %d", pcfg_upd->spi.updated);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, spi, ms_frm);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, spi, retry);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, spi, reinit);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, spi, start);

    // nav update
    LOG_MSG_VERBOSE("nav.updated = %d", pcfg_upd->nav.updated);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, enable);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, mode);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, type);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, con_frame_get_num);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, w);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, h);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, wh);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, hh);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, w_ds);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, h_ds);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, w_hg_ds);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, h_hg_ds);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, vk_timeout);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, dclick_gap);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, nav, longpress);

    // nav.gain update
    for (i = 0; i < CFG_NAV_AGC_MODE_MAX; i++) {
        LOG_MSG_VERBOSE("nav.gain[%d].updated = %d", i, pcfg_upd->nav.gain[i].updated);
        GET_UPD_GAIN_ITEM_VALUE_3(psyscfgs, pcfg_upd, nav, gain, i, v0c);
        GET_UPD_GAIN_ITEM_VALUE_3(psyscfgs, pcfg_upd, nav, gain, i, v20);
        GET_UPD_GAIN_ITEM_VALUE_3(psyscfgs, pcfg_upd, nav, gain, i, v2c);
        GET_UPD_GAIN_ITEM_VALUE_3(psyscfgs, pcfg_upd, nav, gain, i, v24);
    }

    // test update
    LOG_MSG_VERBOSE("test.updated = %d", pcfg_upd->test.updated);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, test, fd_threshold);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, test, deadpx_hard_threshold);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, test, deadpx_norm_threshold);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, test, scut);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, test, detev_ww);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, test, detev_hh);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, test, deteline_h);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, test, deteline_w);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, test, deadpx_max);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, test, badline_max);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, test, finger_detect_mode);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, test, deadpx_cut);

    // mmi update
    LOG_MSG_VERBOSE("mmi.updated = %d", pcfg_upd->mmi.updated);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, dac_min);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, dac_max);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, grey_range_left);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, grey_range_right);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, nav_base_frame_num);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, max_tune_time);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, auto_adjust_w);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, auto_adjust_h);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, frm_loop_max);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, postprocess_ctl);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, white_base_white_thr);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, white_base_black_thr);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, black_base_white_thr);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, black_base_black_thr);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, middle_base_white_thr);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, middle_base_black_thr);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, diff_base_min_thr);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, diff_base_max_thr);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, snr_cut);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, base_size);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, snr_img_num);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, snr_thr);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, distortion);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, finger_num);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, mmi, sum_type);

    // ft update
    LOG_MSG_VERBOSE("ft.updated = %d", pcfg_upd->ft.updated);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, ft, line_step_min);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, ft, ignore);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, ft, min_theta);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, ft, max_theta);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, ft, quality_thr);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, ft, line_distance_min);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, ft, line_distance_max);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, ft, cut);

    // esd update
    LOG_MSG_VERBOSE("esd.updated = %d", pcfg_upd->esd.updated);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, esd, irq_check);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, esd, irq_reg);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, esd, irq_val);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, esd, int_reg);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, esd, int_val);
    GET_UPD_VALUE_2(psyscfgs, pcfg_upd, esd, int_beacon);

    // pb agc update
    LOG_MSG_VERBOSE("pb.agc.updated = %d", pcfg_upd->pb.agc.updated);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, agc, skip_fd);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, agc, fd_threshold);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, agc, skip_small);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, agc, max);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, agc, max_small);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, agc, hwagc_enable);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, agc, hwcov_wake);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, agc, hwcov_tune);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, agc, exp_size);

    // pb threshold update
    LOG_MSG_VERBOSE("pb.threshold.updated = %d", pcfg_upd->pb.threshold.updated);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, alg_select);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, enrolNum);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, max_templates_num);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, templates_size);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, identify_far_threshold);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, update_far_threshold);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, enroll_quality_threshold);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, enroll_coverage_threshold);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, quality_threshold);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, coverage_threshold);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, skin_threshold);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, artificial_threshold);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, samearea_detect);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, samearea_threshold);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, samearea_dist);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, samearea_start);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, samearea_check_once_num);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, samearea_check_num_total);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, dy_fast);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, segment);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, water_finger_detect);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, shake_coe);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, noise_coe);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, gray_prec);
    GET_UPD_VALUE_3(psyscfgs, pcfg_upd, pb, threshold, water_detect_threshold);

    offset += sizeof(cf_upd_set_t);

    for (i = 0; i < CFG_MAX; i++) {
        if (pcfg_upd->cfg[i].updated && pcfg_upd->cfg[i].len > 0) {
            size = pcfg_upd->cfg[i].len * sizeof(cf_register_t);
            cf_register_t *preg = (cf_register_t*)malloc(size);
            if (preg != NULL) {
                memcpy((char *)preg, (char *)buffer + offset, size);
                silfp_cfg_update(psyscfgs, (e_mode_config_t)i, preg, pcfg_upd->cfg[i].len);
            }
            offset += size;
        }
    }

    for (i = 0; i < CFG_PB_PARAM_MAX; i++) {
        if (pcfg_upd->param[i].updated && pcfg_upd->param[i].len > 0) {
            size = pcfg_upd->param[i].len * sizeof(int32_t);
            int32_t *params = (int32_t*)malloc(size);
            if (params != NULL) {
                memcpy((char *)params, (char *)buffer + offset, size);
                silfp_cfg_pb_param_update(psyscfgs, (e_pb_param_t)i, params, pcfg_upd->param[i].len);
            }
            offset += size;
        }
    }

    return offset;
}
