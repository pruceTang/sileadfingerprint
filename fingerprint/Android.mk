##
# export SEC_TYPE=gp
# export SEC_CFLAGS=-DAAA -DBBB
##

LOCAL_PATH := $(call my-dir)

# NOSEC(non security) TEE(trustonic) QSEE(qsee) GP(global platform) ALL(all)
ifeq ($(SEC_TYPE), )
SEC_TYPE = QSEE
endif

ifeq ($(SEC_TYPE), ALL)
SEC_TYPE = QSEE TEE NOSEC GP
endif

#------------------------------------------------
build_date := $(shell date +%y%m%d%H%M%S)
LOCAL_CFLAGS_BASE := -DBUILD_DATE="\"$(build_date)\""
LOCAL_CFLAGS_BASE += -DPLATFORM_VERSION="\"$(PLATFORM_VERSION)\""

ifneq ($(SEC_CFLAGS), )
LOCAL_CFLAGS_BASE += $(SEC_CFLAGS)
endif

#------------------------------------------------
EXCEPT_SRC_LIST := 

#------------------------------------------------
SRC_FILES_SUFFIX := %.cpp %.c
BASE_FILES_PATH := $(LOCAL_PATH) $(LOCAL_PATH)/log
BASE_ALL_FILES  := $(foreach src_path, $(BASE_FILES_PATH), $(wildcard $(src_path)/*))
BASE_SRC_LIST   := $(filter $(SRC_FILES_SUFFIX), $(BASE_ALL_FILES))
BASE_SRC_LIST   := $(BASE_SRC_LIST:$(LOCAL_PATH)/%=%)

CA_FILES_PATH := $(foreach type, $(SEC_TYPE), $(LOCAL_PATH)/$(shell echo $(type) | tr A-Z a-z))
CA_ALL_FILES  := $(foreach src_path, $(CA_FILES_PATH), $(shell find $(src_path) -type f 2>/dev/null))
CA_SRC_LIST   := $(filter $(SRC_FILES_SUFFIX), $(CA_ALL_FILES))
BASE_SRC_LIST += $(CA_SRC_LIST:$(LOCAL_PATH)/%=%)

BASE_SRC_LIST := $(filter-out $(EXCEPT_SRC_LIST), $(BASE_SRC_LIST))

CA_CFLAGS := $(foreach type, $(SEC_TYPE), $(if $(shell find $(LOCAL_PATH)/$(shell echo $(type) | tr A-Z a-z) -type f 2>/dev/null), -DSECURITY_TYPE_$(shell echo $(type) | tr a-z A-Z)))
CA_CFLAGS += $(foreach type, $(SEC_TYPE), $(if $(shell [[ "gp" == "$(shell echo $(type) | tr A-Z a-z)" ]] && echo true), -DTBASE_API_LEVEL=3))
CA_CFLAGS += $(LOCAL_CFLAGS_BASE)
CA_CFLAGS := $(strip $(CA_CFLAGS))
$(warning $(CA_CFLAGS))

#------------------------------------------------
include $(CLEAR_VARS)
# if $(PLATFORM_SDK_VERSION) less then 21 (android5.0), not enable sepolicy
local_build_silead_sepolicy := $(shell if [ $(PLATFORM_SDK_VERSION) -ge 21 ]; then echo yes; else echo no; fi)

# if $(PLATFORM_SDK_VERSION) is android8.0 (26) or higher, enable vendor img
local_silead_support_vendor := $(shell if [ $(PLATFORM_SDK_VERSION) -ge 26 ]; then echo yes; else echo no; fi)
ifeq ($(local_silead_support_vendor), yes)
    LOCAL_VENDOR_MODULE := true
endif

ifneq ($(local_build_silead_sepolicy), yes)
    CA_CFLAGS += -DSILEAD_SEPOLICY_UNSUPPORT
endif

ifneq ($(BUILD_SILEAD_FW_HARDWARE_INCLUDE), )
    LOCAL_C_INCLUDES += $(BUILD_SILEAD_FW_HARDWARE_INCLUDE)
    CA_CFLAGS += -DANDROID_NDK
endif

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libsl_fp_impl

LOCAL_SRC_FILES := $(BASE_SRC_LIST)
LOCAL_CFLAGS := $(CA_CFLAGS)
LOCAL_CFLAGS += -DSL_FP_FEATURE_OPPO_CUSTOMIZE
LOCAL_CFLAGS += -DSL_FP_FEATURE_OPPO_CUSTOMIZE_OPTIC
LOCAL_CFLAGS += -DSIL_DEBUG_ALL_LOG
ifeq ($(SILEAD_TEST_TA), yes)
LOCAL_CFLAGS += -DSIL_DUMP_IMAGE
endif

ifeq ($(SILEAD_CUST_X), yes)
LOCAL_CFLAGS += -DFP_QSEE_TZAPP_NAME=\"sldcps\"
LOCAL_CFLAGS += -DFP_CONFIG_PATH=\"\/data\/vendor\/silead\"
LOCAL_CFLAGS += -DFP_BMP_PATH=\"\/data\/vendor\/silead\"
endif


LOCAL_C_INCLUDES += $(LOCAL_PATH) \
                    $(LOCAL_PATH)/include
LOCAL_SHARED_LIBRARIES := libdl libcutils libselinux liblog libsl_fp_impl_util
LOCAL_REQUIRED_MODULES := libsl_fp_impl_util

include $(BUILD_SHARED_LIBRARY)
