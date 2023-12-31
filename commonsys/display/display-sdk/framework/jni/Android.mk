# @file Android.mk
#

LOCAL_PATH:= $(call my-dir)
ifneq ($(TARGET_DISABLE_DISPLAY),true)

include $(CLEAR_VARS)
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libsd_sdk_display.qti
LOCAL_MODULE_OWNER := qti

LOCAL_SRC_FILES:= displaysdk_jni.c
LOCAL_HEADER_LIBRARIES  := display_intf_headers display_proprietary_intf_headers

LOCAL_SHARED_LIBRARIES := liblog libutils libsdm-disp-apis.qti


LOCAL_CFLAGS += -Wno-multichar -Wno-unused-parameter -Werror
LOCAL_CLANG  := true

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE:= false
LOCAL_SYSTEM_EXT_MODULE := true

include $(BUILD_SHARED_LIBRARY)
endif #TARGET_DISABLE_DISPLAY
