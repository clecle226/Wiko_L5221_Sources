DEVICE_PACKAGE_OVERLAYS := device/tinno/L5221/overlay

TARGET_USES_QCOM_BSP := true
# Disable NFC service for memory Optimization
ifeq ($(TARGET_PRODUCT),msm8909)
TARGET_USES_QCA_NFC := false
endif
ifeq ($(TARGET_USES_QCOM_BSP), true)
# Add QC Video Enhancements flag
TARGET_ENABLE_QC_AV_ENHANCEMENTS := true
endif #TARGET_USES_QCOM_BSP

#Audio PA  TN:peter
TARGET_USES_EXTERN_PA := true

# cts test disable it TN:peter
# "adb shell setprop persist.tinno.debug 1" root phone 
ifeq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
# Shuai.Chen, Date20150728, Modify For Not Root Phone In Project Blu, FCCBLUSA-508, Start
#ifneq ($(PROJECT_NAME),blu_us)
TINNO_ROOT_SU := true
#endif
# Shuai.Chen, Date20150728, Modify For Not Root Phone In Project Blu, FCCBLUSA-508, End
endif

TARGET_COMPILE_WITH_CTS := false

ifeq ($(TARGET_COMPILE_WITH_CTS),true)
TINNO_ROOT_SU := false
endif

#QTIC flag
-include $(QCPATH)/common/config/qtic-config.mk

# media_profiles and media_codecs xmls for msm8909
ifeq ($(TARGET_ENABLE_QC_AV_ENHANCEMENTS), true)
PRODUCT_COPY_FILES += device/qcom/msm8909/media/media_profiles_8909.xml:system/etc/media_profiles.xml \
                      device/qcom/msm8909/media/media_codecs_8909.xml:system/etc/media_codecs.xml
endif

$(call inherit-product, device/qcom/common/common.mk)

PRODUCT_NAME := L5221
PRODUCT_DEVICE := L5221
TARGET_VENDOR := tinno
TARGET_LOGO_SIZE :=fwvga
# BWJ for EBBAL-233 project config start.
ifeq ($(strip $(PROJECT_NAME)),)
PROJECT_NAME := $(strip $(subst ro.project =,,$(shell cat $(PWD)/build.ini |grep ^ro.project)))
ifeq ($(strip $(PROJECT_NAME)),)
  PROJECT_NAME := trunk
  $(warning --PROJECT_NAME---is--null--use--default---trunk!-)
endif
endif

#halezhang start
ifeq ($(PROJECT_NAME),lax_mx)
#  PRODUCT_NAME := LT500
#  PRODUCT_DEVICE := LT500
endif
ifeq ($(PROJECT_NAME),lax_co)
#  PRODUCT_NAME := LT500
#  PRODUCT_DEVICE := LT500
endif
#halezhang end

# inherit-product configs.mk.
# wanglj add for multisim/singlesim config
ifeq ($(strip $(CUST_SIM)),)
    CFG_FILE := vendor/tinno/$(TARGET_PRODUCT)/$(PROJECT_NAME)/configs.mk
else
    $(warning --CUST_SIM--$(CUST_SIM)---)
    CFG_FILE := vendor/tinno/$(TARGET_PRODUCT)/$(PROJECT_NAME)/$(CUST_SIM)/configs.mk
    ifeq ($(wildcard $(CFG_FILE)),)
        CFG_FILE := vendor/tinno/$(TARGET_PRODUCT)/$(PROJECT_NAME)/configs.mk
    endif
endif

# BWJ for FCCELMXA-21 START
ifeq ($(PROJECT_NAME),lax_mx)
  PRODUCT_BRAND := LANIX
  PRODUCT_MODEL  := Ilium LT500
  TARGET_USES_EXTERN_PA := false
endif
ifeq ($(PROJECT_NAME),lax_co)
  PRODUCT_BRAND := LANIX
  PRODUCT_MODEL  := Ilium LT500
  TARGET_USES_EXTERN_PA := false
