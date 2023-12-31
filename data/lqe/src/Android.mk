LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO   := data/inc
LOCAL_COPY_HEADERS      := ../inc/lqe.h

LOCAL_SRC_FILES := lqe.c
LOCAL_SRC_FILES += lqe_cmdq.c
LOCAL_SRC_FILES += lqe_qmi_ott.c
LOCAL_SRC_FILES += lqe_qmi_wds.c

#LOCAL_CFLAGS := -DFEATURE_DSI_TEST

# Logging Features. Enable only one at any time
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_STDERR
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_SYSLOG
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_ADB
LOCAL_CFLAGS += -DFEATURE_DATA_LOG_QXDM
LOCAL_CFLAGS += -DFEATURE_QDP_LINUX_ANDROID

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libdsutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libqmiservices
LOCAL_SHARED_LIBRARIES += libqmi_cci
LOCAL_SHARED_LIBRARIES += libqmi_common_so
LOCAL_SHARED_LIBRARIES += libqmi_client_helper
LOCAL_SHARED_LIBRARIES += liblog

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../dsutils/inc

LOCAL_HEADER_LIBRARIES := libqmi_common_headers \
                          libqmi_cci_headers \
                          libdiag_headers \
                          libqmi_headers \
                          libdataqmiservices_headers \
                          vendor_common_inc
LOCAL_MODULE := liblqe
LOCAL_SANITIZE:=integer_overflow

LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

LOCAL_CLANG := true

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
