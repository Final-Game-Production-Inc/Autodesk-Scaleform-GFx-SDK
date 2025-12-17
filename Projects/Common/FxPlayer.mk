
INCLUDES += -IApps/Samples/Common

ifneq ($(findstring desktop,$(APPS)),)
$(call BUILD_GFX_REN_APPS,FxPlayer,,,Apps/Samples/FxPlayer/FxPlayer.cpp $(NEWFXPLAYER_SRCS))
endif

ifneq ($(findstring mobile,$(APPS)),)
$(call BUILD_GFX_REN_APPS,FxPlayerMobile,,,Apps/Samples/FxPlayerMobile/FxPlayerMobile.cpp \
         Apps/Samples/Common/FxRenderThread.cpp Apps/Samples/Common/FxSoundFMOD.cpp \
         Apps/Samples/Common/FxSharedObjectManager.cpp)
endif

