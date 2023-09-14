PRODUCT_PACKAGES += DeviceStatisticsService
PRODUCT_PACKAGES += DynamicDDSService
PRODUCT_PACKAGES += embms
PRODUCT_PACKAGES += ODLT
PRODUCT_PACKAGES += QCC-AUTHMGR
PRODUCT_PACKAGES += QCC
PRODUCT_PACKAGES += QTIDiagServices
PRODUCT_PACKAGES += remotesimlockservice
PRODUCT_PACKAGES += uimgbaservice
PRODUCT_PACKAGES += uimlpaservice
PRODUCT_PACKAGES += workloadclassifier
PRODUCT_PACKAGES += dpmd
PRODUCT_PACKAGES += qccsyshalservice
PRODUCT_PACKAGES += qspmsvc
PRODUCT_PACKAGES += rtspclient
PRODUCT_PACKAGES += rtspserver
PRODUCT_PACKAGES += sigma_miracasthalservice
PRODUCT_PACKAGES += wfdservice
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/dpm/dpm.conf:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/dpm/dpm.conf
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/init/com.qualcomm.qti.sigma_miracast@1.0-service.rc:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/init/com.qualcomm.qti.sigma_miracast@1.0-service.rc
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/init/dpmd.rc:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/init/dpmd.rc
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/init/qspmsvc.rc:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/init/qspmsvc.rc
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/init/vendor.qti.hardware.qccsyshal@1.0-service.rc:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/init/vendor.qti.hardware.qccsyshal@1.0-service.rc
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/init/wfdservice.rc:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/init/wfdservice.rc
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/perf/wlc_model.tflite:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/perf/wlc_model.tflite
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/permissions/audiosphere.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/permissions/audiosphere.xml
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/permissions/com.qti.dpmframework.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/permissions/com.qti.dpmframework.xml
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/permissions/com.qualcomm.qmapbridge.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/permissions/com.qualcomm.qmapbridge.xml
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/permissions/com.qualcomm.qti.izattools.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/permissions/com.qualcomm.qti.izattools.xml
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/permissions/dpmapi.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/permissions/dpmapi.xml
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/permissions/embms-noship_product_privapp_permissions_qti.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/permissions/embms-noship_product_privapp_permissions_qti.xml
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/permissions/embms.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/permissions/embms.xml
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/permissions/izat.xt.srv.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/permissions/izat.xt.srv.xml
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/etc/permissions/UimService.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/permissions/UimService.xml
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/ActivityExt.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/ActivityExt.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/audiosphere.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/audiosphere.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/com.qti.dpmframework.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/com.qti.dpmframework.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/com.quicinc.cne.api-V1.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/com.quicinc.cne.api-V1.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/com.quicinc.cne.api-V1.1-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/com.quicinc.cne.api-V1.1-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/com.quicinc.cne.constants-V1.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/com.quicinc.cne.constants-V1.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/com.quicinc.cne.constants-V2.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/com.quicinc.cne.constants-V2.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/com.quicinc.cne.constants-V2.1-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/com.quicinc.cne.constants-V2.1-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/dpmapi.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/dpmapi.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/embmslibrary.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/embmslibrary.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/izat.xt.srv.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/izat.xt.srv.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/qmapbridge.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/qmapbridge.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/uimservicelibrary.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/uimservicelibrary.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.data.factory-V1.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.data.factory-V1.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.data.factory-V2.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.data.factory-V2.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.data.factory-V2.1-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.data.factory-V2.1-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.data.factory-V2.2-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.data.factory-V2.2-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.data.factory-V2.3-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.data.factory-V2.3-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.hardware.data.cne.internal.api-V1.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.hardware.data.cne.internal.api-V1.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.hardware.data.cne.internal.constants-V1.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.hardware.data.cne.internal.constants-V1.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.hardware.data.cne.internal.server-V1.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.hardware.data.cne.internal.server-V1.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.hardware.data.qmi-V1.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.hardware.data.qmi-V1.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.hardware.mwqemadapter-V1.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.hardware.mwqemadapter-V1.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.hardware.slmadapter-V1.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.hardware.slmadapter-V1.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.ims.connection-V1.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.ims.connection-V1.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.ims.factory-V1.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.ims.factory-V1.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.ims.factory-V1.1-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.ims.factory-V1.1-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.ims.factory-V2.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.ims.factory-V2.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.ims.factory-V2.1-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.ims.factory-V2.1-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.ims.factory-V2.2-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.ims.factory-V2.2-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.ims.rcssip-V1.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.ims.rcssip-V1.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.ims.rcssip-V1.1-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.ims.rcssip-V1.1-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.ims.rcssip-V1.2-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.ims.rcssip-V1.2-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.ims.rcsuce-V1.0-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.ims.rcsuce-V1.0-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.ims.rcsuce-V1.1-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.ims.rcsuce-V1.1-java.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system_ext/framework/vendor.qti.ims.rcsuce-V1.2-java.jar:$(TARGET_COPY_OUT_SYSTEM_EXT)/framework/vendor.qti.ims.rcsuce-V1.2-java.jar
PRODUCT_PACKAGES += com.qualcomm.qti.dpm.api@1.0
PRODUCT_PACKAGES += com.quicinc.cne.api@1.0
PRODUCT_PACKAGES += com.quicinc.cne.api@1.1
PRODUCT_PACKAGES += com.quicinc.cne.constants@1.0
PRODUCT_PACKAGES += com.quicinc.cne.constants@2.0
PRODUCT_PACKAGES += com.quicinc.cne.constants@2.1
PRODUCT_PACKAGES += vendor.qti.hardware.sigma_miracast@1.0-impl
PRODUCT_PACKAGES += libadsprpc_system
PRODUCT_PACKAGES += libavenhancements
PRODUCT_PACKAGES += libbeluga
PRODUCT_PACKAGES += libbinauralrenderer_wrapper.qti
PRODUCT_PACKAGES += libcdsprpc_system
PRODUCT_PACKAGES += libcomposerextn.qti
PRODUCT_PACKAGES += libcvp_common_system
PRODUCT_PACKAGES += libcvpcpuRev_skel_system
PRODUCT_PACKAGES += libdashdatasource
PRODUCT_PACKAGES += libdashsamplesource
PRODUCT_PACKAGES += libDiagService
PRODUCT_PACKAGES += libdolphin
PRODUCT_PACKAGES += libdpmctmgr
PRODUCT_PACKAGES += libdpmfdmgr
PRODUCT_PACKAGES += libdpmframework
PRODUCT_PACKAGES += libdpmtcm
PRODUCT_PACKAGES += libembmsmmosal
PRODUCT_PACKAGES += libembmsmmparser_lite
PRODUCT_PACKAGES += libembmssqlite
PRODUCT_PACKAGES += libembmstinyxml
PRODUCT_PACKAGES += libframeextension
PRODUCT_PACKAGES += libGPQTEEC_system.qti
PRODUCT_PACKAGES += libGPTEE_system.qti
PRODUCT_PACKAGES += libhoaeffects_csim
PRODUCT_PACKAGES += libhoaeffects.qti
PRODUCT_PACKAGES += lib-imsvtextutils
PRODUCT_PACKAGES += lib-imsvt
PRODUCT_PACKAGES += lib-imsvtutils
PRODUCT_PACKAGES += libjnihelpers
PRODUCT_PACKAGES += liblayerext.qti
PRODUCT_PACKAGES += liblistensoundmodel2.qti
PRODUCT_PACKAGES += libloc2jnibridge
PRODUCT_PACKAGES += libmdsprpc_system
PRODUCT_PACKAGES += libminksocket_system
PRODUCT_PACKAGES += libmink-sock-native-api
PRODUCT_PACKAGES += libmiracastsystem
PRODUCT_PACKAGES += libmmhttpstack
PRODUCT_PACKAGES += libmmiipstreammmihttp
PRODUCT_PACKAGES += libmmipstreamnetwork
PRODUCT_PACKAGES += libmmipstreamsourcehttp
PRODUCT_PACKAGES += libmmipstreamutils
PRODUCT_PACKAGES += libmmparser_lite
PRODUCT_PACKAGES += libmmQSM
PRODUCT_PACKAGES += libmmrtpdecoder
PRODUCT_PACKAGES += libmmrtpencoder
PRODUCT_PACKAGES += libmsp
PRODUCT_PACKAGES += libmwqemiptablemgr
PRODUCT_PACKAGES += libOpenCL_system
PRODUCT_PACKAGES += libqcc_file_agent_sys
PRODUCT_PACKAGES += libqcc
PRODUCT_PACKAGES += libQOC.qti
PRODUCT_PACKAGES += libQSEEComAPI_system
PRODUCT_PACKAGES += libqspmsvc
PRODUCT_PACKAGES += libQTEEConnector_system
PRODUCT_PACKAGES += libqti_workloadclassifiermodel
PRODUCT_PACKAGES += libsdm-disp-apis.qti
PRODUCT_PACKAGES += libsdsprpc_system
PRODUCT_PACKAGES += libseccam
PRODUCT_PACKAGES += libskewknob_system
PRODUCT_PACKAGES += libsmomoconfig.qti
PRODUCT_PACKAGES += libsmomo.qti
PRODUCT_PACKAGES += libthermalclient.qti
PRODUCT_PACKAGES += libtrigger-handler
PRODUCT_PACKAGES += libupdateprof.qti
PRODUCT_PACKAGES += libvr_amb_engine
PRODUCT_PACKAGES += libvraudio_client.qti
PRODUCT_PACKAGES += libvraudio
PRODUCT_PACKAGES += libvr_object_engine
PRODUCT_PACKAGES += libvr_sam_wrapper
PRODUCT_PACKAGES += libwfdclient
PRODUCT_PACKAGES += libwfdcommonutils
PRODUCT_PACKAGES += libwfddisplayconfig
PRODUCT_PACKAGES += libwfdmminterface
PRODUCT_PACKAGES += libwfdmmsink
PRODUCT_PACKAGES += libwfdrtsp
PRODUCT_PACKAGES += libwfdsinksm
PRODUCT_PACKAGES += libwfduibcinterface
PRODUCT_PACKAGES += libwfduibcsinkinterface
PRODUCT_PACKAGES += libwfduibcsink
PRODUCT_PACKAGES += libwfduibcsrcinterface
PRODUCT_PACKAGES += libwfduibcsrc
PRODUCT_PACKAGES += vendor.qti.data.factory@1.0
PRODUCT_PACKAGES += vendor.qti.data.factory@2.0
PRODUCT_PACKAGES += vendor.qti.data.factory@2.1
PRODUCT_PACKAGES += vendor.qti.data.factory@2.2
PRODUCT_PACKAGES += vendor.qti.data.factory@2.3
PRODUCT_PACKAGES += vendor.qti.hardware.data.cne.internal.api@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.data.cne.internal.constants@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.data.cne.internal.server@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.data.qmi@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.embmssl@1.0-adapter-helper
PRODUCT_PACKAGES += vendor.qti.hardware.embmssl@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.embmssl@1.1-adapter-helper
PRODUCT_PACKAGES += vendor.qti.hardware.embmssl@1.1
PRODUCT_PACKAGES += vendor.qti.hardware.mwqemadapter@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.qccsyshal@1.0-halimpl
PRODUCT_PACKAGES += vendor.qti.hardware.qccsyshal@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.qccvndhal@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.sigma_miracast@1.0-halimpl
PRODUCT_PACKAGES += vendor.qti.hardware.sigma_miracast@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.slmadapter@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.wifidisplaysession@1.0
PRODUCT_PACKAGES += vendor.qti.ims.connection@1.0
PRODUCT_PACKAGES += vendor.qti.ims.factory@1.0
PRODUCT_PACKAGES += vendor.qti.ims.factory@1.1
PRODUCT_PACKAGES += vendor.qti.ims.factory@2.0
PRODUCT_PACKAGES += vendor.qti.ims.factory@2.1
PRODUCT_PACKAGES += vendor.qti.ims.factory@2.2
PRODUCT_PACKAGES += vendor.qti.ims.rcssip@1.0
PRODUCT_PACKAGES += vendor.qti.ims.rcssip@1.1
PRODUCT_PACKAGES += vendor.qti.ims.rcssip@1.2
PRODUCT_PACKAGES += vendor.qti.ims.rcsuce@1.0
PRODUCT_PACKAGES += vendor.qti.ims.rcsuce@1.1
PRODUCT_PACKAGES += vendor.qti.ims.rcsuce@1.2
PRODUCT_PACKAGES += vendor.qti.imsrtpservice@3.0
PRODUCT_PACKAGES += com.qualcomm.qti.dpm.api@1.0
PRODUCT_PACKAGES += com.qualcomm.qti.wifidisplayhal@1.0
PRODUCT_PACKAGES += com.quicinc.cne.api@1.0
PRODUCT_PACKAGES += com.quicinc.cne.api@1.1
PRODUCT_PACKAGES += com.quicinc.cne.constants@1.0
PRODUCT_PACKAGES += com.quicinc.cne.constants@2.0
PRODUCT_PACKAGES += com.quicinc.cne.constants@2.1
PRODUCT_PACKAGES += vendor.qti.hardware.sigma_miracast@1.0-impl
PRODUCT_PACKAGES += libadsprpc_system
PRODUCT_PACKAGES += libavenhancements
PRODUCT_PACKAGES += libbeluga
PRODUCT_PACKAGES += libbinauralrenderer_wrapper.qti
PRODUCT_PACKAGES += libcdsprpc_system
PRODUCT_PACKAGES += libcomposerextn.qti
PRODUCT_PACKAGES += libcvp_common_system
PRODUCT_PACKAGES += libcvpcpuRev_skel_system
PRODUCT_PACKAGES += libDiagService
PRODUCT_PACKAGES += libdolphin
PRODUCT_PACKAGES += libdpmctmgr
PRODUCT_PACKAGES += libdpmfdmgr
PRODUCT_PACKAGES += libdpmframework
PRODUCT_PACKAGES += libdpmtcm
PRODUCT_PACKAGES += libframeextension
PRODUCT_PACKAGES += libGPQTEEC_system.qti
PRODUCT_PACKAGES += libGPTEE_system.qti
PRODUCT_PACKAGES += libhoaeffects_csim
PRODUCT_PACKAGES += libhoaeffects.qti
PRODUCT_PACKAGES += lib-imsvtextutils
PRODUCT_PACKAGES += lib-imsvt
PRODUCT_PACKAGES += lib-imsvtutils
PRODUCT_PACKAGES += libjnihelpers
PRODUCT_PACKAGES += liblayerext.qti
PRODUCT_PACKAGES += liblistensoundmodel2.qti
PRODUCT_PACKAGES += libloc2jnibridge
PRODUCT_PACKAGES += libmdsprpc_system
PRODUCT_PACKAGES += libminksocket_system
PRODUCT_PACKAGES += libmink-sock-native-api
PRODUCT_PACKAGES += libmiracastsystem
PRODUCT_PACKAGES += libmmparser_lite
PRODUCT_PACKAGES += libmmrtpdecoder
PRODUCT_PACKAGES += libmmrtpencoder
PRODUCT_PACKAGES += libmwqemiptablemgr
PRODUCT_PACKAGES += libOpenCL_system
PRODUCT_PACKAGES += libqcc_file_agent_sys
PRODUCT_PACKAGES += libqcc
PRODUCT_PACKAGES += libqct_resampler
PRODUCT_PACKAGES += libQSEEComAPI_system
PRODUCT_PACKAGES += libqspmsvc
PRODUCT_PACKAGES += libQTEEConnector_system
PRODUCT_PACKAGES += libqti_workloadclassifiermodel
PRODUCT_PACKAGES += libsdm-disp-apis.qti
PRODUCT_PACKAGES += libsdsprpc_system
PRODUCT_PACKAGES += libseccam
PRODUCT_PACKAGES += libskewknob_system
PRODUCT_PACKAGES += libsmomoconfig.qti
PRODUCT_PACKAGES += libsmomo.qti
PRODUCT_PACKAGES += libthermalclient.qti
PRODUCT_PACKAGES += libtrigger-handler
PRODUCT_PACKAGES += libupdateprof.qti
PRODUCT_PACKAGES += libvr_amb_engine
PRODUCT_PACKAGES += libvraudio_client.qti
PRODUCT_PACKAGES += libvraudio
PRODUCT_PACKAGES += libvr_object_engine
PRODUCT_PACKAGES += libvr_sam_wrapper
PRODUCT_PACKAGES += libwfdclient
PRODUCT_PACKAGES += libwfdcommonutils
PRODUCT_PACKAGES += libwfddisplayconfig
PRODUCT_PACKAGES += libwfdmminterface
PRODUCT_PACKAGES += libwfdmmsink
PRODUCT_PACKAGES += libwfdmmsrc_system
PRODUCT_PACKAGES += libwfdrtsp
PRODUCT_PACKAGES += libwfdservice
PRODUCT_PACKAGES += libwfdsinksm
PRODUCT_PACKAGES += libwfduibcinterface
PRODUCT_PACKAGES += libwfduibcsinkinterface
PRODUCT_PACKAGES += libwfduibcsink
PRODUCT_PACKAGES += libwfduibcsrcinterface
PRODUCT_PACKAGES += libwfduibcsrc
PRODUCT_PACKAGES += vendor.qti.data.factory@1.0
PRODUCT_PACKAGES += vendor.qti.data.factory@2.0
PRODUCT_PACKAGES += vendor.qti.data.factory@2.1
PRODUCT_PACKAGES += vendor.qti.data.factory@2.2
PRODUCT_PACKAGES += vendor.qti.data.factory@2.3
PRODUCT_PACKAGES += vendor.qti.hardware.data.cne.internal.api@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.data.cne.internal.constants@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.data.cne.internal.server@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.data.qmi@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.mwqemadapter@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.qccsyshal@1.0-halimpl
PRODUCT_PACKAGES += vendor.qti.hardware.qccsyshal@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.qccvndhal@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.sigma_miracast@1.0-halimpl
PRODUCT_PACKAGES += vendor.qti.hardware.sigma_miracast@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.slmadapter@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.wifidisplaysession@1.0
PRODUCT_PACKAGES += vendor.qti.ims.connection@1.0
PRODUCT_PACKAGES += vendor.qti.ims.factory@1.0
PRODUCT_PACKAGES += vendor.qti.ims.factory@1.1
PRODUCT_PACKAGES += vendor.qti.ims.factory@2.0
PRODUCT_PACKAGES += vendor.qti.ims.factory@2.1
PRODUCT_PACKAGES += vendor.qti.ims.factory@2.2
PRODUCT_PACKAGES += vendor.qti.ims.rcssip@1.0
PRODUCT_PACKAGES += vendor.qti.ims.rcssip@1.1
PRODUCT_PACKAGES += vendor.qti.ims.rcssip@1.2
PRODUCT_PACKAGES += vendor.qti.ims.rcsuce@1.0
PRODUCT_PACKAGES += vendor.qti.ims.rcsuce@1.1
PRODUCT_PACKAGES += vendor.qti.ims.rcsuce@1.2
PRODUCT_PACKAGES += vendor.qti.imsrtpservice@3.0
PRODUCT_PACKAGES += dpmserviceapp
PRODUCT_PACKAGES += MSDC_UI
PRODUCT_PACKAGES += QAS_DVC_MSP
PRODUCT_PACKAGES += xtra_t_app_setup
PRODUCT_PACKAGES += xtra_t_app
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system/framework/QXPerformance.jar:system/framework/QXPerformance.jar
PRODUCT_COPY_FILES += vendor/qcom/proprietary/prebuilt_HY11/target/product/qssi/system/framework/tcmclient.jar:system/framework/tcmclient.jar
PRODUCT_PACKAGES += vendor.qti.hardware.qccsyshal@1.0-impl
PRODUCT_PACKAGES += vendor.qti.hardware.qccsyshal@1.0-impl