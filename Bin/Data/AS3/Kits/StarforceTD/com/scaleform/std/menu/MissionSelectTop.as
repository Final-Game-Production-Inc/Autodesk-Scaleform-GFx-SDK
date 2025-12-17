/**************************************************************************

Filename    :   MissionSelectTop.as

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
    import flash.events.TouchEvent;
    import flash.text.TextField;
    import scaleform.gfx.InteractiveObjectEx;
    
    import flash.system.System;
	
    import com.scaleform.std.controls.SBButton;
	import scaleform.clik.events.ButtonEvent;

    public class MissionSelectTop extends MovieClip 
    {        
        
        // UI Scene
        public var TitleText:TextField; // The textfield at the top-right that displays the name of the selected galaxy.
        public var StarPointText:TextField; // The x/y ratio that shows the number of stars the player has earned for this level.
        
		public var BtnSettings:SBButton;
        public var BtnTips:SBButton;
		
		public var Glow:MovieClip;
        
        public function MissionSelectTop() {
            super();
            addEventListener( Event.ENTER_FRAME, configUI, false, 0, true );
        }
        
        protected function configUI( e:Event ) {
            removeEventListener( Event.ENTER_FRAME, configUI, false );
            
			InteractiveObjectEx.setHitTestDisable(Glow, true);
        }
        
    }

}