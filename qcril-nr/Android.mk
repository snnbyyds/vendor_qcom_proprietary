ifneq ($(TARGET_NO_TELEPHONY), true)

LOCAL_DIR := $(call my-dir)
QCRIL_NR_DIR := $(LOCAL_DIR)

qcril_cflags := -O0 -g -Wall -Werror -Wno-error=\#warnings
qcril_cppflags := -O0 -g -Wall -Werror -Wno-error=\#warnings
qcril_ldflags += -Wl,--no-allow-shlib-undefined,--unresolved-symbols=report-all

ifeq ($(TARGET_HAS_LOW_RAM),true)
qcril_cflags += -DRIL_FOR_LOW_RAM
qcril_cppflags += -DRIL_FOR_LOW_RAM
endif

# Uncomment the following line to debug CFI crashes
# qcril_sanitize_diag = cfi
#

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
qcril_sanitize := address
qcril_sanitize_diag = address
endif

ifeq ($(ATEL_ENABLE_LLVM_SA),true)
    LLVM_SA_OUTPUT_DIR := $(PRODUCT_OUT)/atel-llvm-sa-results/qcril-nr
    LLVM_SA_FLAG := --compile-and-analyze $(LLVM_SA_OUTPUT_DIR)
    qcril_cflags   += $(LLVM_SA_FLAG)
    qcril_cppflags += $(LLVM_SA_FLAG)
endif

ifeq ($(call is-board-platform-in-list, monaco),true)
qcril_cflags += -DRIL_VTS_ISSUES
qcril_cppflags += -DRIL_VTS_ISSUES
endif

include $(call all-makefiles-under,$(LOCAL_DIR))
endif
