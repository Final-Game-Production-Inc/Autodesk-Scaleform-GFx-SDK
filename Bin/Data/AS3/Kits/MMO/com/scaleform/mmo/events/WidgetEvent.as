/**************************************************************************

Filename    :   WidgetEvent.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.events {

    import flash.display.DisplayObject;
    import flash.events.Event;
    
    public class WidgetEvent extends Event {
        
    // Constants:
        public static const OPEN_WIDGET:String = "openWidget";
        public static const CLOSE_WIDGET:String = "closeWidget";
        
    // Public Properties:
        public var widgetID:int;
    
    // Protected Properties:
        
    // Initialization:
        public function WidgetEvent(type:String, 
                                    bubbles:Boolean = true, 
                                    cancelable:Boolean = true,
                                    id:int = -1) {
            super(type, bubbles, cancelable);
            widgetID = id;
        }
    }
}