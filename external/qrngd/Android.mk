LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := qrngtest.c
LOCAL_SHARED_LIBRARIES := libc
LOCAL_MODULE = qrngtest
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)
