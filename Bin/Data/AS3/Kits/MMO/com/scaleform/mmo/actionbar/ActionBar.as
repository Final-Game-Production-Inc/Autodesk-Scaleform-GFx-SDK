/**************************************************************************

Filename    :   ActionBar.as
Content    :   
                


Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.actionbar {
    
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.display.Stage;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.display.DisplayObject;
    
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.controls.DragSlot;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.events.ButtonEvent;
    import scaleform.clik.interfaces.IListItemRenderer;
    
    import com.scaleform.mmo.core.DragTargetList;
    import com.scaleform.mmo.GameDataModel;
    import com.scaleform.mmo.events.TooltipEvent;
    
    public class ActionBar extends DragTargetList {
        
    // Public Properties:
        
    // Protected Properties:
        protected var _index:int;
        
    // UI Elements:
        
    // Initialization:
        public function ActionBar() {
            super();
            
            itemRendererName = "ActionBarActionSlot";
        }
     
    // Public Getter / Setters:
        public function get index():int { return _index; }
        public function set index(value:int):void { _index = value; }
        
    // Protected Methods:
        public function getWidth():Number {
            // NFM: This should be cleaned up (divided into subclassses, HorizontalActionBar, VerticalActionBar).
            if (_totalColumns > _totalRows) {
                return _rendererWidth * (_dataProvider.length);
            }
            else {
                return _rendererWidth;
            }
        }
        
        public function getHeight():Number {
            // NFM: This should be cleaned up (divided into subclassses, HorizontalActionBar, VerticalActionBar).
            if (_totalRows > _totalColumns) {
                return _rendererHeight * (_dataProvider.length);
            }
            else {
                return _rendererHeight;
            }
        }
        
        override protected function setupRenderer(mc:DisplayObject, index:int = -1):void {
            var slot:ActionSlot = mc as ActionSlot;
            //slot.binding = ( this.name, index );
            slot.addEventListener(MouseEvent.CLICK, handleActionSlotClick, false, 0, true);
            stageRef.dispatchEvent( new TooltipEvent( TooltipEvent.REGISTER_ELEMENT, true, false, slot ) );
        }
        
        protected function handleActionSlotClick(e:MouseEvent):void {
            var slot:ActionSlot = e.target as ActionSlot;
            GameDataModel.notifySlotUse( slot.binding );
        }
        
    // Public Methods:
        override public function toString():String { 
            return "[MMOKit ActionBar " + name + "]";
        }
    }
}