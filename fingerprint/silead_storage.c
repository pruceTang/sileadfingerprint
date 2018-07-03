/******************************************************************************
 * @file   silead_storage.c
 * @brief  Contains storage functions.
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
 * Luke Ma     2018/5/13   0.1.1      Add config load/save I/F
 *
 *****************************************************************************/

#define FILE_TAG "silead_storage"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <selinux/android.h>

#include "silead_storage.h"
#include "silead_error.h"

#define FNAME_MAX      64
#define FULLNAME_MAX   255

#define BAK_SUFFIX ".bak"
#define MAX_LOAD_FAIL 5

typedef struct __attribute__((packed)) _tpl {
    uint32_t id;
    uint32_t len;
    char name[FNAME_MAX];
    int32_t checksum;
    int32_t failcount;
} T_TPL_MAP;

typedef struct __attribute__((packed)) _tpls {
    T_TPL_MAP tpl[TPL_MAX_ST];
    int32_t  num;
    int32_t init_once;
    int32_t checksum;
} T_TPL_SET;

static char m_str_tpl_path[PATH_MAX] = {0};

static T_TPL_SET tpls;
static pthread_mutex_t sto_lock;
static uint32_t lastId;

#define TPL_FNAME "silead.tpl"

static void _storage_dump();

const char *silfp_storage_get_tpl_path()
{
    if (m_str_tpl_path[0]) {
        return m_str_tpl_path;
    } else {
        return "/data/silead/fp/";
    }
}

void silfp_storage_set_tpl_path(const char *path)
{
    if (path != NULL) {
        snprintf(m_str_tpl_path, sizeof(m_str_tpl_path), "%s", path);
    }
}

static int32_t _storage_mkdir(const char *path)
{
    char dir_name[FULLNAME_MAX];
    int32_t i, len;

    if (path == NULL || !path[0]) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
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
                    return -SL_ERROR_STO_OP_FAILED;
                }
            }
            dir_name[i] = '/';
        }
    }
#ifndef SILEAD_SEPOLICY_UNSUPPORT
    if (selinux_android_restorecon(dir_name, 0) < 0) {
        LOG_MSG_DEBUG("restore sepolicy failed");
    }
#endif

    return SL_SUCCESS;
}

