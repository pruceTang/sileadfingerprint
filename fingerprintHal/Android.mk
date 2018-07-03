ifeq ($(SILEAD_FP_SUPPORT), yes)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifneq ($(BUILD_SILEAD_FW_HARDWARE_INCLUDE), )
    LOCAL_C_INCLUDES := $(BUILD_SILEAD_FW_HARDWARE_INCLUDE)
endif

ifeq ($(BUILD_SILEAD_LIBS_RELATIVE_PATH_UNSUPPORT), true)
    LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
else
    LOCAL_MODULE_RELATIVE_PATH := hw
endif

ifeq ($(SILEAD_ANDROID_SUPPORT_VENDOR), yes)
    LOCAL_VENDOR_MODULE := true
endif

ifeq ($(PLATFORM_SDK_VERSION), 23)      #android6
    LOCAL_CFLAGS := -DANDROID6
endif

LOCAL_CFLAGS += -DSL_FP_FEATURE_OPPO_CUSTOMIZE
LOCAL_CFLAGS += -DSL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC

LOCAL_MODULE := fingerprint.silead.default
LOCAL_SRC_FILES := fingerprint.cpp
LOCAL_SHARED_LIBRARIES := liblog libsl_fp_impl
LOCAL_REQUIRED_MODULES := libsl_fp_impl
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

endif