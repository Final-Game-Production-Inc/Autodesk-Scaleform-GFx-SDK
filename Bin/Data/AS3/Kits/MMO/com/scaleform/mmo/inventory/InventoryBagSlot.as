/**************************************************************************

Filename    :   InventoryBagSlot.as
Content    :   
                

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.inventory {
       
    import com.scaleform.mmo.core.MDragTarget;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import scaleform.clik.events.ComponentEvent;
    import scaleform.clik.data.DataBinding;
    
    public class InventoryBagSlot extends MDragTarget {
        
    // Constants:
        
    // Public Properties:
        
    // Protected Properties:
        protected var _selected:Boolean = false;
        
    // UI Elements:
        
    // Initialization:
        public function InventoryBagSlot() {
            super();
            _selected = false;
        }
        
    // Public Getter / Setters:
        public function get selected():Boolean { return _selected; }
        public function set selected(value:Boolean):void { 
            if (_selected == value) return;
            _selected = value; 
            setState((_state == "out" || _state == "up") ? "out" : "selecting");
            validateNow();
        }

    // Public Methods:
    
    // Protected Methods:      
		override protected function setItemSlotData(data:Object):void {
			// If binding changes, then make sure to close the open bag
			if (selected && (!data || this.data.id != data.id)) {
				selected = false;
				dispatchEvent(new Event(Event.SELECT));
			}			
			super.setItemSlotData(data);
		}
	
        override protected function handleMouseUp(e:MouseEvent):void { 
            // Clean up the original mouse listeners since the user has "clicked".
            cleanupDragListeners();
            
            selected = !_selected;
            dispatchEvent(new Event(Event.SELECT));
        }
    
        override protected function setState(state:String):void {
            _state = state;
            var prefixes:Vector.<String> = getStatePrefixes();
            var states:Array = _stateMap[state];
            if (states == null || states.length == 0) { return; }
            var l:uint = prefixes.length;
            for (var i:uint=0; i<l; i++) {
                var prefix:String = prefixes[i];
                var sl:uint = states.length;
                for (var j:uint=0; j<sl; j++) {
                    var thisLabel:String = prefix + states[j];
                    if (_labelHash[thisLabel]) {
                        _newFrame = thisLabel;
                        invalidateState();
                        return;
                    }
                }
            }
        }
        
        // Optimization to states look-up
        //LM: Make static.
        protected var statesDefault:Vector.<String> = Vector.<String>([""]);
        protected var statesSelected:Vector.<String> = Vector.<String>(["selected_", ""]);
        protected function getStatePrefixes():Vector.<String> {
            return _selected ? statesSelected : statesDefault;
        }        
        
        override public function toString():String { 
            return "[MMOKit InventoryBagSlot " + name + "]";
        }
    }
}