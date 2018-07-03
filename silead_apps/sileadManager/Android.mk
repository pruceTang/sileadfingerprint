ifeq ($(SILEAD_FP_TEST_SUPPORT), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(SILEAD_ANDROID_SUPPORT_VENDOR), yes)
    LOCAL_VENDOR_MODULE := true
endif

LOCAL_PACKAGE_NAME := sileadManager
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_STATIC_JAVA_LIBRARIES := com.silead.manager
LOCAL_REQUIRED_MODULES := com.silead.manager

include $(BUILD_PACKAGE)

endif #SILEAD_FP_TEST_SUPPORT