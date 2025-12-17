/**************************************************************************

Filename    :   MissionSelectView.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.menu
{
    import com.scaleform.std.controls.SBButton;
    import com.scaleform.std.events.GameEvent;
	import com.scaleform.std.events.MainMenuEvent;
	import com.scaleform.std.events.SystemEvent;
	import com.scaleform.std.hud.PauseDisplay;
    import com.scaleform.std.system.SaveSystem;
    import com.scaleform.std.system.SoundSystem;
    import com.scaleform.std.utils.Utils;
	import flash.display.DisplayObjectContainer;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    
    import flash.ui.Multitouch;
    import flash.ui.MultitouchInputMode;
    import flash.events.TouchEvent;
    
    import com.scaleform.std.menu.MissionSelectTop;
    import com.scaleform.std.menu.MissionSelectBottom;
    import com.scaleform.std.events.MapEvent;
    
    import com.scaleform.std.core.GameData;
    
    import scaleform.clik.events.ButtonEvent;

    public class MissionSelectView extends MovieClip 
    {
        // Animation labels
        public static const LABEL_ANIMATION_OFF:String = "off";
        public static const LABEL_ANIMATION_OPEN:String = "open";
        public static const LABEL_ANIMATION_CLOSE:String = "close";
        
        // Animation labels for GalaxyButtonA (GalaxyButton Container)
        public static const LABEL_ANIMATION_GALAXYBTN_OFF:String = "off";
        public static const LABEL_ANIMATION_GALAXYBTN_OPEN:String = "open";
        public static const LABEL_ANIMATION_GALAXYBTN_CLOSE:String = "close";
        
        // UI Scene
        public var TopMenu:MissionSelectTop; // The top of the Mission Select UI. Logo, back, sound, music, score.
        public var BottomMenu:MissionSelectBottom; // The bottom of the Mission Select UI. Research, database, medals.
        
        public var GalaxyButtonA:MovieClip; // Bottom right container for button that enters the selected galaxy.
        public var GalaxyButton:SBButton; // Button with GalaxyButtonA container for entering selected galaxy.
        
		public var AboutWindow:MovieClip; // About window. 
        public var Popup:MissionInfoContainer; // The container that contains the Mission Info popup. 
        public var MissionInfo:MissionInfoView; // The window that displays detailed information about the selected mission.
        
        protected var _showingGalaxy:Boolean = false; // Whether we're showing the galaxy or a nebula.
        protected var _focusedGalaxyId:int = 0; // Which galaxy is currently focused.
        protected var _focusedMissionId:int = 0; // Which nebula is currently focused.
		
		protected var _pauseDisplay:PauseDisplay;
        
        public function MissionSelectView() {
            super();
            
            // Cache references to children's children.
            GalaxyButton = GalaxyButtonA.GalaxyButton.BtnGalaxy as SBButton;
            MissionInfo = Popup.MissionInfo;
			
            addEventListener( Event.ADDED_TO_STAGE, configUI, false, 0, true );
        }
        
        protected function configUI( e:Event ) {
            removeEventListener( Event.ADDED_TO_STAGE, configUI, false );
            
        }
        
        public function open():void {
            if (GameData.getShowMenuTutorial()) {
                GameData.setShowMenuTutorial(false);                
                dispatchEvent(new GameEvent(GameEvent.SHOW_MENU_TUTORIAL, true, false, 0, 0, open));
            } else {
                gotoAndPlay( LABEL_ANIMATION_OPEN );
                configureMenu();
            }
        }
        
        public function close():void {
            gotoAndPlay( LABEL_ANIMATION_CLOSE );
        }
        
        protected function configureMenu():void {
            // _showingGalaxy = focusedMission > 0; // If they provided a nebula, we'll focus that within a galaxy.
            
            // Test Data
            //////////////////////////////
            // BottomMenu.BtnResearch.enabled = false;
            BottomMenu.BtnResearch.setAvailableSP( 0 );
            
            // BottomMenu.BtnDatabase.enabled = false;
            BottomMenu.BtnDatabase.setAvailableSP( 0 );
            
            // BottomMenu.BtnMedals.enabled = false;
            BottomMenu.BtnMedals.setAvailableSP( 0 );
            
            TopMenu.TitleText.text = "The Milky Way";
            TopMenu.StarPointText.text = GameData.getNumStars() + "/" + GameData.getMaxStars();
            //////////////////////////////
            
            // Top Menu
			TopMenu.BtnSettings.addEventListener( ButtonEvent.CLICK, handleSettingsButtonPress, false, 0, true);
            TopMenu.BtnTips.addEventListener( ButtonEvent.CLICK, handleShowTutorial, false, 0, true);
            
            // Bottom Menu
            BottomMenu.BtnResearch.addEventListener( ButtonEvent.CLICK, handleResearchButtonPress, false, 0, true );
            BottomMenu.BtnDatabase.addEventListener( ButtonEvent.CLICK, handleDatabaseButtonPress, false, 0, true );
            BottomMenu.BtnMedals.addEventListener  ( ButtonEvent.CLICK, handleMedalsButtonPress, false, 0, true );
         
            // Galaxy Button
            GalaxyButton.addEventListener( ButtonEvent.CLICK, handleGalaxyButtonPress, false, 0, true );
            GalaxyButtonA.visible = false;
            GalaxyButtonA.gotoAndPlay("close");
            
            // Mission Info
            MissionInfo.BtnClose.addEventListener( ButtonEvent.CLICK, handleMissionClosePress, false, 0, true );
            
            // 
            stage.addEventListener( MapEvent.MISSION_SELECTED, handleMissionSelected, false, 0, true );
            stage.addEventListener( MapEvent.GALAXY_SELECTED, handleGalaxySelected, false, 0, true );
            
            //stage.addEventListener( MapEvent.GALAXY_TRANSITION_COMPLETE, handleGalaxyTransitionComplete, false, 0, true );
            stage.addEventListener( MapEvent.MISSION_TRANISITION_COMPLETE, handleMissionTransitionComplete, false, 0, true );
        }
        
        protected function handleMissionTransitionComplete( me:MapEvent ):void {
            GalaxyButtonA.mouseChildren = GalaxyButtonA.mouseEnabled = true;
            GalaxyButtonA.gotoAndPlay("open");
            GalaxyButton.visible = true;
        }
        
        protected function handleMissionSelected( me:MapEvent ):void {
            var mId:uint = me.missionId;
            _focusedMissionId = mId;
            openMissionInfo( mId );
            
            // TopMenu.TitleText.text = GameData.getMissionSystemName( mId );
            // TopMenu.StarPointText.text = GameData.getMiss() + "/15";
            
            GalaxyButtonA.mouseChildren = GalaxyButtonA.mouseEnabled = false;
            GalaxyButtonA.gotoAndPlay("close");
            // GalaxyButton.visible = false;
        }
        
        protected function handleGalaxySelected( me:MapEvent ):void {
            var gId:uint = me.galaxyId;
            _focusedGalaxyId = gId;
            
            TopMenu.TitleText.text = GameData.getGalaxyName( gId ); // GameData.getGalaxyName( galaxyId );
            TopMenu.StarPointText.text = GameData.getGalaxyNumStars( gId ) + "/" + GameData.getGalaxyMaxStars( gId )
        }
        
        public function openGalaxy( gidx:uint ):void {
            _showingGalaxy = false;
            // Opens the galaxy at Galaxies[ idx ];
        }
        public function closeGalaxy( gix:uint ):void {
            _showingGalaxy = true;
            // Returns to galaxy browser.
        }
        
        public function openMissionInfo( mId:uint ):void {
            // Opens the mission at Missions[ idx ];
            MissionInfo.showMissionData( _focusedGalaxyId, mId );
            Popup.open();
        }
        public function closeMissionInfo():void {
            SoundSystem.playSound(SoundSystem.SOUND_FX, "PlanetPopupClose.mp3");
            dispatchEvent(new MapEvent(MapEvent.MISSION_INFO_CLOSED));
        }
        
        protected function handleMissionClosePress( be:ButtonEvent ):void {
            closeMissionInfo();
            GalaxyButtonA.mouseChildren = GalaxyButtonA.mouseEnabled = true;
            GalaxyButtonA.gotoAndPlay("open");
        }
        
        // Top Menu
		protected function handleSettingsButtonPress( be:ButtonEvent ):void {
			if ( _pauseDisplay == null ) { 
                _pauseDisplay = new PauseDisplay();
            }
            _pauseDisplay.addEventListener( Event.COMPLETE, handlePauseMenuClose, false, 0, true );
            _pauseDisplay.x = (stage.stageWidth / 2) + ((1280 - stage.stageWidth) / 2);
            _pauseDisplay.y = (stage.stageHeight / 2) + ((720 - stage.stageHeight) / 2);
            (root as DisplayObjectContainer).addChild( _pauseDisplay ); // Add it on top of everything.
			
            _pauseDisplay.open("menu");			
		}
        protected function handleShowTutorial( e:Event ):void {
            dispatchEvent(new GameEvent(GameEvent.SHOW_MENU_TUTORIAL, true, false, 0, 0, null));
        }
		protected function handlePauseMenuClose( e:Event ):void {
            _pauseDisplay.removeEventListener( Event.COMPLETE, handlePauseMenuClose );
            (root as DisplayObjectContainer).removeChild( _pauseDisplay );
//            dispatchEvent( new SystemEvent( SystemEvent.RESUME_GAME ) );
        }
        
        // Bottom Menu
        protected function handleResearchButtonPress( be:ButtonEvent ):void {
            // Toggle Research Menu
        }
        protected function handleDatabaseButtonPress( be:ButtonEvent ):void {
            // Toggle Database Menu
        }
        protected function handleMedalsButtonPress( be:ButtonEvent ):void {
            // Toggle Medals Menu
        }
        
        protected function handleGalaxyButtonPress( be:ButtonEvent ):void {
            // Return to Galaxy
            TopMenu.TitleText.text = "The Milky Way";
            TopMenu.StarPointText.text = GameData.getNumStars() + "/" + GameData.getMaxStars();
            
            dispatchEvent( new MapEvent( MapEvent.RETURN_TO_GALAX_MAP ) );
            GalaxyButtonA.mouseChildren = GalaxyButtonA.mouseEnabled = false;
            GalaxyButtonA.gotoAndPlay("close");
        }
    }

}