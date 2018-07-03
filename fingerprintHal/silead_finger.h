#ifndef __SILEAD_FINGER_H__
#define __SILEAD_FINGER_H__

#include <hardware/hardware.h>
#include <hardware/fingerprint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t silfp_finger_pre_enroll();
int silfp_finger_enroll(const hw_auth_token_t *hat, uint32_t __unused gid, uint32_t __unused timeout_sec);
int silfp_finger_post_enroll();
int silfp_finger_authenticate(uint64_t operation_id, uint32_t __unused gid);
uint64_t silfp_finger_get_auth_id();
int silfp_finger_cancel();
int silfp_finger_remove(uint32_t __unused gid, uint32_t fid);
int silfp_finger_enumerate6(fingerprint_finger_id_t *results, uint32_t *max_size);
int silfp_finger_enumerate();
int silfp_finger_set_active_group(uint32_t gid, const char *store_path);
int silfp_finger_set_notify_callback(fingerprint_notify_t notify);

int silfp_finger_init();
int silfp_finger_deinit();

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE
int silfp_get_enroll_total_times();
int silfp_pause_enroll();
int silfp_continue_enroll();
int silfp_pause_identify();
int silfp_continue_identify();
int silfp_get_alikey_status();
int silfp_cleanup();
int silfp_set_touch_event_listener();
int silfp_set_screen_state(uint32_t sreenstate);
int silfp_dynamically_config_log(uint32_t on);
int silfp_get_engineering_info(uint32_t type);
#endif

#ifdef SL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
int silfp_touch_down();
int silfp_touch_up();
int silfp_send_fingerprint_cmd(int32_t cmd_id, int8_t* in_buf, uint32_t size);
#endif

#ifdef __cplusplus
}
#endif

#endif // __SILEAD_FINGER_H__