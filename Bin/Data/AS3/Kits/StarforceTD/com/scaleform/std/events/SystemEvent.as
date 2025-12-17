/**************************************************************************

Filename    :   SystemEvent.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.events 
{
    
    import flash.events.Event;
    
    public class SystemEvent extends Event {
        
    // Constants:
        public static const PAUSE_GAME:String = "systemPauseGame";
        public static const RESUME_GAME:String = "systemResumeGame";
        
    // Public Properties:
        
    // Protected Properties:
        
    // Initialization:
        public function SystemEvent( type:String, bubbles:Boolean = true, cancelable:Boolean = true ) {
            super( type, bubbles, cancelable );
        }
        
    // Public getter / setters:
        
    // Public Methods:
        override public function clone():Event {
            return new SystemEvent( type, bubbles, cancelable );
        }
        
        override public function toString():String {
            return formatToString( "SystemEvent", "type", "bubbles", "cancelable" );
        }
        
    // Protected Methods:
        
    }
    
}