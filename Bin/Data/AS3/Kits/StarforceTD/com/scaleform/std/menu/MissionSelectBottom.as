/**************************************************************************

Filename    :   MissionSelectBottom.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.menu
{
    import com.scaleform.std.controls.MainMenuTab;
    import com.scaleform.std.events.GameEvent;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.events.TouchEvent;
    import flash.text.TextField;
    import scaleform.gfx.InteractiveObjectEx;
    
    import com.scaleform.std.controls.SBButton;
    import scaleform.clik.events.ButtonEvent;

    public class MissionSelectBottom extends MovieClip 
    {        
        
        // UI Scene
        public var BtnResearch:MainMenuTab; // Toggles the Research view.
        public var BtnDatabase:MainMenuTab;  // Toggles the Database view.
        public var BtnMedals:MainMenuTab;  // Toggles the Medals view.
		
		public var Glow:MovieClip;
        
        public function MissionSelectBottom() {
            super();
			
			addEventListener( Event.ADDED_TO_STAGE, configUI, false, 0, true );
        }
		
		protected function configUI( e:Event ) {
            removeEventListener( Event.ADDED_TO_STAGE, configUI, false );
            
			BtnResearch.visible = false;
			BtnDatabase.visible = false;
			BtnMedals.visible = false;
            
			InteractiveObjectEx.setHitTestDisable(Glow, true);            
        }
        
        /*
        BtnAbout.addEventListener(ButtonEvent.CLICK, handleAboutClick, false, 0, true);        
        protected function handleAboutClick(e:Event):void {
            dispatchEvent(new GameEvent(GameEvent.SHOW_ABOUT_DIALOG));
        }
        */
        
    }

}