ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS), true)
ifeq ($(TARGET_USE_BT_DUN),true)
PRODUCT_PACKAGES += dun-server
endif #TARGET_USE_BT_DUN
endif #TARGET_FWK_SUPPORTS_FULL_VALUEADDS
