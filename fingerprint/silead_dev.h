/******************************************************************************
 * @file   silead_dev.h
 * @brief  Contains /dev/silead_fp operate functions header file.
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
 * David Wang  2018/6/5    0.1.1      Support wakelock & pwdn
 *
 *****************************************************************************/

#ifndef __SILEAD_DEV_H__
#define __SILEAD_DEV_H__

#include <sys/ioctl.h>

#include "silead_screen.h"

#define FP_SENSOR_DEVICE "/dev/silead_fp"

#ifndef SPI_IOC_MAGIC
#define SPI_IOC_MAGIC  'k'
#endif

enum _pwdn_mode_t {
	SIFP_PWDN_NONE = 0,
	SIFP_PWDN_POWEROFF = 1, /* shutdown the avdd power supply */
	SIFP_PWDN_FLASH = 2, /* shutdown avdd 200ms for H/W full reset */
	SIFP_PWDN_MAX,
};

typedef struct fp_dev_key_t {
    int32_t key;
    uint32_t value;   /* key down = 1, key up = 0 */
} FP_DEV_KEY;

#define DEVNAME_LEN 16
typedef struct fp_dev_conf_t {
    uint8_t mode;
    uint8_t bits;
    uint16_t delay;
    uint32_t speed;
    char dev[DEVNAME_LEN];
    uint8_t nl_id;
    uint8_t dev_id;
    uint16_t reserve;
    uint32_t reg;
    char ta[DEVNAME_LEN];
} FP_DEV_CONF;

#define PROC_VND_ID_LEN    32

#define SIFP_IOC_MAGIC 's' //SPI_IOC_MAGIC //'k'

#define SIFP_IOC_RESET         _IOW(SIFP_IOC_MAGIC, 10, uint8_t)

#define SIFP_IOC_ENABLE_IRQ    _IO(SIFP_IOC_MAGIC,  11)
#define SIFP_IOC_DISABLE_IRQ   _IO(SIFP_IOC_MAGIC,  12)
#define SIFP_IOC_WAIT_IRQ      _IOR(SIFP_IOC_MAGIC, 13, uint8_t)
#define SIFP_IOC_CLR_IRQ       _IO(SIFP_IOC_MAGIC,  14)

#define SIFP_IOC_KEY_EVENT     _IOW(SIFP_IOC_MAGIC, 15, struct fp_dev_key_t)
#define SIFP_IOC_INIT          _IOR(SIFP_IOC_MAGIC, 16, struct fp_dev_conf_t)
#define SIFP_IOC_DEINIT        _IO(SIFP_IOC_MAGIC,  17)
#define SIFP_IOC_IRQ_STATUS    _IOR(SIFP_IOC_MAGIC, 18, uint8_t)
#define SIFP_IOC_SCR_STATUS    _IOR(SIFP_IOC_MAGIC, 20, uint8_t)
#define SIFP_IOC_GET_VER       _IOR(SIFP_IOC_MAGIC, 21, char[10])
#define SIFP_IOC_SET_KMAP      _IOW(SIFP_IOC_MAGIC, 22, uint16_t[7])
#define SIFP_IOC_ACQ_SPI       _IO(SIFP_IOC_MAGIC,  23)
#define SIFP_IOC_RLS_SPI       _IO(SIFP_IOC_MAGIC,  24)
#define SIFP_IOC_PKG_SIZE      _IOR(SIFP_IOC_MAGIC, 25, uint8_t)     /* support from v0.0.5 */
#define SIFP_IOC_DBG_LEVEL     _IOWR(SIFP_IOC_MAGIC,26, uint8_t)     /* support from v0.0.6 */
#define SIFP_IOC_WAKELOCK      _IOW(SIFP_IOC_MAGIC, 27, uint8_t)
#define SIFP_IOC_PWDN          _IOW(SIFP_IOC_MAGIC, 28, uint8_t)
#define SIFP_IOC_PROC_NODE     _IOW(SIFP_IOC_MAGIC, 29, char[PROC_VND_ID_LEN])
/**tp info struct for underglass fingerprint,add by pruce_tang_20180710 start **/
typedef struct fingerprint_tp_info {
    uint16_t touch_state;//  up:0 ---------------------- down :1
    uint16_t image_state;// before capture img : 0 -------- after capture img 1
    int32_t touch_positon_x;
    int32_t touch_positon_y;
    uint32_t touch_coverage;	
} fingerprint_tp_info_t;
#define SIFP_IOC_GET_TP_TOUCH_INFO          _IOR(SIFP_IOC_MAGIC, 30, struct fp_dev_conf_t)
int32_t silfp_dev_get_tp_touch_info(int32_t fd, fingerprint_tp_info_t *tp_touch_info);

int32_t silfp_dev_get_ver(int32_t fd, char *ver, uint32_t len);
int32_t silfp_dev_enable(int32_t fd);
int32_t silfp_dev_disable(int32_t fd);
int32_t silfp_dev_init(int32_t fd, FP_DEV_CONF *t);
int32_t silfp_dev_deinit(int32_t fd);
int32_t silfp_dev_hw_reset(int32_t fd, uint8_t delayms);
int32_t silfp_dev_enable_irq(int32_t fd);
int32_t silfp_dev_disable_irq(int32_t fd);
int32_t silfp_dev_get_screen_status(int32_t fd, uint8_t *status);
int32_t silfp_dev_set_screen_cb(screen_cb listen, void *param);

int32_t silfp_dev_wait_finger_status(int32_t fd, int32_t uncancelable);
void silfp_dev_cancel(void);
void silfp_dev_sync_finger_status_optic(int32_t down);

int32_t silfp_dev_send_key(int32_t fd, uint32_t key);
int32_t silfp_dev_get_pkg_size(int32_t fd, uint8_t *size);
int32_t silfp_dev_set_log_level(int32_t fd, uint8_t *lvl);

int32_t silfp_dev_wakelock(int32_t fd, uint8_t lock);
int32_t silfp_dev_pwdn(int32_t fd, uint8_t avdd_down);
int32_t silfp_dev_create_proc_node(int32_t fd, char *chipname);

#endif /* __SILEAD_DEV_H__ */
