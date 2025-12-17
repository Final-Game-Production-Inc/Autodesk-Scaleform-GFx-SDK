/**************************************************************************

Filename    :   HUDTop.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.hud
{
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.events.TouchEvent;
    import flash.text.TextField;
    
    import scaleform.clik.events.ButtonEvent;
    
    import com.scaleform.std.controls.SBButton;
    import com.scaleform.std.events.HUDEvent;
    import com.scaleform.std.events.SystemEvent;
    import com.scaleform.std.system.SoundSystem;
    import com.scaleform.std.utils.Constants;
    import com.scaleform.std.events.GameEvent;

    public class HUDTop extends MovieClip 
    {        
        
        // UI Scene
        public var TitleText:TextField; // The textfield at the top-right that displays the name of the selected galaxy.
        public var StarPointText:TextField; // The x/y ratio that shows the number of stars the player has earned for this level.
        
        //public var BtnPause:SBButton;
		public var BtnSettings:SBButton;
        public var BtnTips:SBButton;
        
        public function HUDTop() {
            super();
            addEventListener( Event.ENTER_FRAME, configUI, false, 0, true );
			
			// PPS: Hide this for now
			//BtnPause.visible = false;			
        }
        
        protected function configUI( e:Event ) {
            removeEventListener( Event.ENTER_FRAME, configUI, false );
			
            //BtnPause.addEventListener( ButtonEvent.CLICK, handlePauseClick, false, 0, true );
            BtnSettings.addEventListener( ButtonEvent.CLICK, handleBackClick, false, 0, true );			
            BtnTips.addEventListener( ButtonEvent.CLICK, handleShowGameTutorial, false, 0, true );
        }
        
        /*
        protected function handlePauseClick( e:ButtonEvent ):void {
            dispatchEvent( new SystemEvent( SystemEvent.PAUSE_GAME ) );
            
            var eventType:String = Constants.isTouchEnabled() ? TouchEvent.TOUCH_BEGIN : MouseEvent.MOUSE_DOWN;
            stage.addEventListener( eventType, onStageClickResumeGame, false, 0, true );
        }
        */
                
        protected function handleBackClick( e:ButtonEvent ):void {
            dispatchEvent( new HUDEvent( HUDEvent.OPEN_EXIT_MENU ) );
            dispatchEvent( new SystemEvent( SystemEvent.PAUSE_GAME ) );
        }
        
        protected function handleShowGameTutorial(e:Event):void {            
            dispatchEvent(new GameEvent(GameEvent.SHOW_GAME_TUTORIAL, true, false, 0, 0, resumeGame));
            dispatchEvent( new SystemEvent( SystemEvent.PAUSE_GAME ) );
        }
        
        protected function resumeGame():void {
            dispatchEvent( new SystemEvent( SystemEvent.RESUME_GAME ) );
        }
        
        /*
        protected function onStageClickResumeGame( e:Event ):void {
            if ( e.target == BtnPause ) { return; } // Ignore the initial ButtonEvent.CLICK that comes in immediately following the Puase.
            var eventType:String = Constants.isTouchEnabled() ? TouchEvent.TOUCH_BEGIN : MouseEvent.MOUSE_DOWN;
            stage.removeEventListener( eventType, onStageClickResumeGame, false );
            dispatchEvent( new SystemEvent( SystemEvent.RESUME_GAME ) );
        }
        */
        
    }

}
