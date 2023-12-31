ENABLE_FASTCV := true
ENABLE_SCVE := true
ENABLE_CVP := true
ifeq ($(TARGET_USES_QMAA), true)
ifneq ($(TARGET_USES_QMAA_OVERRIDE_FASTCV), true)
ENABLE_FASTCV := false
endif
ifneq ($(TARGET_USES_QMAA_OVERRIDE_SCVE), true)
ENABLE_SCVE := false
endif
ifneq ($(TARGET_USES_QMAA_OVERRIDE_CVP), true)
ENABLE_CVP := false
endif
endif

#FASTCV
FASTCV_SYSTEM := libapps_mem_heap_system
FASTCV_SYSTEM += libapps_mem_heap_system.so
FASTCV_SYSTEM += libdspCV_skel_system
FASTCV_SYSTEM += libdspCV_skel_system.so
FASTCV_SYSTEM += libfastcvadsp_system
FASTCV_SYSTEM += libfastcvadsp_system.so
FASTCV_SYSTEM += libfastcvadsp_skel_system
FASTCV_SYSTEM += libfastcvadsp_skel_system.so
FASTCV_SYSTEM += libfastcvdsp_skel_system
FASTCV_SYSTEM += libfastcvdsp_skel_system.so
FASTCV_SYSTEM += libfastcvadsp_stub_system
FASTCV_SYSTEM += libfastcvdsp_stub_system
FASTCV_SYSTEM += libfastcvopt_system

FASTCV_INTERNAL_SYSTEM := fastcv_test_oem_system
FASTCV_INTERNAL_SYSTEM += fastcv_test_system
FASTCV_INTERNAL_SYSTEM += fastcv_test_noopencv_system
#SCVE
SCVE_SYSTEM := gModel_system.dat
SCVE_SYSTEM += gModel.dat_system
SCVE_SYSTEM += libobjectMattingApp_skel_system
SCVE_SYSTEM += libobjectMattingApp_skel_system.so
SCVE_SYSTEM += libscveBlobDescriptor_system
SCVE_SYSTEM += libscveBlobDescriptor_skel_system
SCVE_SYSTEM += libscveBlobDescriptor_skel_system.so
SCVE_SYSTEM += libscveBlobDescriptor_stub_system
SCVE_SYSTEM += libscveCleverCapture_system
SCVE_SYSTEM += libscveCleverCapture_skel_system
SCVE_SYSTEM += libscveCleverCapture_skel_system.so
SCVE_SYSTEM += libscveCleverCapture_stub_system
SCVE_SYSTEM += libscveCommon_system
SCVE_SYSTEM += libscveCommon_stub_system
SCVE_SYSTEM += libscveFaceLandmark_skel_system
SCVE_SYSTEM += libscveFaceLandmark_skel_system.so
SCVE_SYSTEM += libscveFaceLandmarks_system
SCVE_SYSTEM += libscveFaceLandmarks_stub_system
SCVE_SYSTEM += libscveFaceRecognition_system
SCVE_SYSTEM += libscveFaceRecognition_skel_system
SCVE_SYSTEM += libscveFaceRecognition_skel_system.so
SCVE_SYSTEM += libscveFaceRecognition_stub_system
SCVE_SYSTEM += libscveImageCloning_system
SCVE_SYSTEM += libscveImageCorrection_system
SCVE_SYSTEM += libscveImageRemoval_system
SCVE_SYSTEM += libscveMotionVector_system
SCVE_SYSTEM += libscveObjectMatting_system
SCVE_SYSTEM += libscveObjectMatting_stub_system
SCVE_SYSTEM += libscveObjectSegmentation_system
SCVE_SYSTEM += libscveObjectSegmentation_skel_system
SCVE_SYSTEM += libscveObjectSegmentation_skel_system.so
SCVE_SYSTEM += libscveObjectSegmentation_stub_system
SCVE_SYSTEM += libscveObjectTracker_system
SCVE_SYSTEM += libscveObjectTracker_stub_system
SCVE_SYSTEM += libscvePanorama_system
SCVE_SYSTEM += libscvePanorama_lite_system
SCVE_SYSTEM += libscveScan3D_system
SCVE_SYSTEM += libhvxMathVIO_system
SCVE_SYSTEM += libhvxMathVIO_system.so
SCVE_SYSTEM += libscveT2T_skel_system
SCVE_SYSTEM += libscveT2T_skel_system.so
SCVE_SYSTEM += libscveTextReco_system
SCVE_SYSTEM += libscveTextReco_skel_system
SCVE_SYSTEM += libscveTextReco_skel_system.so
SCVE_SYSTEM += libscveTextReco_stub_system
SCVE_SYSTEM += libscveTextRecoPostProcessing_system
SCVE_SYSTEM += libscveVideoSummary_system
SCVE_SYSTEM += libscveVideoSummary_skel_system
SCVE_SYSTEM += libscveVideoSummary_skel_system.so
SCVE_SYSTEM += libscveVideoSummary_stub_system
SCVE_SYSTEM += nontextremoval_eng.model
SCVE_SYSTEM += nontextremoval_multilang.model
SCVE_SYSTEM += punRangeData.rst
SCVE_SYSTEM += vendor.qti.hardware.scve.panorama@1.0-adapter-helper
SCVE_SYSTEM += vendor.qti.hardware.scve.panorama@1.0
SCVE_SYSTEM += vendor.qti.hardware.scve.panorama@1.0-halimpl
SCVE_SYSTEM += vendor.qti.hardware.scve.panorama@1.0-impl
SCVE_SYSTEM += vendor.qti.hardware.scve.panorama@1.0-service.rc
SCVE_SYSTEM += vendor.qti.hardware.scve.panorama@1.0-service
SCVE_SYSTEM += vendor.qti.hardware.scve.objecttracker@1.0-adapter-helper
SCVE_SYSTEM += vendor.qti.hardware.scve.objecttracker@1.0
SCVE_SYSTEM += vendor.qti.hardware.scve.objecttracker@1.0-halimpl
SCVE_SYSTEM += vendor.qti.hardware.scve.objecttracker@1.0-impl
SCVE_SYSTEM += vendor.qti.hardware.scve.objecttracker@1.0-service.rc
SCVE_SYSTEM += vendor.qti.hardware.scve.objecttracker@1.0-service
#SCVE TEST BINARIES
SCVE_SYSTEM += scveTestFaceLandmarks_system
SCVE_SYSTEM += scveTestPanorama_system
SCVE_SYSTEM += scveTestObjectTracker_system
SCVE_SYSTEM += scveTestFaceRecognition_system
SCVE_SYSTEM += scveTestImageCorrection_system
SCVE_SYSTEM += scveTestObjectSegmentation_system
SCVE_SYSTEM += scveTestScan3D_system
#CVP LIBRARIES
CVP_SYSTEM := libcvp_system
CVP_SYSTEM += libcvp_system.so
CVP_SYSTEM += libcvp_stub_system
CVP_SYSTEM += libcvp_stub_system.so
CVP_SYSTEM += libcvp_common_system
CVP_SYSTEM += libcvp_common_system.so
CVP_SYSTEM += libcvpcpuRev_skel_system
CVP_SYSTEM += libcvpcpuRev_skel_system.so
CVP_SYSTEM += vendor.qti.hardware.cvp@1.0-adapter-helper
CVP_SYSTEM += vendor.qti.hardware.cvp@1.0
CVP_SYSTEM += libcvp_hidl

