/**************************************************************************

Filename    :   VictoryDisplay.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.hud
{
    
    import com.scaleform.std.events.GameEvent;
    import flash.display.MovieClip;
    import flash.text.TextField;
    import flash.events.Event;
    
    import scaleform.clik.events.ButtonEvent;
    
    import com.scaleform.std.controls.SBButton;
    import com.scaleform.std.events.MapEvent;
    import com.scaleform.std.core.GameData;
    
    import com.scaleform.std.system.SoundSystem;

    public class VictoryDisplay extends MovieClip
    {
        
        public static const LABEL_STATE_OFF:String = "off";
        public static const LABEL_ANIM_OPEN:String = "open";
        public static const LABEL_ANIM_SHOW_BUTTONS:String = "btn";
        public static const LABEL_ANIM_CLOSE:String = "close";
        
        public static const LABEL_STARS_NORM:String = "off";
        public static const LABEL_STARS_ANIM_STAR1:String = "star1";
        public static const LABEL_STARS_ANIM_STAR2:String = "star2";
        public static const LABEL_STARS_ANIM_STAR3:String = "star3";
        
        protected var _starFrame:String = null;
        
    // UI Elements
        public var Victory:MovieClip;
        public var Button2:MovieClip;
        public var Button1:MovieClip;
        
    // Initialization
        public function VictoryDisplay() {
            super();
			
            addEventListener( "openAnimComplete", handleOpenAnimComplete, false, 0, true );
        }
        
        public function open():void {
            gotoAndPlay( LABEL_ANIM_OPEN );
            Victory.addEventListener( Event.COMPLETE, handleVictoryAnimComplete, false, 0, true );
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "VictoryMotion.mp3");
            SoundSystem.playSound(SoundSystem.SOUND_MUSIC, "BGM_Victory.mp3", true );				
        }
        
        public function setData( stars:uint ):void { 
            switch ( stars ) { 
                case 2:
                    _starFrame = LABEL_STARS_ANIM_STAR2;
                    break;
                case 3:
                    _starFrame = LABEL_STARS_ANIM_STAR3;
                    break;
                default:
                    _starFrame = LABEL_STARS_ANIM_STAR1;
                    break;
            }
        }
        
        protected function configureButtons():void {
            Button1.Button.label = "REPLAY";
            Button1.Button.addEventListener( ButtonEvent.CLICK, handleReplayClick, false, 0, true );
            
            Button2.Button.label = "CONTINUE";
            Button2.Button.addEventListener( ButtonEvent.CLICK, handleContinueClick, false, 0, true );
        }
        
        protected function handleReplayClick( be:ButtonEvent ):void {
            stage.dispatchEvent( new GameEvent( GameEvent.LAUNCH_MISSION, true, true, GameData.getCurrentGalaxyId(), GameData.getCurrentMissionId() ) );
        }
        protected function handleContinueClick( be:ButtonEvent ):void {
            stage.dispatchEvent( new GameEvent( GameEvent.OPEN_MAIN_MENU, true, true, GameData.getCurrentGalaxyId(), GameData.getCurrentMissionId() ) );
        }
        
        protected function handleVictoryAnimComplete( e:Event ):void {
            Victory.removeEventListener( Event.COMPLETE, handleVictoryAnimComplete, false );
            gotoAndPlay( LABEL_ANIM_SHOW_BUTTONS );
            configureButtons();
        }
        
        protected function handleOpenAnimComplete( e:Event ):void {
            Victory.gotoAndPlay( _starFrame );		
        }
        
        
    }

}