endif
# BWJ for FCCELMXA-21 END

#add blu_us pa config
ifeq ($(PROJECT_NAME),blu_us)
  TARGET_USES_EXTERN_PA := false
endif

#add mob_sa pa config
ifeq ($(PROJECT_NAME),mob_sa)
  TARGET_USES_EXTERN_PA := false
endif
# wlj for FCCBLFRA-1 start
ifeq ($(PROJECT_NAME),wik_fr)
  PRODUCT_BRAND := WIKO
  TARGET_USES_EXTERN_PA := false
endif
# wlj for FCCBLFRA-1 end

$(warning --CFG_FILE--$(CFG_FILE)---)
$(call inherit-product-if-exists, $(CFG_FILE))

PROJECT_FLAGS := $(shell echo CONFIG_PROJECT_$(TARGET_PRODUCT)_$(PROJECT_NAME) | tr a-z A-Z)

# Create build.ini,full building info.
$(shell rm -rf $(PWD)/build.ini)
$(shell echo PLATFORM_VERSION = $(PLATFORM_VERSION) >> $(PWD)/build.ini)
$(shell echo TARGET_BUILD_VARIANT = $(TARGET_BUILD_VARIANT) >> $(PWD)/build.ini)
$(shell echo PROJECT_FLAGS = $(PROJECT_FLAGS) >> $(PWD)/build.ini)
$(shell echo ro.project = $(PROJECT_NAME) >> $(PWD)/build.ini)
$(shell echo ro.target = $(TARGET_PRODUCT) >> $(PWD)/build.ini)
# BWJ for EBBAL-233 project config end.

#bingqian.tang date20150811 add for FCCELMXA-525. in mx and co, need remove REVERIE
ifneq ($(strip $(PROJECT_NAME)),lax_mx)
ifneq ($(strip $(PROJECT_NAME)),lax_co)
ifeq ($(strip $(TARGET_USES_QTIC)),true)
# font rendering engine feature switch
-include $(QCPATH)/common/config/rendering-engine.mk
ifneq (,$(strip $(wildcard $(PRODUCT_RENDERING_ENGINE_REVLIB))))
    MULTI_LANG_ENGINE := REVERIE
endif
endif
endif
endif

#Android EGL implementation
PRODUCT_PACKAGES += libGLES_android

# Audio configuration file
PRODUCT_COPY_FILES += \
    device/tinno/L5221/audio_policy.conf:system/etc/audio_policy.conf \
    device/tinno/L5221/audio_effects.conf:system/vendor/etc/audio_effects.conf \
    device/tinno/L5221/mixer_paths_qrd_skuh.xml:system/etc/mixer_paths_qrd_skuh.xml \
    device/tinno/L5221/mixer_paths_qrd_skui.xml:system/etc/mixer_paths_qrd_skui.xml \
    device/tinno/L5221/mixer_paths_msm8909_pm8916.xml:system/etc/mixer_paths_msm8909_pm8916.xml \
    device/tinno/L5221/mixer_paths_skua.xml:system/etc/mixer_paths_skua.xml \
    device/tinno/L5221/mixer_paths_skuc.xml:system/etc/mixer_paths_skuc.xml \
    device/tinno/L5221/mixer_paths_skue.xml:system/etc/mixer_paths_skue.xml \
    device/tinno/L5221/sound_trigger_mixer_paths.xml:system/etc/sound_trigger_mixer_paths.xml \
    device/tinno/L5221/sound_trigger_platform_info.xml:system/etc/sound_trigger_platform_info.xml \
    device/tinno/L5221/logd-catch.sh:system/bin/logd-catch.sh \
    device/tinno/L5221/apedata_mount.sh:system/bin/apedata_mount.sh

ifeq ($(TARGET_USES_EXTERN_PA),true)
PRODUCT_COPY_FILES += \
    device/tinno/L5221/mixer_paths.xml:system/etc/mixer_paths.xml \
    device/tinno/L5221/ftm_test_config:system/etc/ftm_test_config
