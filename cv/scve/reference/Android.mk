REFERENCE_ROOT := $(call my-dir)

ifeq  ($(SCVE_APK_ENABLE), true)
    ##### Enable only social Camera apk for the qcs605
    ifneq ($(call is-board-platform-in-list,qcs605),true)
       #include $(REFERENCE_ROOT)/ObjectTracker/Android.mk
       include $(REFERENCE_ROOT)/Panorama/Android.mk
       include $(REFERENCE_ROOT)/Touch2Track/Android.mk
    endif
    #include $(REFERENCE_ROOT)/SocialCamera/Android.mk

    SCVE_MAKE_SCAN3D_TARGETS := sdm845
    ifeq ($(call is-board-platform-in-list,$(SCVE_MAKE_SCAN3D_TARGETS)),true)
    # Uncomment this line below after downloading depth driver libraries.
    #   include $(REFERENCE_ROOT)/3DScanner/Android.mk
    endif
endif
