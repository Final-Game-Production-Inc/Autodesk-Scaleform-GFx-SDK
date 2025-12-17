/**************************************************************************

Filename    :   HUDTop.as

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
    import flash.events.TouchEvent;
    import flash.text.TextField;
    
    import com.scaleform.std.controls.SBButton;

    public class HUDTop extends MovieClip 
    {        
        
        // UI Scene
        public var TitleText:TextField; // The textfield at the top-right that displays the name of the selected galaxy.
        public var StarPointText:TextField; // The x/y ratio that shows the number of stars the player has earned for this level.
        
        public var BtnTutorial:SBButton; 
        public var BtnPause:SBButton;
        public var BtnBack:SBButton; // Button on thats opens the menu that allows the user to return to the Main Menu..
        public var BtnSound:SBButton; // Button on top left that toggles sound effect playback.
        public var BtnMusic:SBButton; // Button in the center that toggles music playback.
        public var BtnTips:SBButton;
        
        public function HUDTop() {
            super();
            addEventListener( Event.ENTER_FRAME, configUI, false, 0, true );
        }
        
        protected function configUI( e:Event ) {
            removeEventListener( Event.ENTER_FRAME, configUI, false );
        }
        
    }

}