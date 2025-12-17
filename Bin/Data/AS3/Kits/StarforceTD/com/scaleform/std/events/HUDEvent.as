/**************************************************************************

Filename    :   HUDEvent.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.events 
{
    
    import flash.events.Event;
    
    public class HUDEvent extends Event {
        
    // Constants:
        public static const OPEN_DATABASE:String = "hudOpenDatabase";
        public static const OPEN_EXIT_MENU:String = "hudOpenExitMenu";
        public static const CLOSE_DATABASE:String = "hudCloseDatabase";
        public static const CLOSE_EXIT_MENU:String = "hudCloseExitMenu";
        
    // Public Properties:
        
    // Protected Properties:
        
    // Initialization:
        public function HUDEvent( type:String, bubbles:Boolean = true, cancelable:Boolean = true ) {
            super( type, bubbles, cancelable );
        }
        
    // Public getter / setters:
        
    // Public Methods:
        override public function clone():Event {
            return new HUDEvent( type, bubbles, cancelable );
        }
        
        override public function toString():String {
            return formatToString( "HUDEvent", "type", "bubbles", "cancelable" );
        }
        
    // Protected Methods:
        
    }
    
}