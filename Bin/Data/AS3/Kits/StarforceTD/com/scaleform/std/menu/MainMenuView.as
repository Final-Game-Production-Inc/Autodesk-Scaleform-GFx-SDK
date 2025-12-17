/**************************************************************************

Filename    :   MainMenuView.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.menu
{
	import com.scaleform.std.system.SoundSystem;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.display.StageScaleMode;
    
    import flash.ui.Multitouch;
    import flash.ui.MultitouchInputMode;
    import flash.events.TouchEvent;
    
    import com.scaleform.std.menu.MissionSelectView;
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.events.LoadingEvent;
    import com.scaleform.std.events.MapEvent    

    public class MainMenuView extends MovieClip 
    {
        
        // UI Scene
        public var MapMenu:MissionSelectView; // The UI for the Mission Select screen.
        public var Map:MapInterface; // The actual map for the Mission Select screen.
        
        public var GalaxyField:MovieClip;
        public var StarField:MovieClip; // @TODO: Pass these to the Map Interface.
        
        public var StarLayerGalaxy:MovieClip;
        public var StarLayerNebula:MovieClip;
        public var Vignette:MovieClip;
        
        public function MainMenuView() {
            super();
            
            GameData.init();
            
            addEventListener( Event.ADDED_TO_STAGE, configUI, false, 0, true );
        }
        
        protected function configUI( e:Event ) {
            removeEventListener( Event.ADDED_TO_STAGE, configUI, false );
            
            stage.addEventListener( MapEvent.MISSION_SELECTED, handleMissionSelected, false, 0, true );
            stage.addEventListener( MapEvent.MISSION_INFO_CLOSED, handleReturnToGalaxy, false, 0, true);
        }
        
        public function open():void {
			//visible = true;
			
            Vignette.mouseEnabled = Vignette.mouseChildren = false;
            
            Map.populateGalaxyStars( StarLayerGalaxy );
            Map.populateNebulaStars( StarLayerNebula );
            
            // @TODO: This should be based on how Starforce.as opens the file.
            Map.showGalaxies( false );
            
            // Start music
            SoundSystem.playSound(SoundSystem.SOUND_MUSIC, "BGM_MapScreen.mp3", true);			
			
            MapMenu.open(); // Open up and highlight the first galaxy.
            Map.open();
        }
        
        public function close():void {
			//visible = false;
			
            MapMenu.close();
            Map.close();
        }
        
        public function handleMissionSelected(e:Event):void {
            Map.visible = false;
            GalaxyField.visible = false;
            StarField.visible = false;
            Vignette.visible = false;
            StarLayerGalaxy.visible = false;
            StarLayerNebula.visible = false;
        }
        public function handleReturnToGalaxy(e:Event):void {
            
            MapMenu.Popup.close(); // This will also set it to visible = false at the end of the animation.
            
            Map.visible = true;
            GalaxyField.visible = true;
            StarField.visible = true;
            Vignette.visible = true;
            StarLayerGalaxy.visible = true;
            StarLayerNebula.visible = true;
        }
        
    }

}