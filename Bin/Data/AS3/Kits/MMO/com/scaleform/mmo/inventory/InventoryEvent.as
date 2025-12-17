/**************************************************************************

Filename    :   InventoryEvent.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.inventory {

    import flash.display.DisplayObject;
    import flash.events.Event;
    
    public class InventoryEvent extends Event {
        
    // Constants:
        public static const OPEN_BAG:String = "openBag";
        public static const CLOSE_BAG:String = "closeBag";
        
    // Public Properties:
        public var bagIdx:int;
    
    // Protected Properties:
        
    // Initialization:
        public function InventoryEvent( type:String, 
                                        bubbles:Boolean = true, 
                                        cancelable:Boolean = true,
                                        idx:int = -1) {
            super(type, bubbles, cancelable);
            bagIdx = idx;
        }
    }
}