LOCAL_PATH:= $(call my-dir)

ifndef QC_PROP_ROOT
  QC_PROP_ROOT := vendor/qcom/proprietary
endif

SECUREMSM_SHIP_PATH      := $(QC_PROP_ROOT)/securemsm
SSG_PATH                 := $(QC_PROP_ROOT)/ssg

COMMON_C_INCLUDES = \
                    $(SECUREMSM_SHIP_PATH)/smcinvoke/inc \
                    $(SECUREMSM_SHIP_PATH)/smcinvoke/QCBOR/inc \
                    $(SSG_PATH)/mink/include/ \
                    $(SSG_PATH)/connection-security/include/ \
                    $(LOCAL_PATH) \

COMMON_SHARED_LIBRARIRS = \
                          liblog \
                          libminkdescriptor \
                          libminksocket \

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(COMMON_C_INCLUDES)
LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_SHARED_LIBRARIES += $(COMMON_SHARED_LIBRARIRS)
LOCAL_CFLAGS += -Wno-unused-parameter

LOCAL_MODULE := pvclicense_sample
LOCAL_SRC_FILES := pvclicense.cpp
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)
