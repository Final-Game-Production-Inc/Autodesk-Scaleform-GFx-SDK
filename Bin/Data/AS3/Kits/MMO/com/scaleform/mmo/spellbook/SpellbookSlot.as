/**************************************************************************

Filename    :   SpellbookSlot.as
Content    :   
                

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.spellbook {
    
    import flash.display.Sprite;
    import flash.events.MouseEvent;
    import flash.utils.getDefinitionByName;
    
    import scaleform.clik.interfaces.IListItemRenderer;
    import scaleform.clik.controls.DragSlot;
    import scaleform.clik.managers.DragManager;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.data.ListData;
    import scaleform.clik.events.DragEvent;
    import scaleform.clik.events.ButtonEvent;
    
    import com.scaleform.mmo.GameDataModel;
    import com.scaleform.mmo.core.MDragTarget;
    
    public class SpellbookSlot extends MDragTarget {
        
    // Constants:
    
    // Public Properties:
        
    // Protected Properties:
        protected var _owner:UIComponent;
        protected var _selectable:Boolean = true;
        protected var _selected:Boolean = false;
        
    // UI Elements:
        
    // Initialization:
        public function SpellbookSlot() {
            super();
        }
        
    // Public Getter / Setters:        
        
    // Public Methods:                
        override protected function handleMouseUp(e:MouseEvent):void { 
            // Clean up the original mouse listeners since the user has "clicked".
            cleanupDragListeners();
			
			_content.x = 0;
            _content.y = 0;
			
			GameDataModel.notifySlotUse(binding);
        }
        
    // Protected Methods:
        override protected function configUI():void {
            super.configUI();
        }
        
        override protected function draw():void {
            super.draw();
        }
        
        override public function toString():String { 
            return "[MMOKit SpellBookSlot " + name + "]";
        }
    }
}