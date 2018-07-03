ifeq ($(SILEAD_FP_TEST_SUPPORT), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(SILEAD_ANDROID_SUPPORT_VENDOR), yes)
    LOCAL_VENDOR_MODULE := true
endif

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := frrfarTest
LOCAL_CERTIFICATE := platform
LOCAL_STATIC_JAVA_LIBRARIES := com.silead.manager android-support-v4 android-support-v7-appcompat
LOCAL_REQUIRED_MODULES := com.silead.manager

include $(BUILD_PACKAGE)

endif #SILEAD_FP_TEST_SUPPORT