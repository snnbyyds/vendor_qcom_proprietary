# sources and intermediate files are separated

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

# Logging Features. Enable only one at any time
LOCAL_CFLAGS += -DFEATURE_DATA_LOG_STDERR
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_SYSLOG
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_ADB
#LOCAL_CFLAGS += -DFEATURE_DATA_LOG_QXDM

LOCAL_CFLAGS += -DFEATURE_DS_LINUX_NO_RPC
LOCAL_CFLAGS += -DFEATURE_DS_LINUX_ANDROID

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_C_INCLUDES := system/core/libnetutils/
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../src/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qmi/platform
LOCAL_C_INCLUDES += vendor/qcom/proprietary/qmi-framework/inc
LOCAL_C_INCLUDES += vendor/qcom/proprietary/qmi-framework/qcci/inc
LOCAL_C_INCLUDES += vendor/qcom/proprietary/qmi-framework/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_SRC_FILES := \
	nl_listener.c


LOCAL_HEADER_LIBRARIES := libqmi_common_headers \
                          libqmi_cci_headers \
                          libdiag_headers \
                          libqmi_headers \
                          libdataqmiservices_headers

LOCAL_MODULE := nl_listener
LOCAL_SANITIZE:=integer_overflow
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
	libdsutils  \
	libqmi  \
	liblog  \
	libdiag  \
	libnetutils \
	libnetmgr

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)