else
PRODUCT_COPY_FILES += \
    device/qcom/msm8909/mixer_paths.xml:system/etc/mixer_paths.xml 
endif

#charging animation files
PRODUCT_COPY_FILES += \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/error_battery.bmp:system/media/chg_animation/error_battery.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/logo0.bmp:system/media/chg_animation/logo0.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/logo1.bmp:system/media/chg_animation/logo1.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/logo2.bmp:system/media/chg_animation/logo2.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/logo3.bmp:system/media/chg_animation/logo3.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/logo4.bmp:system/media/chg_animation/logo4.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/logo5.bmp:system/media/chg_animation/logo5.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/num_0.bmp:system/media/chg_animation/num_0.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/num_1.bmp:system/media/chg_animation/num_1.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/num_2.bmp:system/media/chg_animation/num_2.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/num_3.bmp:system/media/chg_animation/num_3.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/num_4.bmp:system/media/chg_animation/num_4.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/num_5.bmp:system/media/chg_animation/num_5.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/num_6.bmp:system/media/chg_animation/num_6.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/num_7.bmp:system/media/chg_animation/num_7.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/num_8.bmp:system/media/chg_animation/num_8.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/num_9.bmp:system/media/chg_animation/num_9.bmp \
      system/core/healthd/chg_animation/$(TARGET_LOGO_SIZE)/percent.bmp:system/media/chg_animation/percent.bmp


# NFC packages
ifeq ($(TARGET_USES_QCA_NFC),true)
NFC_D := true

ifeq ($(NFC_D), true)
    PRODUCT_PACKAGES += \
        libnfcD-nci \
        libnfcD_nci_jni \
        nfc_nci.msm8916 \
        NfcDNci \
        Tag \
        com.android.nfc_extras \
        com.android.nfc.helper \
        SmartcardService \
        org.simalliance.openmobileapi \
        org.simalliance.openmobileapi.xml \
        libassd
else
    PRODUCT_PACKAGES += \
    libnfc-nci \
    libnfc_nci_jni \
    nfc_nci.msm8916 \
    NfcNci \
    Tag \
    com.android.nfc_extras
endif

# file that declares the MIFARE NFC constant
# Commands to migrate prefs from com.android.nfc3 to com.android.nfc
# NFC access control + feature files + configuration
PRODUCT_COPY_FILES += \
        packages/apps/Nfc/migrate_nfc.txt:system/etc/updatecmds/migrate_nfc.txt \
        frameworks/native/data/etc/com.nxp.mifare.xml:system/etc/permissions/com.nxp.mifare.xml \
        frameworks/native/data/etc/com.android.nfc_extras.xml:system/etc/permissions/com.android.nfc_extras.xml \
        frameworks/native/data/etc/android.hardware.nfc.xml:system/etc/permissions/android.hardware.nfc.xml \
        frameworks/native/data/etc/android.hardware.nfc.hce.xml:system/etc/permissions/android.hardware.nfc.hce.xml
# Enable NFC Forum testing by temporarily changing the PRODUCT_BOOT_JARS
# line has to be in sync with build/target/product/core_base.mk
endif # TARGET_USES_QCA_NFC

PRODUCT_BOOT_JARS += qcmediaplayer \
                     WfdCommon \
                     oem-services \
                     qcom.fmradio \
                     org.codeaurora.Performance \
                     vcard \
                     tcmiface

# Listen configuration file
PRODUCT_COPY_FILES += \
    device/qcom/msm8909/listen_platform_info.xml:system/etc/listen_platform_info.xml

# Feature definition files for msm8909
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml

#fstab.qcom
PRODUCT_PACKAGES += fstab.qcom

#Tinno:CJ key
ifeq ($(PROJECT_NAME), wik_fr)
    PRODUCT_DEFAULT_DEV_CERTIFICATE := vendor/tinno/requirment/wik_fr/security/releasekey
