################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)
#-----------------------------------------------------------
#LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE := libarcsoft_beauty_shot
#LOCAL_PREBUILT_LIBS := libarcsoft_night_shot.so
LOCAL_PREBUILT_LIBS := libarcsoft_panorama_burstcapture.so 
#LOCAL_LDLIBS := -ldl

#LOCAL_PREBUILT_LIBS := mpbase.a
#LOCAL_STATIC_LIBRARIES := mpbase

#-----------------------------------------------------------
include $(BUILD_MULTI_PREBUILT)
#include $(BUILD_SHARED_LIBRARY)

################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))