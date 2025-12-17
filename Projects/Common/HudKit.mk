
FXHUD_COMMON := Apps/Kits/HUD/Common/FxHUDView.cpp \
                Apps/Kits/HUD/Common/FxMinimapView.cpp \
                Apps/Samples/GameDelegate/FxGameDelegate.cpp

FXHUDDEMO := $(FXHUD_COMMON) \
             Apps/Kits/HUD/HUDDemo/HUDAdapter.cpp \
             Apps/Kits/HUD/HUDDemo/HUDEntityController.cpp \
             Apps/Kits/HUD/HUDDemo/HUDSimulation.cpp \
             Apps/Kits/HUD/HUDDemo/HUDKitDemo.cpp

FXMINIMAPDEMO := $(FXHUD_COMMON) \
                 Apps/Kits/HUD/MinimapDemo/FxMinimapViewOld.cpp \
                 Apps/Kits/HUD/MinimapDemo/MinimapAdapter.cpp \
                 Apps/Kits/HUD/MinimapDemo/MinimapSimulation.cpp \
                 Apps/Kits/HUD/MinimapDemo/MinimapDemo.cpp

INCLUDES += -IApps/Samples/GameDelegate -IApps/Samples/FxPlayer

ifneq ($(findstring kits,$(APPS)),)
$(call BUILD_GFX_REN_APPS,HudDemo,,,$(FXHUDDEMO) $(NEWFXPLAYER_SRCS))
endif