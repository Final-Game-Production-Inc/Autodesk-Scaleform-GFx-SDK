/**************************************************************************

Filename    :   ActionSlot.as
Content    :   
                


Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.actionbar {
    
    import flash.display.MovieClip;
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
    
    import com.scaleform.mmo.core.MDragTarget;
    
    public class ActionSlot extends MDragTarget implements IListItemRenderer {
        
    // Constants:
    
    // Public Properties:
        
    // Protected Properties:
        protected var _owner:UIComponent;
        protected var _selectable:Boolean = true;
        protected var _selected:Boolean = false;
        
    // UI Elements:
    
    // Initialization:
        public function ActionSlot() {
            super();

            // PPS: Move to base?
            slotUse.mouseEnabled = false;
            slotUse.mouseChildren = false;
        }
        
    // Public Getter / Setters:
        public function get owner():UIComponent { return _owner; }
        public function set owner(value:UIComponent):void { _owner = value; }
        
        public function get selectable():Boolean { return _selectable; }
        public function set selectable(value:Boolean):void { _selectable = value; }
        
        public function get selected():Boolean { return _selected; }
        public function set selected(value:Boolean):void { _selected = value; }
        
    // Public Methods:
        public function setListData(listData:ListData):void {
            index = listData.index;
            gotoAndPlay("up"); // Refresh the component, including the timeline state.
        }
        public function setData(data:Object):void {
            // We are getting a binding ID (String) here
            binding = String(data);
        }
        
    // Public Methods:
        override protected function handleMouseUp(e:MouseEvent):void { 
            // Clean up the original mouse listeners since the user has "clicked".
            cleanupDragListeners()
            
            // Dispatch a generic CLIK ButtonEvent.CLICK to be caught by whomever may be listening.
            // dispatchEvent(new ButtonEvent(ButtonEvent.CLICK));
        }
        
    // Protected Methods:       
        override public function toString():String { 
            return "[MMOKit ActionSlot " + name + "]";
        }
    }
}