$(call inherit-product, frameworks/native/build/phone-hdpi-512-dalvik-heap.mk)
$(call inherit-product, device/qcom/msm8916_32/msm8916_32.mk)

#QTIC flag
-include $(QCPATH)/common/config/qtic-config.mk

PRODUCT_NAME := msm8916_32_512
PRODUCT_DEVICE := msm8916_32_512
