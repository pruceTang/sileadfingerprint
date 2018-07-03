LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(SILEAD_ANDROID_SUPPORT_VENDOR), yes)
    LOCAL_VENDOR_MODULE := true
endif

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES += $(call all-Iaidl-files-under, src)

LOCAL_PACKAGE_NAME := opticCal
LOCAL_CERTIFICATE := platform
LOCAL_DEX_PREOPT := false

include $(BUILD_PACKAGE)