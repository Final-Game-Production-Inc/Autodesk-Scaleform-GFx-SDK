/**************************************************************************

Filename    :   Constants.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.utils 
{
    import flash.display.Stage;
    import flash.ui.Multitouch;
    
    public class Constants
    {
        public static const DEGTORAD:Number = 0.01745329251994329577;
        
		public static var SPEED_STEP:Number = 0;		
        public static var FRAME_DELTA:Number;
		
        public static var isDebugMode:Boolean = false;
        
        public static function initialize( stage:Stage ):void {
			SPEED_STEP = stage.frameRate;
            FRAME_DELTA = 1 / stage.frameRate;
        }
        
        public static function isTouchEnabled():Boolean {
            return Multitouch.supportsTouchEvents;
        }
    }

}