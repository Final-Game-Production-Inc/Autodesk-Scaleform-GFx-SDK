/**************************************************************************

Filename    :   TooltipEvent.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.events {
    
    import flash.display.DisplayObject;
    import flash.events.Event;
    
    import scaleform.clik.core.UIComponent;
    
    public class TooltipEvent extends Event {
        
    // Constants:
        public static const REGISTER_ELEMENT:String = "tooltipRegister";
        public static const UNREGISTER_ELEMENT:String = "tooltipUnregister";
        
    // Public Properties:
        public var elem:UIComponent;
    
    // Protected Properties:
        
    // Initialization:
        public function TooltipEvent(type:String, 
                                    bubbles:Boolean = true, 
                                    cancelable:Boolean = false,
                                    element:UIComponent = null) {
            super(type, bubbles, cancelable);
            elem = element;
        }
    }
}