FXMENU_STATES := Apps/Kits/Menu/States/GameState_InGame.cpp \
				 Apps/Kits/Menu/States/GameState_Loading.cpp \
				 Apps/Kits/Menu/States/GameState_MainMenu.cpp \
				 Apps/Kits/Menu/States/GameState_Splash.cpp \
				 Apps/Kits/Menu/States/GameState_Start.cpp \
				 Apps/Kits/Menu/States/GameState_System.cpp \
				 Apps/Kits/Menu/States/GameStateWidgetSupport.cpp
            
FXMENU_UTIL :=  Apps/Kits/Menu/Util/GameMemoryFileCache.cpp \
				Apps/Kits/Menu/Util/GameDataLoader.cpp \
			   
FXMENU_VIEWS := Apps/Kits/Menu/Views/MainMenuView.cpp \
				Apps/Kits/Menu/Views/ManagerView.cpp \
				Apps/Kits/Menu/Views/MenuDialog.cpp \
				Apps/Kits/Menu/Views/MenuView.cpp \
				Apps/Kits/Menu/Views/SettingsView.cpp \
				Apps/Kits/Menu/Views/StartGameView.cpp \
				Apps/Kits/Menu/Views/SystemUIDialog.cpp \
				Apps/Kits/Menu/Views/UIView.cpp

FXMENU_DEMO := 	$(FXMENU_STATES) \
				$(FXMENU_UTIL) \
				$(FXMENU_VIEWS) \
				Apps/Kits/Menu/Game.cpp \
				Apps/Kits/Menu/GameResourceManager.cpp \
				Apps/Kits/Menu/GameUIManager.cpp \
				Apps/Kits/Menu/MenuKitDemo.cpp

                 

INCLUDES +=  -IApps/Demos/FxPlayer

ifneq ($(findstring kits,$(APPS)),)
$(call BUILD_GFX_REN_APPS,MenuDemo,,,$(FXMENU_DEMO) $(NEWFXPLAYER_SRCS))
endif