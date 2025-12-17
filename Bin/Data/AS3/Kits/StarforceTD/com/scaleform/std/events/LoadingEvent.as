/**************************************************************************

Filename    :   LoadingEvent.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.events 
{
    
    import flash.events.Event;
    import flash.geom.Point;
    
    public class LoadingEvent extends Event {
        
    // Constants:
        public static const LOAD_START:String = "loadStart";
        public static const LOAD_COMPLETE:String = "loadComplete";
        public static const LOAD_OPEN_COMPLETE:String = "loadOpenComplete";
        public static const LOAD_CLOSE_COMPLETE:String = "loadCloseComplete";
        
    // Public Properties:
        
    // Protected Properties:
        
    // Initialization:
        public function LoadingEvent(  type:String, bubbles:Boolean = true, cancelable:Boolean = true) 
        {
            super(type, bubbles, cancelable);
        }
        
    // Public Methods:
        override public function clone():Event {
            return new LoadingEvent(type, bubbles, cancelable);
        }
        
        override public function toString():String {
            return formatToString("LoadingEvent", "type", "bubbles", "cancelable");
        }
        
    // Protected Methods:
        
    }
    
}