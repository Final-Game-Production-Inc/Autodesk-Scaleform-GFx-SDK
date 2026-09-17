LOCAL_PATH := $(SOURCE_PATH)

ifeq ($(SF_ENABLE_SOUND),true)
include $(CLEAR_VARS)
LOCAL_MODULE    := libfmodex
LOCAL_SRC_FILES := 3rdParty/fmod/Android/lib/$(APP_ABI)/libfmodex.so
include $(PREBUILT_SHARED_LIBRARY)
endif
include $(CLEAR_VARS)
LOCAL_MODULE    := curl
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/libcurl.a
include $(PREBUILT_STATIC_LIBRARY) 
include $(CLEAR_VARS)
LOCAL_MODULE    := expat
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/libexpat.a
include $(PREBUILT_STATIC_LIBRARY) 
include $(CLEAR_VARS)
LOCAL_MODULE    := jpeg
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/libjpeg.a
include $(PREBUILT_STATIC_LIBRARY) 
include $(CLEAR_VARS)
LOCAL_MODULE    := png
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/libpng.a
include $(PREBUILT_STATIC_LIBRARY) 
include $(CLEAR_VARS)
LOCAL_MODULE    := z
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/libz.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE    := pcre
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/libpcre.a
include $(PREBUILT_STATIC_LIBRARY) 
include $(CLEAR_VARS)