/**************************************************************************

Filename    :   PauseDisplay.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.hud
{
    
    import com.scaleform.std.events.GameEvent;
	import com.scaleform.std.events.MainMenuEvent;
    import com.scaleform.std.utils.Utils;
    import flash.display.MovieClip;
	import flash.display.Stage;
    import flash.system.Capabilities;
    import flash.text.TextField;
    import flash.events.Event;
    
    import scaleform.clik.events.ButtonEvent;
    
    import com.scaleform.std.controls.SBButton;
    import com.scaleform.std.events.MapEvent;
    import com.scaleform.std.core.GameData;
	import com.scaleform.std.system.SoundSystem;
	import flash.system.System;

    public class PauseDisplay extends MovieClip
    {
        
        public static const LABEL_STATE_OFF:String = "off";
        public static const LABEL_ANIM_OPEN:String = "open";
        public static const LABEL_ANIM_CLOSE:String = "close";
        
    // UI Elements
		public var Button1:MovieClip;
		public var Button2:MovieClip;
		public var Button3:MovieClip;
        public var Button4:MovieClip;
		public var Checkbox1:MovieClip;
		public var Checkbox2:MovieClip;
        //public var Checkbox3:MovieClip;
        
		protected var _stageRef:Stage;
		
    // Initialization
        public function PauseDisplay() {
            super();
        }
        
        public function open(mode:String /*menu or hud*/):void {
            gotoAndPlay( LABEL_ANIM_OPEN );
            
			_stageRef = stage;
            mouseEnabled = mouseChildren = true;
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "PausePopupOpen.mp3");
			
			if (mode == "menu") {
				configureMenuButtons();
			} else { 
				configureHudButtons();
			}
			
			Checkbox1.Checkbox.toggle = true;
			Checkbox1.Checkbox.selected = !SoundSystem.isSoundFxMuted;
			Checkbox1.Checkbox.addEventListener( ButtonEvent.CLICK, handleSoundClick, false, 0, true );			
			Checkbox2.Checkbox.toggle = true;
			Checkbox2.Checkbox.selected = !SoundSystem.isMusicMuted;	
			Checkbox2.Checkbox.addEventListener( ButtonEvent.CLICK, handleMusicClick, false, 0, true );			
            //Checkbox3.Checkbox.toggle = true;
			//Checkbox3.Checkbox.selected = Utils.showTutorial;	
			//Checkbox3.Checkbox.addEventListener( ButtonEvent.CLICK, handleTutorialClick, false, 0, true );
        }

        protected function configureMenuButtons():void {
            Button1.Button.label = "RESUME";
            Button1.Button.addEventListener( ButtonEvent.CLICK, handleResumeClick, false, 0, true );
            
			Button2.Button.label = "TITLE SCREEN";
            Button2.Button.addEventListener( ButtonEvent.CLICK, handleTitleScreen, false, 0, true );
			
            if (Capabilities.os == "iPhone") {
                Button3.Button.label = "ABOUT";
                Button3.Button.addEventListener( ButtonEvent.CLICK, handleAboutClick, false, 0, true );
                Button4.visible = false;
            } else {
                Button3.Button.label = "QUIT";
                Button3.Button.addEventListener( ButtonEvent.CLICK, handleExitGame, false, 0, true );
                
                Button4.Button.label = "ABOUT";
                Button4.Button.addEventListener( ButtonEvent.CLICK, handleAboutClick, false, 0, true );
            }
        }        
		
        protected function configureHudButtons():void {
            Button1.Button.label = "RESUME";
            Button1.Button.addEventListener( ButtonEvent.CLICK, handleResumeClick, false, 0, true );
            
            Button2.Button.label = "REDEPLOY";
            Button2.Button.addEventListener( ButtonEvent.CLICK, handleReplayClick, false, 0, true );
            
            Button3.Button.label = "SURRENDER";
            Button3.Button.addEventListener( ButtonEvent.CLICK, handleSurrenderClick, false, 0, true );
            
            Button4.Button.label = "ABOUT";
            Button4.Button.addEventListener( ButtonEvent.CLICK, handleAboutClick, false, 0, true );
        }
        
        protected function handleAboutClick(e:Event):void {
            dispatchEvent(new GameEvent(GameEvent.SHOW_ABOUT_DIALOG));
        }
        
        protected function handleExitGame():void {
            System.exit( 1 );
        }
		
		protected function handleTitleScreen(e:ButtonEvent):void {
            disableButtons();
			gotoAndPlay( LABEL_ANIM_CLOSE );
			
			addEventListener( Event.COMPLETE, handleTitleScreenImpl, false, 0, true );
		}
		protected function handleTitleScreenImpl(e:Event):void {
			removeEventListener(Event.COMPLETE, handleTitleScreenImpl, false);
			
			_stageRef.dispatchEvent( new MainMenuEvent( MainMenuEvent.MAP_TO_INTRO_TRANSITION, true, true ) );
		}
		
        protected function handleSoundClick( e:ButtonEvent ):void {
            var mute:Boolean = !SoundSystem.isSoundFxMuted;
            
            if (mute) SoundSystem.muteSoundFx();
            else SoundSystem.unmuteSoundFx();
        }
        
        protected function handleMusicClick( e:ButtonEvent ):void {
            var mute:Boolean = !SoundSystem.isMusicMuted;

            if (mute) SoundSystem.muteMusic();
            else SoundSystem.unmuteMusic();
        }		
		
        protected function handleReplayClick( be:ButtonEvent ):void {
            disableButtons();
            _stageRef.dispatchEvent( new GameEvent( GameEvent.LAUNCH_MISSION, true, true, GameData.getCurrentGalaxyId(), GameData.getCurrentMissionId() ) );
        }
        protected function handleSurrenderClick( be:ButtonEvent ):void {
            disableButtons();
            _stageRef.dispatchEvent( new GameEvent( GameEvent.OPEN_MAIN_MENU, true, true, GameData.getCurrentGalaxyId(), GameData.getCurrentMissionId() ) );
        }
        protected function handleResumeClick( be:ButtonEvent ):void {
            disableButtons();
            gotoAndPlay( LABEL_ANIM_CLOSE );
            
			SoundSystem.playSound(SoundSystem.SOUND_FX, "PausePopupClose.mp3");
        }
        
        protected function disableButtons():void {
            Button1.Button.enabled = false;
            Button2.Button.enabled = false;
            Button3.Button.enabled = false;
            Checkbox1.Checkbox.enabled = false;
            Checkbox2.Checkbox.enabled = false;
            //Checkbox3.Checkbox.enabled = false;
        }
            
    }

}