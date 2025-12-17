/**************************************************************************

Filename    :   WindowEvent.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.events {

    import flash.display.DisplayObject;
    import flash.events.Event;
    
    public class WindowEvent extends Event {
        
    // Constants:
        public static const WINDOW_READY:String = "windowReady";
        
    // Public Properties:
        public var widgetID:int;
        public var windowRef:DisplayObject;
    
    // Protected Properties:
        
    // Initialization:
        public function WindowEvent(type:String, 
                                    bubbles:Boolean = true, 
                                    cancelable:Boolean = true,
                                    id:int = -1,
                                    ref:DisplayObject = null) {
            super(type, bubbles, cancelable);
            widgetID = id;
            windowRef = ref;
        }
    }
}