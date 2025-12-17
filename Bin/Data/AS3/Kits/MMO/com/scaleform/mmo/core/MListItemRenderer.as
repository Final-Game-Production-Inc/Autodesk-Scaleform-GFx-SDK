/**************************************************************************

Filename    :   MListItemRenderer.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.core {
    
    import flash.events.FocusEvent;
    
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.data.ListData;
    import scaleform.clik.interfaces.IListItemRenderer;
    import scaleform.clik.controls.ListItemRenderer;
    
    public class MListItemRenderer extends ListItemRenderer implements IListItemRenderer {
        
    // Constants:
        
    // Public Properties:
        
    // Protected Properties:
        
    // UI Elements:
    
    // Initialization:
        public function MListItemRenderer() {
            super();
        }
        
    // Public Methods:
        override public function setData(data:Object):void {
            super.setData(data);
            if (data == null) { return; }
            if (data.isDisabled) {
                enabled = false;
            }
        }
        
        override public function toString():String {
            return "[MMOKit ListItemRenderer " + index + ", " + name + "]";
        }
        
    // Protected Methods:
        
    }
    
}