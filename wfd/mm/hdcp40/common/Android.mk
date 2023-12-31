LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#-----------------------------------------------------------------
# Define
#-----------------------------------------------------------------
LOCAL_CFLAGS := -D_ANDROID_
LOCAL_CFLAGS += -Wconversion
LOCAL_CFLAGS += -USYSTRACE_PROFILE
#----------------------------------------------------------------
# SRC CODE
#----------------------------------------------------------------
LOCAL_SRC_FILES := src/WFD_HdcpCP.cpp
LOCAL_SRC_FILES += src/HDCPManager.cpp

#----------------------------------------------------------------
# INCLUDE PATH
#----------------------------------------------------------------
LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_HEADER_LIBRARIES := libmmosal_headers display_proprietary_headers

#----------------------------------------------------------------
# Dx HDCP COMPILE TIME
#----------------------------------------------------------------
COMPILE_HDCP_LIB := false
WFD_NOSHIP_HDCP_PATH  := $(TOP)/vendor/qcom/proprietary/wfd-noship/mm/hdcp40
WFD_INTERNAL_LIB_PATH := $(TOP)/vendor/qcom/proprietary/wfd-internal/mm/hdcp40
COMPILE_HDCP_LIB := $(shell if [[ -d $(WFD_NOSHIP_HDCP_PATH) && -d $(WFD_INTERNAL_LIB_PATH) ]] ; then echo true; fi)

LOCAL_SHARED_LIBRARIES := libmmosal
ifeq ($(strip $(COMPILE_HDCP_LIB)),true)
$(info Compiling HDCP module)
LOCAL_C_INCLUDES += $(WFD_NOSHIP_HDCP_PATH)
LOCAL_C_INCLUDES += $(WFD_NOSHIP_HDCP_PATH)/HDCP_API
LOCAL_SHARED_LIBRARIES_32 += libDxHdcp
LOCAL_SHARED_LIBRARIES += libmm-hdcpmgr
LOCAL_CFLAGS_32 += -DWFD_HDCP_ENABLED
else
$(info Not enabling HDCP compilation)
endif

ifeq ($(call is-board-platform-in-list, lahaina holi),true)
LOCAL_CFLAGS += -DKERNEL_5_4
LOCAL_HEADER_LIBRARIES += qti_kernel_headers
endif

LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils

LOCAL_MODULE:= libwfdhdcpcp

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SANITIZE := integer_overflow
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libwfdhdcp_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/./inc
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_HEADER_LIBRARY)