static int32_t _storage_make_filename(const char *path, const char *name, char *fname)
{
    int32_t len;
    const char *p = (char *)path;

    if (name == NULL || fname == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if (p == NULL) {
        p = silfp_storage_get_tpl_path();
    }

    if (p == NULL) {
        LOG_MSG_DEBUG("get tpl path failed");
        return -SL_ERROR_STO_OP_FAILED;
    }

    len = strlen(p);
    if (len > 0 && p[len - 1] == '/') {
        snprintf(fname, FULLNAME_MAX, "%s%s", p, name);
    } else {
        snprintf(fname, FULLNAME_MAX, "%s/%s", p, name);
    }

    return SL_SUCCESS;
}

static int32_t _storage_calc_checksum(const char *in, const uint32_t len, int32_t *checksum)
{
    uint32_t i;
    unsigned long int sum = 0;
    unsigned char *p;

    if (in == NULL || checksum == NULL || len == 0) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    p = (unsigned char *)in;
    for (i = 0; i < len; i++) {
        sum += *p++;
    }

    while (sum >> 16) {
        sum = (sum >> 16) + (sum & 0xFFFF);
    }

    *checksum = ~sum;

    return SL_SUCCESS;
}

static int32_t _storage_save_sets(const char *path, const char *name, T_TPL_SET *sets)
{
    int32_t ret = SL_SUCCESS;
    FILE *fp = NULL;
    char fname[FULLNAME_MAX];
    char fname_bak[FULLNAME_MAX];
    int32_t checksum;

    if (path == NULL || name == NULL || sets == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if(access(path, F_OK) != 0) {
        ret = _storage_mkdir(path);
        if (ret < 0) {
            return ret;
        }
    }

    ret = _storage_make_filename(path, name, (char *)fname);
    if (ret < 0) {
        return ret;
    }

    snprintf(fname_bak, FULLNAME_MAX, "%s%s", fname, BAK_SUFFIX);
    if ((fp = fopen(fname_bak, "wb")) == NULL) {
        LOG_MSG_DEBUG("open fail (%d:%s)", errno, strerror(errno));
        return -SL_ERROR_STO_OP_FAILED;
    }

    _storage_calc_checksum((char *)sets, sizeof(T_TPL_SET) - sizeof(int32_t), &checksum);
    sets->checksum = checksum;

    ret = fwrite(sets, sizeof(char), sizeof(T_TPL_SET), fp);
    fclose(fp);

    if (ret != sizeof(T_TPL_SET)) {
        unlink((char *)fname_bak);

        LOG_MSG_DEBUG("write failed (%d)", ret);
        return -SL_ERROR_STO_OP_FAILED;
    }

    ret = rename(fname_bak, fname);
    if (ret < 0) {
        LOG_MSG_DEBUG("rename failed (%d:%s)", errno, strerror(errno));
        ret = -SL_ERROR_STO_OP_FAILED;
    }

    ret = SL_SUCCESS;

    return ret;
}

static int32_t _storage_init()
{
    T_TPL_SET *ptpls = &tpls;
    FILE *fp = NULL;
    char  fname[FULLNAME_MAX];
    const char *path = NULL;
    int32_t ret, i;
    int32_t checksum;

    /*if (ptpls->init_once) {
        return ptpls->num;
    }*/

    path = silfp_storage_get_tpl_path();
    if (path == NULL) {
        LOG_MSG_DEBUG("get tpl path failed");
        return -SL_ERROR_STO_OP_FAILED;
    }

    ret = _storage_make_filename(path, TPL_FNAME, (char *)fname);
    if (ret < 0) {
        return ret;
    }

    lastId = 0;
    pthread_mutex_init(&sto_lock, NULL);

    pthread_mutex_lock(&sto_lock);
    do {
        memset(ptpls, 0, sizeof(T_TPL_SET));
        fp = fopen(fname, "rb");
        if (fp == NULL) {
            LOG_MSG_DEBUG("open tpl name fail (%d:%s), reset", errno, strerror(errno));
            ptpls->init_once = 1;
            ret = _storage_save_sets(path, TPL_FNAME, ptpls);
            ret = 0;
            break;
        }

        fseek(fp, 0, SEEK_SET);
        ret = fread(ptpls, sizeof(char), sizeof(T_TPL_SET), fp);
        fclose(fp);

        _storage_calc_checksum((char *)ptpls, sizeof(T_TPL_SET) - sizeof(int32_t), &checksum);
        if (ret != sizeof(T_TPL_SET) || checksum != ptpls->checksum) {
            LOG_MSG_DEBUG("fingerprint data has been tampered with (%d), reset", ret);
            memset(ptpls, 0, sizeof(T_TPL_SET));
            ptpls->init_once = 1;
            ret = _storage_save_sets(path, TPL_FNAME, ptpls);
            ret = 0;
            break;
        } else {
            ptpls->num = 0;
            for (i = 0; i < TPL_MAX_ST; i++) {
                if (ID_VALID(ptpls->tpl[i].id)) {
                    ptpls->num++;
                    if (lastId < ptpls->tpl[i].id) {
                        lastId = ptpls->tpl[i].id;
                    }
                }
            }
            ptpls->init_once = 1;
            ret = ptpls->num;
        }
    } while (0);
    pthread_mutex_unlock(&sto_lock);

    //_storage_dump();

    return ret;
}

int32_t silfp_storage_get_idlist(uint32_t *idlist, int32_t force)
{
    int32_t i, ret;
    T_TPL_SET *ptpls = &tpls;
    int32_t num = 0;

    if (idlist == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if (!ptpls->init_once || force) {
        ret = _storage_init();
        if (ret < 0) {
            return ret;
        }
    }

    pthread_mutex_lock(&sto_lock);
    for (i = 0; i < TPL_MAX_ST ; i ++) {
        if (ID_VALID(ptpls->tpl[i].id)) {
            *idlist++ = ptpls->tpl[i].id;
            num++;
        }
    }
    pthread_mutex_unlock(&sto_lock);

    return num;
}

static int32_t _storage_save_ex(const char *path, const char *name, const char *buf, const uint32_t len)
{
    FILE *fp;
    char fname[FULLNAME_MAX], *p;
    char fname_bak[FULLNAME_MAX];
    int32_t ret, l;

    if (buf == NULL || len == 0) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _storage_make_filename(path, name, (char *)fname);
    if (ret < 0) {
        return ret;
    }

    snprintf(fname_bak, FULLNAME_MAX, "%s%s", fname, BAK_SUFFIX);
    if ((fp = fopen(fname_bak, "wb")) == NULL) {
        LOG_MSG_DEBUG("open fail (%d:%s)", errno, strerror(errno));
        return -SL_ERROR_STO_OP_FAILED;
    }

    p = (char *)buf;
    l = len;

    do {
        ret = fwrite(p, sizeof(char), l, fp);
        if (ret > 0) {
            l -= ret;
            p += ret;
        } else {
            break;
        }
    } while(l > 0);

    fclose(fp);

    if (l > 0) {
        LOG_MSG_DEBUG("write fail (%d:%s)", errno, strerror(errno));
        return -SL_ERROR_STO_OP_FAILED;
    }

    ret = rename(fname_bak, fname);
    if (ret < 0) {
        LOG_MSG_DEBUG("rename failed (%d:%s)", errno, strerror(errno));
    } else {
        ret = len;
    }

    return ret;
}

inline int32_t _storage_save(const char *name, const char *buf, const uint32_t len)
{
    return _storage_save_ex(NULL, name, buf, len);
}

int32_t _storage_remove(const char *name)
{
    char fname[FULLNAME_MAX], *p;
    int32_t ret;

    if (name == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _storage_make_filename(NULL, name, (char *)fname);
    if (ret < 0) {
        return ret;
    }

    ret = unlink((char *)fname);
    if (ret < 0) {
        LOG_MSG_DEBUG("remove failed (%d:%s:%s)", errno, strerror(errno), fname);
    }

    ret = SL_SUCCESS;

    return ret;
}

int32_t silfp_storage_update(const uint32_t id, const char *buf, const uint32_t len)
{
    T_TPL_SET *ptpls = &tpls;
    int32_t i, ret = SL_SUCCESS;
    int32_t checksum = 0;

    if (!ptpls->init_once) {
        ret = _storage_init();
        if (ret < 0) {
            return ret;
        }
    }

    pthread_mutex_lock(&sto_lock);
    for (i = 0; i < TPL_MAX_ST; i ++) {
        if (ptpls->tpl[i].id == id) {
            _storage_calc_checksum(buf, len, &checksum);
            if (checksum != ptpls->tpl[i].checksum) {
                ret = _storage_save((const char *)ptpls->tpl[i].name, buf, len);
                if (ret >= 0) { // Checksum updated, need save it.
                    ptpls->tpl[i].len = ret;
                    ptpls->tpl[i].checksum = checksum;
                    if (_storage_save_sets(silfp_storage_get_tpl_path(), TPL_FNAME, ptpls) < 0) {
                        LOG_MSG_DEBUG("Oooops, save tpl config fail. but tpl data is saved.");
                    }
                }
            }
            break;
        }
    }
    pthread_mutex_unlock(&sto_lock);

    if (i >= TPL_MAX_ST) {
        LOG_MSG_DEBUG("update failed, not find the tpl");
        ret = -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

int32_t silfp_storage_save(const char *buf, const uint32_t len, uint32_t *fid)
{
    T_TPL_SET *ptpls = &tpls;
    int32_t i = 0, ret = SL_SUCCESS;
    uint32_t id;
    int32_t max_fail_index = -1;
    int32_t max_fail = 0;

    if (!ptpls->init_once) {
        ret = _storage_init();
        if (ret < 0) {
            return ret;
        }
    }

    id = silfp_storage_get_next_fid();

    pthread_mutex_lock(&sto_lock);
    for (i = 0; i < TPL_MAX_ST; i ++) {
        if (ptpls->tpl[i].id == id) {
            LOG_MSG_DEBUG("save failed, already exist");
            ret = -SL_ERROR_STO_OP_FAILED;
        }
    }

    if (ret >= 0) {
        for (i = 0; i < TPL_MAX_ST; i ++) {
            if (!ID_VALID(ptpls->tpl[i].id)) {
                break;
            } else {
                if (ptpls->tpl[i].failcount > max_fail) {
                    max_fail = ptpls->tpl[i].failcount;
                    max_fail_index = i;
                }
            }
        }

        if (i >= TPL_MAX_ST) {
            if (max_fail_index >= 0 && max_fail_index < TPL_MAX_ST) {
                i = max_fail_index;
            }
        }

        if (i >= 0 && i < TPL_MAX_ST) {
            if (max_fail_index == i) {
                ptpls->num--;
                _storage_remove(ptpls->tpl[i].name);
            }

            memset(&ptpls->tpl[i], 0, sizeof(T_TPL_MAP));
            sprintf((char *)ptpls->tpl[i].name, "f%d.dat", id);
            ret = _storage_save((const char *)ptpls->tpl[i].name, buf, len);
            if (ret >= 0) {
                ptpls->tpl[i].id = id;
                ptpls->tpl[i].len = ret;
                _storage_calc_checksum(buf, len, &ptpls->tpl[i].checksum);

                ptpls->num ++;
                ret = _storage_save_sets(silfp_storage_get_tpl_path(), TPL_FNAME, ptpls);
                if (ret < 0) {
                    _storage_remove((const char *)ptpls->tpl[i].name);
                    memset(&ptpls->tpl[i], 0, sizeof(T_TPL_MAP));
                    ptpls->num --;
                } else {
                    ret = 0;
                    if (fid != NULL) {
                        *fid = id;
                    }
                }
            }
        }
    }
    pthread_mutex_unlock(&sto_lock);

    if (i >= TPL_MAX_ST) {
        LOG_MSG_DEBUG("save failed, tpl full");
        ret = -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

static int32_t _storage_load_ex(const char *path, const char *name, char *buf, const uint32_t len)
{
    FILE *fp = NULL;
    char fname[FULLNAME_MAX], *p;
    int32_t ret, l;

    if (buf == NULL || len == 0) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _storage_make_filename(path, name, (char *)fname);
    if (ret < 0) {
        return ret;
    }

    if ((fp = fopen(fname, "rb")) == NULL) {
        LOG_MSG_DEBUG("open failed (%d:%s)", errno, strerror(errno));
        return -SL_ERROR_STO_OP_FAILED;
    }

    p = buf;
    l = len;

    fseek(fp, 0, SEEK_SET);
    do {
        ret = fread(p, sizeof(char), l, fp);
        if (ret > 0) {
            l -= ret;
            p += ret;
        } else {
            break;
        }
    } while(l > 0);

    fclose(fp);

    if (l > 0) {
        LOG_MSG_DEBUG("read fail (%d:%s)", errno, strerror(errno));
        return -SL_ERROR_STO_OP_FAILED;
    }

    ret = len;

    return ret;
}

inline int32_t _storage_load(const char *name, char *buf, const uint32_t len)
{
    return _storage_load_ex(NULL, name, buf, len);
}

int32_t silfp_storage_load(const uint32_t id, char *buf, const uint32_t len)
{
    T_TPL_SET *ptpls = &tpls;
    int32_t i, ret = SL_SUCCESS;
    uint32_t size = len;
    int32_t oldfailcount;
    int32_t checksum;

    if (!ptpls->init_once) {
        ret = _storage_init();
        if (ret < 0) {
            return ret;
        }
    }

    pthread_mutex_lock(&sto_lock);
    for (i = 0; i < TPL_MAX_ST; i ++) {
        if (ptpls->tpl[i].id == id) {
            if (size > ptpls->tpl[i].len) {
                size = ptpls->tpl[i].len;
            }
            oldfailcount = ptpls->tpl[i].failcount;
            ret = _storage_load((const char *)ptpls->tpl[i].name, buf, size);
            if (ret > 0) {
                _storage_calc_checksum(buf, ret, &checksum);
                if (ptpls->tpl[i].checksum != checksum) {
                    ptpls->tpl[i].failcount++;
                    LOG_MSG_DEBUG("load failed, tpl invalid");
                    ret = -SL_ERROR_STO_OP_FAILED;
                } else {
                    ptpls->tpl[i].failcount = 0;
                }
            } else {
                ptpls->tpl[i].failcount++;
            }

            if (oldfailcount != ptpls->tpl[i].failcount) {
                if (ptpls->tpl[i].failcount >= MAX_LOAD_FAIL) {
                    T_TPL_MAP tpl = ptpls->tpl[i];
                    memset(&ptpls->tpl[i], 0, sizeof(T_TPL_MAP));
                    ptpls->num--;
                    _storage_remove((const char *)tpl.name);
                    LOG_MSG_DEBUG("max load error, reset the tpl");
                }
                if(_storage_save_sets(silfp_storage_get_tpl_path(), TPL_FNAME, ptpls) < 0) {
                    LOG_MSG_DEBUG("Ooops, how can I do!!!");
                }
            }
            break;
        }
    }
    pthread_mutex_unlock(&sto_lock);

    if (i >= TPL_MAX_ST) {
        LOG_MSG_DEBUG("load failed, not find the tpl");
        ret = -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

int32_t silfp_storage_remove(const uint32_t id)
{
    T_TPL_SET *ptpls = &tpls;
    int32_t i, ret = SL_SUCCESS;

    if (!ptpls->init_once) {
        ret = _storage_init();
        if (ret < 0) {
            return ret;
        }
    }

    pthread_mutex_lock(&sto_lock);
    for (i = 0; i < TPL_MAX_ST; i ++) {
        if (ptpls->tpl[i].id == id) {
            T_TPL_MAP tpl = ptpls->tpl[i];
            memset(&ptpls->tpl[i], 0, sizeof(T_TPL_MAP));
            ptpls->num--;
            ret = _storage_save_sets(silfp_storage_get_tpl_path(), TPL_FNAME, ptpls);
            if (ret >= 0) {
                ret = _storage_remove((const char *)tpl.name);
                ret = 0;
            } else {
                ptpls->tpl[i] = tpl;
                ptpls->num ++;
                ret = -SL_ERROR_STO_OP_FAILED;
            }
            break;
        }
    }
    pthread_mutex_unlock(&sto_lock);

    if (i >= TPL_MAX_ST) {
        LOG_MSG_DEBUG("remove failed, not find the tpl");
        //ret = -SL_ERROR_STO_OP_FAILED;
        ret = 0; // if the tpl not exist, then report success
    }

    return ret;
}

void silfp_storage_release(void)
{
    pthread_mutex_destroy(&sto_lock);
    LOG_MSG_VERBOSE("storage release");
}

static void _storage_dump(void)
{
    T_TPL_SET *ptpls = &tpls;
    int32_t i, ret;

    if (!ptpls->init_once) {
        ret = _storage_init();
        if (ret < 0) {
            return;
        }
    }

    pthread_mutex_lock(&sto_lock);
    LOG_MSG_VERBOSE("tpls: %d (%d), (%s)", ptpls->num, ptpls->init_once, silfp_storage_get_tpl_path());
    for (i = 0; i < TPL_MAX_ST; i ++) {
        LOG_MSG_VERBOSE("tpls[%d]: %d->%s %d", i, ptpls->tpl[i].id, ptpls->tpl[i].name, ptpls->tpl[i].failcount);
    }
    pthread_mutex_unlock(&sto_lock);
}

uint32_t silfp_storage_get_next_fid(void)
{
    T_TPL_SET *ptpls = &tpls;
    int32_t reset = 0;
    uint32_t newid = lastId + 1;
    int32_t i;

    if (newid < TPLID_S) {
        newid = TPLID_S;
    }

    if (newid >= TPLID_E) {
        reset = 1;
        newid = TPLID_S;
    }

    pthread_mutex_lock(&sto_lock);
    do {
        for (i = 0; i < TPL_MAX_ST; i++) {
            if (ptpls->tpl[i].id == newid) { // found
                newid++;
                if (newid >= TPLID_E && !reset) {
                    newid = TPLID_S;
                }
                break;
            }
        }
        if (i >= TPL_MAX_ST) { // not found
            break;
        }
    } while (newid < TPLID_E);
    pthread_mutex_unlock(&sto_lock);

    lastId = newid;

    if (newid >= TPLID_E) {
        newid = 0;
    }
    return newid;
}

int32_t silfp_storage_inc_fail_count(const uint32_t id)
{
    T_TPL_SET *ptpls = &tpls;
    int32_t i, ret = SL_SUCCESS;

    if (!ptpls->init_once) {
        ret = _storage_init();
        if (ret < 0) {
            return ret;
        }
    }

    pthread_mutex_lock(&sto_lock);
    for (i = 0; i < TPL_MAX_ST; i ++) {
        if (ptpls->tpl[i].id == id) {
            ptpls->tpl[i].failcount++;
            if (ptpls->tpl[i].failcount >= MAX_LOAD_FAIL) {
                T_TPL_MAP tpl = ptpls->tpl[i];
                memset(&ptpls->tpl[i], 0, sizeof(T_TPL_MAP));
                ptpls->num--;
                _storage_remove((const char *)tpl.name);
                LOG_MSG_DEBUG("max load error, reset the tpl");
            }
            if(_storage_save_sets(silfp_storage_get_tpl_path(), TPL_FNAME, ptpls) < 0) {
                LOG_MSG_DEBUG("Ooops, how can I do!!!");
            }
            break;
        }
    }
    pthread_mutex_unlock(&sto_lock);

    if (i >= TPL_MAX_ST) {
        LOG_MSG_DEBUG("load failed, not find the tpl");
        ret = -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

int32_t silfp_storage_remove_file(const char *fname)
{
    if (fname == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }
    return unlink(fname);
}

int32_t silfp_storage_get_file_size(const char *fname)
{
    struct stat st;

    if (fname == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if(stat(fname, &st) < 0) {
        return -1;
    }

    return st.st_size;
}

int32_t silfp_storage_load_config(const char *path, const char *name, char *buf, const uint32_t len)
{
    return _storage_load_ex(path, name, buf, len);
}

int32_t silfp_storage_save_config(const char *path, const char *name, const char *buf, const uint32_t len)
{
    int32_t ret = SL_SUCCESS;

    if (path == NULL || name == NULL || buf == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if(access(path, F_OK) != 0) {
        ret = _storage_mkdir(path);
        if (ret < 0) {
            return ret;
        }
    }

    return _storage_save_ex(path, name, buf, len);
}
