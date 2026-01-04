LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := lib${APP_NAME}
LOCAL_SRC_FILES := ../obj/local/armeabi/lib${APP_NAME}.so

include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libfmodex
LOCAL_SRC_FILES := ../bin/lib/armeabi/libfmodex.so

include $(PREBUILT_SHARED_LIBRARY)
