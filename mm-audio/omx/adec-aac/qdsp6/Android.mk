ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(call is-board-platform-in-list,msm8909 msm8996 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm710 qcs605 sdmshrike msmnile $(MSMSTEPPE) $(TRINKET) kona lahaina holi lito bengal atoll),true)
ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libOmxAacDec-def := -g -O3
libOmxAacDec-def += -DQC_MODIFIED
libOmxAacDec-def += -D_ANDROID_
libOmxAacDec-def += -D_ENABLE_QC_MSG_LOG_
libOmxAacDec-def += -DVERBOSE
libOmxAacDec-def += -D_DEBUG
libOmxAacDec-def += -DAUDIOV2
libOmxAacDec-def += -Wconversion
#ifeq ($(call is-board-platform-in-list,msm8960 msm8974 msm8226 msm8610 copper apq8084 msm8996 msm8998 apq8098_latv sdm845 sdmshrike msmnile $(MSMSTEPPE)),true)
#libOmxAacDec-def += -DMSM_ALSA
#endif
ifeq ($(call is-board-platform-in-list,msm8996 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm710 qcs605 sdmshrike msmnile $(MSMSTEPPE) $(TRINKET) kona lahaina holi lito bengal atoll),true)
libOmxAacDec-def += -DQCOM_AUDIO_USE_SYSTEM_HEAP_ID
endif

# ---------------------------------------------------------------------------------
#             Make the Shared library (libOmxAacDec)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

libOmxAacDec-inc        := $(LOCAL_PATH)/inc
TARGET_INCLUDE_LIBION := false
ifeq ($(call is-board-platform-in-list,msmnile sdmshrike $(MSMSTEPPE) $(TRINKET) kona lahaina lito bengal atoll holi),true)
TARGET_INCLUDE_LIBION := true
endif
ifeq ($(call is-board-platform-in-list,sdm660),true)
      ifeq ($(TARGET_KERNEL_VERSION), 4.19)
            TARGET_INCLUDE_LIBION := true
      endif
endif
ifeq ($(TARGET_INCLUDE_LIBION),true)
include $(LIBION_HEADER_PATH_WRAPPER)
libOmxAacDec-inc += $(LIBION_HEADER_PATHS)
endif
libOmxAacDec-inc        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
libOmxAacDec-inc        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/audio
libOmxAacDec-inc	+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/techpack/audio/include
libOmxAacDec-inc        += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_HEADER_LIBRARIES := libomxcore_headers
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DLKM)),true)
  LOCAL_HEADER_LIBRARIES += audio_kernel_headers
  libOmxAacDec-inc += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include
endif

LOCAL_MODULE            := libOmxAacDec
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libOmxAacDec-def)
LOCAL_C_INCLUDES        := $(libOmxAacDec-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libutils liblog
ifeq ($(TARGET_INCLUDE_LIBION),true)
LOCAL_SHARED_LIBRARIES  += libion
endif

LOCAL_SRC_FILES         := src/adec_svr.c
LOCAL_SRC_FILES         += src/omx_aac_adec.cpp

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(call is-board-platform-in-list,kona lahaina holi),true)
LOCAL_SANITIZE := integer_overflow
endif
include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#             Make the apps-test (mm-adec-omxaac-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

mm-aac-dec-test-inc     := $(LOCAL_PATH)/inc
mm-aac-dec-test-inc     += $(LOCAL_PATH)/test
mm-aac-dec-test-inc     += $(AUDIO_ROOT)/omx/alsa-utils/qdsp6/inc
mm-aac-dec-test-inc     += $(TARGET_OUT_HEADERS)/mm-audio/audio-alsa
mm-aac-dec-test-inc     += $(TARGET_OUT_HEADERS)/mm-audio/omx/alsa-utils/qdsp6/inc
mm-aac-dec-test-inc     += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
mm-aac-dec-test-inc     += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/audio
mm-aac-dec-test-inc     += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/techpack/audio/include
mm-aac-dec-test-inc     += $(TARGET_OUT_HEADERS)/common/inc
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DLKM)),true)
  LOCAL_HEADER_LIBRARIES := audio_kernel_headers
  mm-aac-dec-test-inc += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include
endif

ifeq ($(call is-board-platform,msm8960),true)
mm-aac-dec-test-inc        += $(AUDIO_ROOT)/audio-alsa/inc
mm-aac-dec-test-inc        += $(TARGET_OUT_HEADERS)/mm-audio/libalsa-intf
endif

ifeq ($(call is-board-platform-in-list,msm8974 msm8226 msm8610 copper apq8084 msm8994 msm8992 msm8996 msm8998 sdm660 msmskunk),true)
mm-aac-dec-test-inc        += $(AUDIO_ROOT)/audio-alsa/inc
mm-aac-dec-test-inc        += $(TARGET_OUT_HEADERS)/mm-audio/libalsa-intf
endif

LOCAL_MODULE            := mm-adec-omxaac-test
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libOmxAacDec-def)
LOCAL_C_INCLUDES        := $(mm-aac-dec-test-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libmm-omxcore
LOCAL_SHARED_LIBRARIES  += libOmxAacDec

ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_SRC_FILES         := test/omx_aac_dec_test.c

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
