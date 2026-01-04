#########################
# Scaleform Variables   #
#########################
# Scaleform app name
SF_APP_NAME := SWFToTexture
# Scaleform package name
SF_PKG_NAME := com.scaleform
# Scaleform config type
SF_BUILD_CONFIG := Debug_NoRTTI
# Scaleform sound support (enabled if set to 'true', otherwise disabled)
SF_ENABLE_SOUND := true
# Scaleform app ABI
SF_APP_ABI := armeabi-v7a
# Root of Scaleform directory
SF_ROOT_DIR := ../../..
# Scaleform output module
SF_OUTPUT_MODULE := $(SF_APP_NAME)_$(SF_BUILD_CONFIG)
# Scaleform build defines
SF_BUILD_DEFINES := -DSF_ANDROID_NDK_BUILD -DSF_BUILD_DEBUG
ifeq ($(SF_ENABLE_SOUND),true)
SF_BUILD_DEFINES += -DGFX_ENABLE_SOUND -DGFX_SOUND_FMOD
endif
SF_BUILD_DEFINES += '-DSF_PLATFORM_SYSTEM_APP_ARGS(AppPackageId,SystemClass,AppClass,args)=SF_ANDROID_JNI(SystemClass,AppClass,com_scaleform_$(subst _,_1,$(SF_OUTPUT_MODULE)), MainActivity, args)'
SF_BUILD_DEFINES += '-DSF_ANDROID_APP_PACKAGE="$(SF_PKG_NAME).$(SF_OUTPUT_MODULE)"'
SF_BUILD_DEFINES += '-DSF_ANDROID_CONTENT_DIR="/sdcard/"'

#########################
# Application Variables #
#########################
# Module(s) to build
APP_MODULE := $(SF_OUTPUT_MODULE) 
# Optimization level for modules (debug or release)
APP_OPTIM := debug
# Target ABI (armeabi-v7a or x86)
APP_ABI := $(SF_APP_ABI)
# Use the gnustl 
APP_STL := gnustl_static
# Disable C++ exception support
APP_CFLAGS += -fno-exceptions
# Disable run-time type information (Scaleform is pre-compiled with no-rtti)
APP_CFLAGS += -fno-rtti
# Set function visibility to hidden for Shipping config
# Note: don't enable this for FxPlayerTiny as it will strip native_app_glue
#APP_CFLAGS += -fvisibility=hidden
