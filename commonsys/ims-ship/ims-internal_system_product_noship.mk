#IMS_INTERNAL
ifneq ($(ENABLE_HYP), true)

IMS_INTERNAL := IMSAudio
IMS_INTERNAL += libloopbackvtjni
IMS_INTERNAL += com.qualcomm.qti.imsaudio
IMS_INTERNAL += com.qti.vtloopback

PRODUCT_PACKAGES += $(IMS_INTERNAL)

endif
