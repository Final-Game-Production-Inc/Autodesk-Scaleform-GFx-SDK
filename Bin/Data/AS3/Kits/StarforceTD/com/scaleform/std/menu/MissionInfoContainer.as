/**************************************************************************

Filename    :   MissionInfoContainer.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.menu
{
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    
    import flash.ui.Multitouch;
    import flash.ui.MultitouchInputMode;
    import flash.events.TouchEvent;
    
    import com.scaleform.std.system.SoundSystem;

    public class MissionInfoContainer extends MovieClip 
    {
        // Animation labels
        public static const LABEL_ANIMATION_OFF:String = "off";
        public static const LABEL_ANIMATION_OPEN:String = "open";
        public static const LABEL_ANIMATION_CLOSE:String = "close";
        
        public var MissionInfo:MissionInfoView;
        
        public function MissionInfoContainer() {
            super();
        }
        
        public function open():void {
            MissionInfo.enableButtons();
            gotoAndPlay( LABEL_ANIMATION_OPEN );
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "PlanetPopupOpen.mp3");
        }
        public function close():void {
            MissionInfo.disableButtons();
            gotoAndPlay( LABEL_ANIMATION_CLOSE );
			
            
        }
        public function hide():void {
            gotoAndPlay( LABEL_ANIMATION_OFF );
        }
    }

}