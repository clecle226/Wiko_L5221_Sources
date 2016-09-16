$(call inherit-product, frameworks/native/build/phone-hdpi-512-dalvik-heap.mk)
$(call inherit-product, device/qcom/msm8909/msm8909.mk)

#QTIC flag
-include $(QCPATH)/common/config/qtic-config.mk

PRODUCT_NAME := msm8909_512
PRODUCT_DEVICE := msm8909_512
