ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(call is-board-platform-in-list,msm8996 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm710 qcs605 sdmshrike msmnile $(MSMSTEPPE) $(TRINKET) kona lahaina holi lito bengal),true)
ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libOmxAmrDec-def := -g -O3
libOmxAmrDec-def += -DQC_MODIFIED
libOmxAmrDec-def += -D_ANDROID_
libOmxAmrDec-def += -D_ENABLE_QC_MSG_LOG_
libOmxAmrDec-def += -DVERBOSE
libOmxAmrDec-def += -D_DEBUG
libOmxAmrDec-def += -Wconversion
ifeq ($(call is-board-platform-in-list,msm8996 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm710 qcs605 sdmshrike msmnile $(MSMSTEPPE) $(TRINKET) kona lahaina holi lito bengal),true)
libOmxAmrDec-def += -DQCOM_AUDIO_USE_SYSTEM_HEAP_ID
endif

# ---------------------------------------------------------------------------------
#             Make the Shared library (libOmxAmrDec)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

libOmxAmrDec-inc        := $(LOCAL_PATH)/inc
TARGET_INCLUDE_LIBION := false
ifeq ($(call is-board-platform-in-list,msmnile sdmshrike $(MSMSTEPPE) $(TRINKET) kona lahaina lito bengal holi),true)
TARGET_INCLUDE_LIBION := true
endif
ifeq ($(call is-board-platform-in-list,sdm660),true)
     ifeq ($(TARGET_KERNEL_VERSION), 4.19)
          TARGET_INCLUDE_LIBION := true
     endif
endif
ifeq ($(TARGET_INCLUDE_LIBION),true)
include $(LIBION_HEADER_PATH_WRAPPER)
libOmxAmrDec-inc += $(LIBION_HEADER_PATHS)
endif
libOmxAmrDec-inc        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
libOmxAmrDec-inc        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/audio
libOmxAmrDec-inc	+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/techpack/audio/include
libOmxAmrDec-inc        += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_HEADER_LIBRARIES := libomxcore_headers
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DLKM)),true)
  LOCAL_HEADER_LIBRARIES += audio_kernel_headers
  libOmxAmrDec-inc += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include
endif

ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
  LOCAL_HEADER_LIBRARIES += libutils_headers
endif

LOCAL_MODULE            := libOmxAmrDec
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libOmxAmrDec-def)
LOCAL_C_INCLUDES        := $(libOmxAmrDec-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libutils liblog
ifeq ($(TARGET_INCLUDE_LIBION),true)
LOCAL_SHARED_LIBRARIES  += libion
endif

LOCAL_SRC_FILES         := src/adec_svr.c
LOCAL_SRC_FILES         += src/omx_amr_adec.cpp

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(call is-board-platform-in-list,kona lahaina holi),true)
LOCAL_SANITIZE := integer_overflow
endif
include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#             Make the apps-test (mm-adec-omxamr-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

mm-amr-dec-test-inc     := $(LOCAL_PATH)/inc
mm-amr-dec-test-inc     += $(LOCAL_PATH)/test
mm-amr-dec-test-inc     += $(TARGET_OUT_HEADERS)/common/inc
mm-amr-dec-test-inc     += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
mm-amr-dec-test-inc     += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/audio
mm-amr-dec-test-inc     += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/techpack/audio/include
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DLKM)),true)
  LOCAL_HEADER_LIBRARIES := audio_kernel_headers
  mm-amr-dec-test-inc += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include
endif

LOCAL_MODULE            := mm-adec-omxamr-test
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libOmxAmrDec-def)
LOCAL_C_INCLUDES        := $(mm-amr-dec-test-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libmm-omxcore
LOCAL_SHARED_LIBRARIES  += libOmxAmrDec

ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
LOCAL_PROPRIETARY_MODULE := true
LOCAL_HEADER_LIBRARIES += libutils_headers
endif

LOCAL_SRC_FILES         := test/omx_amr_dec_test.c

ifeq ($(call is-board-platform-in-list,kona lahaina holi),true)
LOCAL_SANITIZE := integer_overflow
endif
include $(BUILD_EXECUTABLE)

endif #BUILD_TINY_ANDROID
endif #TARGET_BOARD_PLATFORM
endif
# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
