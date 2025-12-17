/**************************************************************************

Filename    :   FxEvent.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.events 
{
    
    import flash.events.Event;
    
    public class FxEvent extends Event {
        
    // Constants:
        public static const CREATED:String  = "fxCreated";
        public static const COMPLETE:String = "fxComplete";
        public static const LAYER_FX:uint = 0;
        public static const LAYER_BOTTOM:uint = 1;
        
    // Public Properties:
        /** An override in case this Fx is on a layer other than the Fx layer. */
        public var layer:uint = 0;
        
    // Protected Properties:
        
    // Initialization:
        public function FxEvent( type:String, bubbles:Boolean = true, cancelable:Boolean = true,
                                 layer:uint = 0 ) {
            super( type, bubbles, cancelable );
            this.layer = layer;
        }
        
    // Public Methods:
        override public function clone():Event {
            return new FxEvent( type, bubbles, cancelable );
        }
        
        override public function toString():String {
            return formatToString( "FxEvent", "type", "bubbles", "cancelable" );
        }
        
    // Protected Methods:
        
    }
    
}