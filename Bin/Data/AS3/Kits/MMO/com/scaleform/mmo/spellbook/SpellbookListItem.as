/**************************************************************************

Filename    :   SpellbookListItem.as
Content    :   
                

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.spellbook {
       
	import scaleform.clik.controls.ListItemRenderer;
	
    public class SpellbookListItem extends ListItemRenderer {
        
    // Initialization:
        public function SpellbookListItem() {
            super();
        }
        
    // Public Methods:
        override public function setData(data:Object):void {
            visible = (data != null);
        }	
        
        override public function toString():String { 
            return "[MMOKit SpellbookListItem " + name + "]";
        }
    }
}