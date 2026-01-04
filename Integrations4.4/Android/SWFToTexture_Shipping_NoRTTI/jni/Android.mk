LOCAL_PATH := $(call my-dir)

MY_LOCAL_PATH := $(LOCAL_PATH)
SOURCE_PATH := $(SF_ROOT_DIR)
LOCAL_PATH := $(SOURCE_PATH)

include $(MY_LOCAL_PATH)/Lib/Android.mk
include $(MY_LOCAL_PATH)/3rdParty/Android.mk

include $(CLEAR_VARS)
LOCAL_MODULE := $(SF_OUTPUT_MODULE)

LOCAL_STATIC_LIBRARIES := gfxplatform_android_gl gfxrender_gl gfxplatform gfxexpat gfx_air gfx_as2 gfx_as3 gfxexpat curl gfx gfx_as2 z png jpeg expat pcre curl

ifeq ($(SF_ENABLE_SOUND),true)
LOCAL_SHARED_LIBRARIES := fmodex
LOCAL_STATIC_LIBRARIES += gfxsound_fmod
else
LOCAL_STATIC_LIBRARIES += gfxsound_null
endif

LOCAL_C_INCLUDES := $(SOURCE_PATH) $(SOURCE_PATH)/Include $(SOURCE_PATH)/Src $(SOURCE_PATH)/Src/Platform
LOCAL_C_INCLUDES += $(SOURCE_PATH)/Apps/Samples/FxPlayerMobile $(SOURCE_PATH)/Apps/Samples/Common
LOCAL_C_INCLUDES += $(SOURCE_PATH)/3rdParty/expat-2.1.0/lib $(SOURCE_PATH)/3rdParty/expat-2.1.0
LOCAL_C_INCLUDES += $(SOURCE_PATH)/3rdParty/fmod/Android/inc
LOCAL_C_INCLUDES += $(SOURCE_PATH)/3rdParty/jpeg-8d
LOCAL_C_INCLUDES += $(SOURCE_PATH)/3rdParty/libpng-1.5.13
LOCAL_C_INCLUDES += $(SOURCE_PATH)/3rdParty/pcre

LOCAL_LDLIBS := -llog -landroid -lGLESv2 -lEGL
LOCAL_CFLAGS += $(SF_BUILD_DEFINES) -DANDROID -DSF_USE_EGL -fno-exceptions -fno-rtti

LOCAL_SRC_FILES := Src/Platform/Platform_Android_GL.cpp                  \
                   Apps/Samples/RenderTexture/FxPlayerSWFToTextureGL.cpp \
                   Apps/Samples/RenderTexture/FxPlayerSWFToTexture.cpp   \
                   Apps/Samples/Common/FontConfigParser.cpp              \
                   Apps/Samples/Common/FxPlayerAppBase.cpp               \
                   Apps/Samples/Common/FxPlayerAppBaseHUD.cpp            \
                   Apps/Samples/Common/FxPlayerAutotest.cpp              \
                   Apps/Samples/Common/FxRenderThread.cpp                \
                   Apps/Samples/FxPlayer/FxHUD.cpp                       \
                   Apps/Samples/Common/FxSharedObjectManager.cpp         \
                   Apps/Samples/Common/FxAmpAppControlCallback.cpp       \
                   Apps/Samples/Common/FxSoundFMOD.cpp

include $(BUILD_SHARED_LIBRARY)