/**************************************************************************

Filename    :   WaveButtonBag.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.controls {
    
    import flash.display.DisplayObject;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.events.TouchEvent;
    import flash.text.TextField;
    
    import scaleform.gfx.FocusEventEx;
    import scaleform.gfx.MouseEventEx;
    
    import com.scaleform.std.utils.Constants;
    
    public class WaveButtonBag extends MovieClip {
        
    // Constants:
        
    // Public Properties:
        
    // Protected Properties:
        
    // UI Elements:
        public var WaveButton:SBButton;

    // Initialization:
        public function WaveButtonBag() {
            super();
            
        }
        
    // Public Getter / Setters:

    // Public Functions
        /** @exclude */
        override public function toString():String { 
            return "[SB WaveButtonBag" + name + "]";
        }
        
    // Protected Methods:
       
    }
}