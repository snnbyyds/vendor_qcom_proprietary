## BoardConfigVendor.mk
## Qualcomm Technologies, Inc proprietary product specific compile-time definitions.
#
-include vendor/qcom/proprietary/common/create_files.mk

USE_CAMERA_STUB := false

## SECIMAGE_DUAL_SIGNING flag to enable the dual signing (RSA and ECC) of abl image.
## setting this to true will generate legacy abl.elf with RSA signing and abl_ecc.elf
## with ECC signing.
## Target using only RSA either set this flag to false or don't define it.
## Other can use this flag to enable the dual signing of their image or fw.

SECIMAGE_DUAL_SIGNING := true

##SECIMAGE tool feature flags
USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN := 1
USES_SEC_POLICY_INTEGRITY_CHECK := 1
USE_SOC_HW_VERSION := true
SOC_HW_VERSION := 0x90020100
SOC_VERS := 0x9002

## SOC_HW_VERSION_ECC & SOC_VERS_ECC defined for Scuba.
## These are used for ECC signing when SECIMAGE_DUAL_SIGNING is set to true.
## Other target with requirement to generate dual signed images can use these Macro.

SOC_HW_VERSION_ECC := 0x90030100
SOC_VERS_ECC := 0x9003

#Flags for generating signed images
USESECIMAGETOOL := true
QTI_GENSECIMAGE_MSM_IDS := monaco # Needs update for Monaco
QTI_GENSECIMAGE_SIGNED_DEFAULT := monaco # Needs update for Monaco
#USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN := 1
#USES_SEC_POLICY_INTEGRITY_CHECK := 1
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_QCOM := true
HAVE_ADRENO_FIRMWARE := true

## wlan flags
BOARD_HAS_QCOM_WLAN := true
BOARD_HAS_QCOM_WIGIG := true
TARGET_USES_ICNSS_QMI := true
BOARD_HAS_ATH_WLAN_AR6320 := true
BOARD_WLAN_DEVICE := qcwcn
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_HOSTAPD_DRIVER := NL80211
WIFI_DRIVER_BUILT := qca_cld3
WIFI_DRIVER_DEFAULT := qca_cld3
WIFI_DRIVER_INSTALL_TO_KERNEL_OUT := true
WPA_SUPPLICANT_VERSION := VER_0_8_X
HOSTAPD_VERSION := VER_0_8_X
CONFIG_ACS := true
CONFIG_IEEE80211AC := true
BOARD_HAS_CFG80211_KERNEL3_4 := true
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
