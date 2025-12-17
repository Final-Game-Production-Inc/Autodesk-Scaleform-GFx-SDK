/**************************************************************************

Filename    :   AboutWindow.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.menu
{
    import com.scaleform.std.controls.SBButton;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.text.TextField;
    import scaleform.clik.events.ButtonEvent;
    
    import com.scaleform.std.system.SoundSystem;

    public class AboutWindow extends MovieClip
    {
        public var AboutBtnClose:SBButton;
        public var titleText:TextField;
        
        public function AboutWindow() 
        {
            super();
            
            addEventListener( Event.COMPLETE, notifyClose, false, 0, true );
            
            addEventListener( Event.ADDED_TO_STAGE, configUI, false, 0, true );
        }
		
		protected function configUI( e:Event ) {
            removeEventListener( Event.ADDED_TO_STAGE, configUI, false );
            
            AboutBtnClose.addEventListener(ButtonEvent.CLICK, handleClose, false, 0, true);
        }
        
        public function open():void {
            gotoAndPlay("open");
            
            SoundSystem.playSound(SoundSystem.SOUND_FX, "PausePopupOpen.mp3");
        }
        
        public function notifyClose(e:Event):void {
            this.parent.removeChild(this);
        }
        
        protected function handleClose(e:Event):void {
            gotoAndPlay("close");
            
            SoundSystem.playSound(SoundSystem.SOUND_FX, "PausePopupClose.mp3");
        }
    }

}