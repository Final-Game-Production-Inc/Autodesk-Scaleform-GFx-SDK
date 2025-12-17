/**************************************************************************

Filename    :   MainMenuEvent.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.events 
{  
    import flash.events.Event;
    
    public class MainMenuEvent extends Event {
        
		public static const LOGO_TO_INTRO_TRANSITION:String = "logoToIntroTransition";
		public static const INTRO_TO_MAP_TRANSITION:String = "introToMapTransition";
		public static const INTRO_TO_MAP_COMPLETE:String = "introToMapComplete";
        public static const MAP_TO_INTRO_TRANSITION:String = "mapToIntroTransition";
        public static const START_GAME:String = "systemStartGame";
		
		public function MainMenuEvent(type:String, bubbles:Boolean = true, cancelable:Boolean = true) {
			super(type, bubbles, cancelable);
		}
    }
    
}