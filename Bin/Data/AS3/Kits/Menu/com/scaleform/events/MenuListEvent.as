/**************************************************************************

Filename    :   MenuListEvent.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
 
package com.scaleform.events {
    
    import scaleform.clik.events.ListEvent;
    import scaleform.clik.interfaces.IListItemRenderer;
    
    public class MenuListEvent extends ListEvent {
        
    // Constants:
        public static const ITEM_CHANGE:String = "itemChange";
        
    // Public Properties:

    // Protected Properties:
        
    // Initialization:
        public function MenuListEvent(type:String, bubbles:Boolean=false, cancelable:Boolean=true, 
                                  index:int=-1, columnIndex:int=-1, rowIndex:int=-1, 
                                  itemRenderer:IListItemRenderer=null, isKeyboard:Boolean = false) {
            super(type, bubbles, cancelable, index, columnIndex, rowIndex, itemRenderer, isKeyboard);
        }
        
    // Public getter / setters:
        
    // Public Methods:
        
    // Protected Methods:
        
    }
    
}