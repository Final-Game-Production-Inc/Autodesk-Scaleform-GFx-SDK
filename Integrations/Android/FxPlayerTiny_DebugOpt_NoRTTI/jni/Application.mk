#########################
# Scaleform Variables   #
#########################
# Scaleform app name
SF_APP_NAME := FxPlayerTiny
# Scaleform package name
SF_PKG_NAME := com.scaleform
# Scaleform config type
SF_BUILD_CONFIG := DebugOpt_NoRTTI
# Scaleform app ABI
SF_APP_ABI := armeabi-v7a
# Root of Scaleform directory
SF_ROOT_DIR := ../../..
# Scaleform output module
SF_OUTPUT_MODULE := $(SF_APP_NAME)_$(SF_BUILD_CONFIG)
# Scaleform build defines
SF_BUILD_DEFINES := -DSF_ANDROID_NDK_BUILD -DSF_BUILD_DEBUG

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
