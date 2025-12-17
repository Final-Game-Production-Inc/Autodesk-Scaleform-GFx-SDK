/**************************************************************************

Filename    :   PrimaryActionBar.as
Content    :   
                


Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.actionbar {
    
    import flash.events.MouseEvent;   
    import scaleform.clik.core.UIComponent;    
    import com.scaleform.mmo.GameDataModel;
	import scaleform.clik.controls.Button;
    
    public class PrimaryActionBar extends UIComponent {
        
    // Public Properties:
        
    // Protected Properties:
        protected var _index:int;
        protected var _slots:Array;
        
    // UI Elements:
        public var LockButton:Button;
        public var actionBarButtonUp:Button;
        public var actionBarButtonDown:Button;
        
    // Initialization:
        public function PrimaryActionBar() {
            super();
        }
     
    // Public Getter / Setters:
    
    // Protected Methods:
        override protected function configUI():void {
            super.configUI();
            
            var i:uint = 0;
            var slot:ActionSlot = getChildByName( "slot" + i++ ) as ActionSlot;
            while (slot != null) {
                slot.binding = "actionbar0.slot" + (i-1);
                slot.addEventListener(MouseEvent.CLICK, handleActionSlotClick, false, 0, true);                
                slot = getChildByName( "slot" + i++ ) as ActionSlot;
            }
        }
        
        protected function handleActionSlotClick(e:MouseEvent):void {
            var slot:ActionSlot = e.target as ActionSlot;
            GameDataModel.notifySlotUse( slot.binding );
        }
        
    // Public Methods:
        override public function toString():String { 
            return "[MMOKit PrimaryActionBar " + name + "]";
        }
    }
}