else
    PRODUCT_DEFAULT_DEV_CERTIFICATE := build/target/product/security/msm8909/releasekey
endif
#Tinno:CJ ota # BWJ for FCCBLUSA-46 FOTA LINE
#PRODUCT_PACKAGES += TNSystemUpdate
TARGET_RELEASETOOLS_EXTENSIONS :=  device/qcom/common

PRODUCT_PACKAGES += \
    libqcomvisualizer \
    libqcompostprocbundle \
    libqcomvoiceprocessing

#OEM Services library
PRODUCT_PACKAGES += oem-services
PRODUCT_PACKAGES += libsubsystem_control
PRODUCT_PACKAGES += libSubSystemShutdown

PRODUCT_PACKAGES += wcnss_service

#wlan driver
PRODUCT_COPY_FILES += \
    device/tinno/L5221/WCNSS_qcom_cfg.ini:system/etc/wifi/WCNSS_qcom_cfg.ini \
    device/tinno/L5221/WCNSS_wlan_dictionary.dat:persist/WCNSS_wlan_dictionary.dat \
    device/tinno/L5221/WCNSS_qcom_wlan_nv.bin:persist/WCNSS_qcom_wlan_nv.bin \
    device/tinno/L5221/sensors_calibration_params.xml:persist/sensors/sensors_calibration_params.xml

#lcd color manager
PRODUCT_COPY_FILES += \
    device/tinno/L5221/pp_calib_data.bin:persist/display/pp_calib_data.bin


PRODUCT_PACKAGES += \
    wpa_supplicant_overlay.conf \
    p2p_supplicant_overlay.conf
#ANT+ stack
PRODUCT_PACKAGES += \
AntHalService \
libantradio \
antradio_app

#halezhang EBBAL-232 start
QCOM_EXREA_APP := FALSE
QCOM_EXREA_DEBUG_APP := FALSE
#halezhang EBBAL-232 end

QCOM_CABL_APP := true

# Defined the locales
#halezhang FCCELMXA-28 for_default_language start
#PRODUCT_LOCALES += th_TH vi_VN tl_PH hi_IN ar_EG ru_RU tr_TR pt_BR bn_IN mr_IN ta_IN te_IN zh_HK \
#        in_ID my_MM km_KH sw_KE uk_UA pl_PL sr_RS sl_SI fa_IR kn_IN ml_IN ur_IN gu_IN or_IN
#halezhang FCCELMXA-28 for_default_language end

# Add the overlay path
#PRODUCT_PACKAGE_OVERLAYS := $(QCPATH)/qrdplus/Extension/res-overlay \
#        $(QCPATH)/qrdplus/globalization/multi-language/res-overlay \
#        $(PRODUCT_PACKAGE_OVERLAYS)

#SaleTracker TINNO_PT add for sts 20150407
#PRODUCT_PACKAGES += SaleTracker
#PRODUCT_COPY_FILES += \
#       vendor/tinno/tinnoapps/SaleTracker/otherFile/Tracksms:persist/sts/Tracksms \
#       vendor/tinno/tinnoapps/SaleTracker/otherFile/SaleTrackerConfig.xml:system/etc/SaleTrackerConfig.xml \
#       vendor/tinno/tinnoapps/SaleTracker/otherFile/libsltrckrcnfg.so:system/lib/libsltrckrcnfg.so

PRODUCT_PROPERTY_OVERRIDES += drm.service.enabled = true \
                              keyguard.no_require_sim=true \
                              ro.com.google.clientidbase=android-google
PRODUCT_PACKAGES += com.google.widevine.software.drm.xml \
                    com.google.widevine.software.drm \
                    libdrmwvmplugin \
                    libwvm \
                    WidevineSamplePlayer \
                    libwvdrmengine \
                    libWVStreamControlAPI_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL) \
                    libwvdrm_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)
