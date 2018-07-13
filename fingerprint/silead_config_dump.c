/******************************************************************************
 * @file   silead_config_dump.c
 * @brief  Contains Chip config files dump to .h files.
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
 * Martin Wu  2018/5/7    0.1.1      Support 6150 related configurations.
 * Martin Wu  2018/5/9    0.1.2      Add 6150 deadpix test param
 * Martin Wu  2018/5/10   0.1.3      Add 6150 nav base param
 * Martin Wu  2018/5/11   0.1.4      Add 6150 auto adjust param
 * Martin Wu  2018/5/14   0.1.5      Add finger detect loop mode
 * John Zhang 2018/5/15   0.1.6      Dump config.h file remove line-end space.
 * Martin Wu  2018/6/8    0.1.7      Add ESD related param
 * Martin Wu  2018/6/16   0.1.8      Add OTP related param
 * Martin Wu  2018/6/18   0.1.9      Add cut_size param
 * Martin Wu  2018/6/25   0.2.0      Add optical base & SNR param
 * Martin Wu  2018/6/25   0.2.1      Add optical factory test param
 * Martin Wu  2018/6/26   0.2.2      Add optical middle tone base param
 * Martin Wu  2018/6/27   0.2.3      Add SPI check 0xBF front porch.
 * Martin Wu  2018/6/30   0.2.4      Add distortion & finger_num param.
 * Martin Wu  2018/7/4    0.2.5      Add AEC param.
 * Martin Wu  2018/7/6    0.2.6      Add dead pixel radius.
 *
 *****************************************************************************/

#define FILE_TAG "silead_config"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "silead_config.h"

#define FULLNAME_MAX   255
#define FP_CONFIG_INCLUDE_PATH "/data/include"
#define FP_CONFIG_MODULE_DEFAULT "xxxx"
#define FP_CONFIG_MODULE_NAME_OBLITE "board_for_"

#define CONFIG_SET_NAME "cfg"
#define FP_DEV_VER_NAME "dev_ver"
static const char *param_name[] = {"FineTune_param", "Navi_param", "Cover_param", "Base_param", "Reduce_param"};

#define STR_TO_UPPER(s, l) \
    do { \
        uint32_t i; \
        for (i = 0; i < l; i++) \
        { \
            if (s[i] >= 'a' && s[i] <= 'z') { \
                s[i] += ('A' - 'a'); \
            } \
        } \
    } while (0)

