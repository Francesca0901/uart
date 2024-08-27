LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := sender_android
LOCAL_SRC_FILES := sender_android.c

LOCAL_SHARED_LIBRARIES := liblog

include $(BUILD_EXECUTABLE)
