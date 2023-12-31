LOCAL_PATH          := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE := vendor.qti.hardware.trustedui@1.0-service-qti
LOCAL_INIT_RC := vendor.qti.hardware.trustedui@1.0-service-qti.rc
LOCAL_SRC_FILES := service.cpp

LOCAL_C_INCLUDES :=

LOCAL_SHARED_LIBRARIES := liblog \
                          libcutils \
                          libdl \
                          libbase \
                          libutils \
                          libhardware \
                          libhidlbase \
                          vendor.qti.hardware.trustedui@1.0 \
                          vendor.qti.hardware.trustedui@1.1 \
                          vendor.qti.hardware.trustedui@1.2 \
                          vendor.qti.hardware.systemhelper@1.0 \
                          libTrustedUI \
                          libTrustedInput \

LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)

