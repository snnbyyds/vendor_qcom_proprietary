LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS               += -Wall -Werror $(qcril_cflags)
LOCAL_CXXFLAGS             += -std=c++17 $(qcril_cppflags)
LOCAL_CPPFLAGS             += -std=c++17 $(qcril_cppflags)
LOCAL_LDFLAGS              += $(qcril_ldflags)

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE             += $(qcril_sanitize)
endif
LOCAL_SRC_FILES            += $(call all-cpp-files-under, src)

LOCAL_MODULE               := qcrilNrNwRegistrationModule
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional

LOCAL_HEADER_LIBRARIES     += qcrilDataInternal_headers
LOCAL_HEADER_LIBRARIES     += libqcrilNr-headers
LOCAL_HEADER_LIBRARIES     += libqcrilNrQtiMutex-headers

LOCAL_SHARED_LIBRARIES     := libqcrilNrQtiMutex

include $(BUILD_STATIC_LIBRARY)