# Enable compilation of FASTCV
ifeq ($(ENABLE_FASTCV), true)
PRODUCT_PACKAGES += $(FASTCV_SYSTEM)
PRODUCT_PACKAGES += $(FASTCV_INTERNAL_SYSTEM)
endif

# Enable compilation of SCVE
ifeq ($(ENABLE_SCVE), true)
PRODUCT_PACKAGES += $(SCVE_SYSTEM)
endif

# Enable compilation of CVP
ifeq ($(ENABLE_CVP), true)
PRODUCT_PACKAGES += $(CVP_SYSTEM)
endif

SCVE_TESTS_DEBUG := 3DScanner
SCVE_TESTS_DEBUG += CleverCaptureDemo
SCVE_TESTS_DEBUG += ImageStudioRef
SCVE_TESTS_DEBUG += ObjectTrackerRef
SCVE_TESTS_DEBUG += PanoramaRef
SCVE_TESTS_DEBUG += SocialCamera
SCVE_TESTS_DEBUG += STA
SCVE_TESTS_DEBUG += Touch2Track
SCVE_TESTS_DEBUG += VideoSummaryRef
PRODUCT_PACKAGES_DEBUG += $(SCVE_TESTS_DEBUG)

#INTERNAL ONLY SCVE APK COMPILATION
SCVE_APK_ENABLE := true
#CAMERA JNI LIBS
ifneq ($(TARGET_SCVE_DISABLED),true)
MM_CAMERA_SCVE += libjni_trackingfocus
MM_CAMERA_SCVE += libjni_panorama
endif

# Enable compilation of SCVE
ifeq ($(ENABLE_SCVE), true)
PRODUCT_PACKAGES += $(MM_CAMERA_SCVE)
endif