#define DUMP_INCLUDE_BEGIN(m) \
    do { \
        char module_upper[512]; \
        snprintf(module_upper, sizeof(module_upper), "%s", m); \
        STR_TO_UPPER(module_upper, strlen(module_upper)); \
        snprintf(buffer, sizeof(buffer), "#ifndef __SILEAD_FP_%s_H__\n", module_upper); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "#define __SILEAD_FP_%s_H__\n", module_upper); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "\n#include \"silead_config.h\"\n"); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_INCLUDE_END(m) \
    do { \
        char module_upper[512]; \
        snprintf(module_upper, sizeof(module_upper), "%s", m); \
        STR_TO_UPPER(module_upper, strlen(module_upper)); \
        snprintf(buffer, sizeof(buffer), "\n#endif /* __SILEAD_FP_%s_H__ */\n", module_upper); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_HEX_VALUE_2(n, a, b) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s{0x%08X, 0x%08X},\n", n, " ", a, b); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_HEX_VALUE_3(n, a, b, c) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s{0x%08X, 0x%08X, 0x%08X},\n", n, " ", a, b, c); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_BEGIN(t, name, m) \
    do { \
        snprintf(buffer, sizeof(buffer), "\n%s %s_%s = {\n", #t, name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ARRAY_BEGIN(t, name, m) \
    do { \
        snprintf(buffer, sizeof(buffer), "\n%s %s_%s[] = {\n", #t, name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_END() \
    do { \
        snprintf(buffer, sizeof(buffer), "};\n"); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer));\
    } while (0)

#define DUMP_SUB_STRUCT_BEGIN(n, a) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = {\n", n, " ", #a); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_SUB_STRUCT_END(n) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s},\n", n, " "); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer));\
    } while (0)

#define DUMP_STRUCT_ITEM_HEX_2(n, a, b) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = 0x%08X,\n", n, " ", #b, pcfgs->a.b); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_HEX_3(n, a, b, c) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = 0x%08X,\n", n, " ", #c, pcfgs->a.b.c); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_HEX_ARRAY_3(n, a) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = {0x%08X, 0x%08X, 0x%08X},\n", n, " ", #a, pcfgs->a[0], pcfgs->a[1], pcfgs->a[2]); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM(n, a) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = %d,\n", n, " ", #a, pcfgs->a); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_2(n, a, b) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = %d,\n", n, " ", #b, pcfgs->a.b); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_3(n, a, b, c) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = %d,\n", n, " ", #c, pcfgs->a.b.c); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_REG_CFG(n, name, m) \
    if (name != NULL) { \
        snprintf(buffer, sizeof(buffer), "%*sADD_CFG(%s_%s),\n", n, " ", name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    }

#define DUMP_STRUCT_PB_PARAM_ITEM(n, name, m) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s{\n", n, " "); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "%*s(int32_t *)%s_%s,\n", n+4, " ", name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "%*sARRAY_SIZE(%s_%s),\n", n+4, " ", name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "%*s0,\n", n+4, " "); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "%*s},\n", n, " "); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_DEV_VER_ITEM(n, name, m) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s(cf_dev_ver_t *)%s_%s,\n", n, " ", name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "%*sARRAY_SIZE(%s_%s),\n", n, " ", name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "%*s0,\n", n, " "); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_GAIN(n, a) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.gain = {0x%08X, 0x%08X, 0x%08X, 0x%08X, %d},\n", n, " ", a.v0c, a.v20, a.v2c, a.v24, 0); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_GAIN_REG(n, a) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.gain_reg = {0x%08X, 0x%08X, 0x%08X, 0x%08X, %d},\n", n, " ", a.reg0c, a.reg20, a.reg2c, a.reg24, 0); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_NAV_GAIN_ITEM(n, a) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s{0x%08X, 0x%08X, 0x%08X, 0x%08X, %d},\n", n, " ", a.v0c, a.v20, a.v2c, a.v24, 0); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

static int32_t _cfg_dump_mkdir(const char *path)
{
    char dir_name[FULLNAME_MAX];
    int32_t i, len;

    if (path == NULL || !path[0]) {
        LOG_MSG_DEBUG("param invalid");
        return -1;
    }

    strcpy(dir_name, path);
    len = strlen(dir_name);

    if(dir_name[len-1] != '/') {
        strcat(dir_name, "/");
    }

    len = strlen(dir_name);

    for (i = 1; i < len ; i++) {
        if(dir_name[i]=='/') {
            dir_name[i] = 0;
            if( access(dir_name, F_OK) != 0 ) {
                if(mkdir(dir_name, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) {
                    LOG_MSG_DEBUG("mkdir error (%d:%s)", errno, strerror(errno));
                    return -1;
                }
            }
            dir_name[i] = '/';
        }
    }

    return 0;
}

static int32_t _cfg_dump_get_file_path(char *buffer, uint32_t len, char *module, int32_t update)
{
    int32_t ret = _cfg_dump_mkdir(FP_CONFIG_INCLUDE_PATH);
    if (ret >= 0) {
        if (update) {
            snprintf(buffer, len, "%s/%s_upd.h", FP_CONFIG_INCLUDE_PATH, module);
        } else {
            snprintf(buffer, len, "%s/%s.h", FP_CONFIG_INCLUDE_PATH, module);
        }
    }
    return ret;
}

static void _cfg_dump_to_file(FILE *fp, char *buffer, uint32_t len)
{
    if (fp != NULL) {
        fwrite(buffer, sizeof(char), len, fp);
    }
}

static void _cfg_dump_config(FILE *fp, const int32_t idx, const cf_mode_config_t *config, const char *module)
{
    uint32_t i = 0;
    char buffer[128];
    char name[64];

    if (config != NULL) {
        snprintf(name, sizeof(name), "reg_%s", silfp_cfg_get_config_name(idx));
        DUMP_STRUCT_ARRAY_BEGIN(const cf_register_t, name, module);
        for (i = 0; i < config->len; i++) {
            DUMP_STRUCT_ITEM_HEX_VALUE_2(4, config->reg[i].addr, config->reg[i].val);
        }
        DUMP_STRUCT_END();
    }
}

static void _cfg_dump_pb_param(FILE *fp, const int32_t idx, const cf_pb_param_t *param, const char *module)
{
    uint32_t i = 0;
    char buffer[512];
    char value[16];
    char value2[16];
    const uint32_t count_per_line = 8;

    if (param != NULL) {
        DUMP_STRUCT_ARRAY_BEGIN(const int32_t, param_name[idx], module);
        for (i = 0; i < param->len; i++) {
            if (i % count_per_line == 0) {
                snprintf(buffer, sizeof(buffer), "    ");
            }
            snprintf(value, sizeof(value), "%d,", param->val[i]);
            if (((i+1) % count_per_line == 0) || (i+1) == param->len) {
                strcat(buffer, (const char *)value);
            } else {
                snprintf(value2, sizeof(value2), "%-*s", 12, value);
                strcat(buffer, (const char *)value2);
            }

            if ((i+1) % count_per_line == 0) {
                strcat(buffer, (const char *)"\n");
                _cfg_dump_to_file(fp, buffer, strlen(buffer));
            }
        }
        if (i % count_per_line != 0) {
            strcat(buffer, (const char *)"\n");
            _cfg_dump_to_file(fp, buffer, strlen(buffer));
        }
        DUMP_STRUCT_END();
    }
}

static void _cfg_dump_dev_ver(FILE *fp, const cf_set_t *pcfgs, const char *module)
{
    uint32_t i = 0;
    char buffer[512];

    if (pcfgs != NULL) {
        DUMP_STRUCT_ARRAY_BEGIN(const cf_dev_ver_t, FP_DEV_VER_NAME, module);
        for (i = 0; i < pcfgs->dev.len; i++) {
            DUMP_STRUCT_ITEM_HEX_VALUE_3(4, pcfgs->dev.ver[i].id, pcfgs->dev.ver[i].sid, pcfgs->dev.ver[i].vid);
        }
        DUMP_STRUCT_END();
    }
}

void silfp_cfg_dump_data(const cf_set_t *pcfgs, char *board_module, int32_t update)
{
    int32_t i;
    FILE *fp = NULL;
    char buffer[128];
    char dump_file_path[256];
    char *module = NULL;

    if (pcfgs == NULL) {
        LOG_MSG_VERBOSE("config NULL");
    } else {
        if (board_module == NULL) {
            module = FP_CONFIG_MODULE_DEFAULT;
        } else {
            if (strncmp(board_module, FP_CONFIG_MODULE_NAME_OBLITE, strlen(FP_CONFIG_MODULE_NAME_OBLITE)) == 0) {
                module = board_module + strlen(FP_CONFIG_MODULE_NAME_OBLITE);
            } else {
                module = board_module;
            }
        }

        if (_cfg_dump_get_file_path(dump_file_path, sizeof(dump_file_path), module, update) < 0) {
            LOG_MSG_ERROR("get file path failed");
            return;
        }

        unlink(dump_file_path);
        if ((fp = fopen(dump_file_path, "a+")) == NULL) {
            LOG_MSG_VERBOSE("fail to open dump file (%d:%s)", errno, strerror(errno));
        }

        DUMP_INCLUDE_BEGIN(module);

        for (i = 0; i < CFG_MAX; i++) {
            _cfg_dump_config(fp, i, &pcfgs->cfg[i], module);
        }

        for (i = 0; i < CFG_PB_PARAM_MAX; i ++) {
            _cfg_dump_pb_param(fp, i, &pcfgs->pb.param[i], module);
        }

        _cfg_dump_dev_ver(fp, pcfgs, module);

        DUMP_STRUCT_BEGIN(cf_set_t, CONFIG_SET_NAME, module);

        // main
        DUMP_SUB_STRUCT_BEGIN(4, dev);
        DUMP_STRUCT_DEV_VER_ITEM(8, FP_DEV_VER_NAME, module);
        DUMP_SUB_STRUCT_END(4);
        DUMP_STRUCT_ITEM_HEX_ARRAY_3(4, mask);

        // common
        DUMP_SUB_STRUCT_BEGIN(4, common);
        DUMP_STRUCT_ITEM_HEX_2(8, common, id);
        DUMP_STRUCT_ITEM_HEX_2(8, common, sid);
        DUMP_STRUCT_ITEM_HEX_2(8, common, vid);
        DUMP_STRUCT_ITEM_2(8, common, w);
        DUMP_STRUCT_ITEM_2(8, common, h);
        DUMP_STRUCT_ITEM_2(8, common, wp);
        DUMP_STRUCT_ITEM_2(8, common, hp);
        DUMP_STRUCT_ITEM_2(8, common, w_hwagc);
        DUMP_STRUCT_ITEM_2(8, common, h_hwagc);
        DUMP_STRUCT_ITEM_2(8, common, wc);
        DUMP_STRUCT_ITEM_2(8, common, hc);
        DUMP_STRUCT_ITEM_2(8, common, rw);
        DUMP_STRUCT_ITEM_2(8, common, wdpi);
        DUMP_STRUCT_ITEM_2(8, common, hdpi);
        DUMP_STRUCT_ITEM_2(8, common, fg_loop);
        DUMP_STRUCT_ITEM_GAIN(8, pcfgs->common.gain);
        DUMP_STRUCT_ITEM_GAIN_REG(8, pcfgs->common.gain_reg);
        // otp
        DUMP_SUB_STRUCT_BEGIN(8, otp);
        DUMP_STRUCT_ITEM_HEX_3(12, common, otp, otp0);
        DUMP_STRUCT_ITEM_HEX_3(12, common, otp, otp1);
        DUMP_STRUCT_ITEM_HEX_3(12, common, otp, otp2);
        DUMP_STRUCT_ITEM_HEX_3(12, common, otp, otp3);
        DUMP_STRUCT_ITEM_HEX_3(12, common, otp, otp4);
        DUMP_STRUCT_ITEM_HEX_3(12, common, otp, otp5);
        DUMP_STRUCT_ITEM_HEX_3(12, common, otp, otp_a0);
        DUMP_SUB_STRUCT_END(8);
        DUMP_SUB_STRUCT_END(4);

        // spi
        DUMP_SUB_STRUCT_BEGIN(4, spi);
        DUMP_STRUCT_ITEM_2(8, spi, ms_frm);
        DUMP_STRUCT_ITEM_2(8, spi, retry);
        DUMP_STRUCT_ITEM_2(8, spi, reinit);
        DUMP_STRUCT_ITEM_2(8, spi, start);
        DUMP_SUB_STRUCT_END(4);

        // nav
        DUMP_SUB_STRUCT_BEGIN(4, nav);
        DUMP_STRUCT_ITEM_2(8, nav, enable);
        DUMP_STRUCT_ITEM_2(8, nav, mode);
        DUMP_STRUCT_ITEM_2(8, nav, type);
        DUMP_STRUCT_ITEM_2(8, nav, con_frame_get_num);
        DUMP_STRUCT_ITEM_2(8, nav, w);
        DUMP_STRUCT_ITEM_2(8, nav, h);
        DUMP_STRUCT_ITEM_2(8, nav, wh);
        DUMP_STRUCT_ITEM_2(8, nav, hh);
        DUMP_STRUCT_ITEM_2(8, nav, w_ds);
        DUMP_STRUCT_ITEM_2(8, nav, h_ds);
        DUMP_STRUCT_ITEM_2(8, nav, w_hg_ds);
        DUMP_STRUCT_ITEM_2(8, nav, h_hg_ds);
        DUMP_SUB_STRUCT_BEGIN(8, gain); // pb.param
        for (i = 0; i < CFG_NAV_AGC_MODE_MAX; i ++) {
            DUMP_STRUCT_NAV_GAIN_ITEM(12, pcfgs->nav.gain[i]);
        }
        DUMP_SUB_STRUCT_END(8);
        DUMP_STRUCT_ITEM_2(8, nav, vk_timeout);
        DUMP_STRUCT_ITEM_2(8, nav, dclick_gap);
        DUMP_STRUCT_ITEM_2(8, nav, longpress);
        DUMP_SUB_STRUCT_END(4);

        // pb
        DUMP_SUB_STRUCT_BEGIN(4, pb);
        DUMP_SUB_STRUCT_BEGIN(8, param); // pb.param
        for (i = 0; i < CFG_PB_PARAM_MAX; i++) {
            DUMP_STRUCT_PB_PARAM_ITEM(12, param_name[i], module);
        }
        DUMP_SUB_STRUCT_END(8);
        DUMP_SUB_STRUCT_BEGIN(8, agc); // pb.agc
        DUMP_STRUCT_ITEM_3(12, pb, agc, skip_fd);
        DUMP_STRUCT_ITEM_3(12, pb, agc, fd_threshold);
        DUMP_STRUCT_ITEM_3(12, pb, agc, skip_small);
        DUMP_STRUCT_ITEM_3(12, pb, agc, max);
        DUMP_STRUCT_ITEM_3(12, pb, agc, max_small);
        DUMP_STRUCT_ITEM_3(12, pb, agc, hwagc_enable);
        DUMP_STRUCT_ITEM_3(12, pb, agc, hwcov_wake);
        DUMP_STRUCT_ITEM_3(12, pb, agc, hwcov_tune);
        DUMP_STRUCT_ITEM_3(12, pb, agc, exp_size);
        DUMP_SUB_STRUCT_END(8);
        DUMP_SUB_STRUCT_BEGIN(8, threshold); // pb.threshold
        DUMP_STRUCT_ITEM_3(12, pb, threshold, alg_select);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, enrolNum);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, max_templates_num);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, templates_size);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, identify_far_threshold);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, update_far_threshold);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, enroll_quality_threshold);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, enroll_coverage_threshold);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, quality_threshold);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, coverage_threshold);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, skin_threshold);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, artificial_threshold);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, samearea_detect);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, samearea_threshold);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, samearea_dist);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, samearea_start);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, samearea_check_once_num);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, samearea_check_num_total);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, dy_fast);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, segment);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, water_finger_detect);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, shake_coe);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, noise_coe);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, gray_prec);
        DUMP_STRUCT_ITEM_3(12, pb, threshold, water_detect_threshold);
        DUMP_SUB_STRUCT_END(8);
        DUMP_SUB_STRUCT_END(4);

        // test
        DUMP_SUB_STRUCT_BEGIN(4, test);
        DUMP_STRUCT_ITEM_2(8, test, fd_threshold);
        DUMP_STRUCT_ITEM_2(8, test, deadpx_hard_threshold);
        DUMP_STRUCT_ITEM_2(8, test, deadpx_norm_threshold);
        DUMP_STRUCT_ITEM_2(8, test, scut);
        DUMP_STRUCT_ITEM_2(8, test, detev_ww);
        DUMP_STRUCT_ITEM_2(8, test, detev_hh);
        DUMP_STRUCT_ITEM_2(8, test, deteline_h);
        DUMP_STRUCT_ITEM_2(8, test, deteline_w);
        DUMP_STRUCT_ITEM_2(8, test, deadpx_max);
        DUMP_STRUCT_ITEM_2(8, test, badline_max);
        DUMP_STRUCT_ITEM_2(8, test, finger_detect_mode);
        DUMP_STRUCT_ITEM_HEX_2(8, test, deadpx_cut);
        DUMP_SUB_STRUCT_END(4);

        // mmi
        DUMP_SUB_STRUCT_BEGIN(4, mmi);
        DUMP_STRUCT_ITEM_HEX_2(8, mmi, dac_min);
        DUMP_STRUCT_ITEM_HEX_2(8, mmi, dac_max);
        DUMP_STRUCT_ITEM_2(8, mmi, grey_range_left);
        DUMP_STRUCT_ITEM_2(8, mmi, grey_range_right);
        DUMP_STRUCT_ITEM_HEX_2(8, mmi, nav_base_frame_num);
        DUMP_STRUCT_ITEM_2(8, mmi, max_tune_time);
        DUMP_STRUCT_ITEM_2(8, mmi, auto_adjust_w);
        DUMP_STRUCT_ITEM_2(8, mmi, auto_adjust_h);
        DUMP_STRUCT_ITEM_2(8, mmi, frm_loop_max);
        DUMP_STRUCT_ITEM_HEX_2(8, mmi, postprocess_ctl);
        DUMP_STRUCT_ITEM_2(8, mmi, white_base_white_thr);
        DUMP_STRUCT_ITEM_2(8, mmi, white_base_black_thr);
        DUMP_STRUCT_ITEM_2(8, mmi, black_base_white_thr);
        DUMP_STRUCT_ITEM_2(8, mmi, black_base_black_thr);
        DUMP_STRUCT_ITEM_2(8, mmi, middle_base_white_thr);
        DUMP_STRUCT_ITEM_2(8, mmi, middle_base_black_thr);
        DUMP_STRUCT_ITEM_2(8, mmi, diff_base_min_thr);
        DUMP_STRUCT_ITEM_2(8, mmi, diff_base_max_thr);
        DUMP_STRUCT_ITEM_HEX_2(8, mmi, snr_cut);
        DUMP_STRUCT_ITEM_2(8, mmi, base_size);
        DUMP_STRUCT_ITEM_2(8, mmi, snr_img_num);
        DUMP_STRUCT_ITEM_2(8, mmi, snr_thr);
        DUMP_STRUCT_ITEM_2(8, mmi, distortion);
        DUMP_STRUCT_ITEM_2(8, mmi, finger_num);
        DUMP_STRUCT_ITEM_2(8, mmi, storage_interval);
        DUMP_STRUCT_ITEM_2(8, mmi, sum_type);
        DUMP_STRUCT_ITEM_2(8, mmi, deadpx_radius);
        DUMP_STRUCT_ITEM_2(8, mmi, auth_reverse_grey);
        // mmi.touch_info
        DUMP_SUB_STRUCT_BEGIN(8, touch_info);
        DUMP_STRUCT_ITEM_3(12, mmi, touch_info, center_x);
        DUMP_STRUCT_ITEM_3(12, mmi, touch_info, center_y);
        DUMP_STRUCT_ITEM_3(12, mmi, touch_info, b1_distance_threshold);
        DUMP_STRUCT_ITEM_3(12, mmi, touch_info, b2_distance_threshold);
        DUMP_STRUCT_ITEM_3(12, mmi, touch_info, b2_b1_distance_threshold);
        DUMP_STRUCT_ITEM_3(12, mmi, touch_info, c1_coverage_threshold);
        DUMP_STRUCT_ITEM_3(12, mmi, touch_info, c2_coverage_threshold);
        DUMP_SUB_STRUCT_END(8);
        DUMP_SUB_STRUCT_END(4);

        // aec
        DUMP_SUB_STRUCT_BEGIN(4, aec);
        DUMP_STRUCT_ITEM_HEX_2(8, aec, left);
        DUMP_STRUCT_ITEM_HEX_2(8, aec, right);
        DUMP_STRUCT_ITEM_2(8, aec, max_loop);
        DUMP_STRUCT_ITEM_2(8, aec, mean_min);
        DUMP_STRUCT_ITEM_2(8, aec, mean_max);
        DUMP_STRUCT_ITEM_2(8, aec, time);
        DUMP_STRUCT_ITEM_2(8, aec, pclk);
        DUMP_SUB_STRUCT_END(4);

        // ft
        DUMP_SUB_STRUCT_BEGIN(4, ft);
        DUMP_STRUCT_ITEM_2(8, ft, line_step_min);
        DUMP_STRUCT_ITEM_2(8, ft, ignore);
        DUMP_STRUCT_ITEM_2(8, ft, min_theta);
        DUMP_STRUCT_ITEM_2(8, ft, max_theta);
        DUMP_STRUCT_ITEM_2(8, ft, quality_thr);
        DUMP_STRUCT_ITEM_2(8, ft, line_distance_min);
        DUMP_STRUCT_ITEM_2(8, ft, line_distance_max);
        DUMP_STRUCT_ITEM_HEX_2(8, ft, cut);
        DUMP_SUB_STRUCT_END(4);

        // esd
        DUMP_SUB_STRUCT_BEGIN(4, esd);
        DUMP_STRUCT_ITEM_2(8, esd, irq_check);
        DUMP_STRUCT_ITEM_HEX_2(8, esd, irq_reg);
        DUMP_STRUCT_ITEM_HEX_2(8, esd, irq_val);
        DUMP_STRUCT_ITEM_HEX_2(8, esd, int_reg);
        DUMP_STRUCT_ITEM_HEX_2(8, esd, int_val);
        DUMP_STRUCT_ITEM_HEX_2(8, esd, int_beacon);
        DUMP_SUB_STRUCT_END(4);

        DUMP_SUB_STRUCT_BEGIN(4, cfg);
        for (i = 0; i < CFG_MAX; i++) {
            DUMP_STRUCT_REG_CFG(8, silfp_cfg_get_config_name(i), module);
        }
        DUMP_SUB_STRUCT_END(4);

        DUMP_STRUCT_END();

        DUMP_INCLUDE_END(module);

        fclose(fp);
    }
}