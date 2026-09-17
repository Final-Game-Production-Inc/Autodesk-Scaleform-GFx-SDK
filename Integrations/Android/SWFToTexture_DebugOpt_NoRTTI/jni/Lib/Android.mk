LOCAL_PATH := $(SOURCE_PATH)

include $(CLEAR_VARS)
LOCAL_MODULE    := gfx
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/$(SF_BUILD_CONFIG)/libgfx.a
include $(PREBUILT_STATIC_LIBRARY) 
include $(CLEAR_VARS)
LOCAL_MODULE    := gfx_as2
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/$(SF_BUILD_CONFIG)/libgfx_as2.a
include $(PREBUILT_STATIC_LIBRARY) 
include $(CLEAR_VARS)
LOCAL_MODULE    := gfx_as3
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/$(SF_BUILD_CONFIG)/libgfx_as3.a
include $(PREBUILT_STATIC_LIBRARY) 
include $(CLEAR_VARS)
LOCAL_MODULE    := gfx_air
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/$(SF_BUILD_CONFIG)/libgfx_air.a
include $(PREBUILT_STATIC_LIBRARY) 
include $(CLEAR_VARS)
LOCAL_MODULE    := gfxexpat
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/$(SF_BUILD_CONFIG)/libgfxexpat.a
include $(PREBUILT_STATIC_LIBRARY) 
include $(CLEAR_VARS)
LOCAL_MODULE    := gfxplatform
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/$(SF_BUILD_CONFIG)/libgfxplatform.a
include $(PREBUILT_STATIC_LIBRARY) 
include $(CLEAR_VARS)
LOCAL_MODULE    := gfxrender_gl
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/$(SF_BUILD_CONFIG)/libgfxrender_gl.a
include $(PREBUILT_STATIC_LIBRARY) 
include $(CLEAR_VARS)
LOCAL_MODULE    := gfxsound_fmod
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/$(SF_BUILD_CONFIG)/libgfxsound_fmod.a
include $(PREBUILT_STATIC_LIBRARY) 
include $(CLEAR_VARS)
LOCAL_MODULE    := gfxsound_null
LOCAL_SRC_FILES := Lib/Android/$(APP_ABI)/$(SF_BUILD_CONFIG)/libgfxsound_null.a
include $(PREBUILT_STATIC_LIBRARY) 

