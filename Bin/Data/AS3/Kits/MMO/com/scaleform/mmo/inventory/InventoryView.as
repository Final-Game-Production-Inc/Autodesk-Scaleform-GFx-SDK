/**************************************************************************

Filename    :   InventoryView.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.inventory {
    
    import flash.display.Sprite;
    import flash.events.Event;
    import flash.geom.Point;
    import flash.text.TextField;
    
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.data.DataBinding;
    
    import com.scaleform.mmo.GameDataModel;
    
    public class InventoryView extends UIComponent {
    
    // Public Properties:
        
    // Protected Properties:
        
    // UI Elements:
        public var b0:InventoryBagSlot;
        public var b1:InventoryBagSlot;
        public var b2:InventoryBagSlot;
        public var b3:InventoryBagSlot;
        public var b4:InventoryBagSlot;
        
        public var gold:TextField;
        public var silver:TextField;
        public var copper:TextField;
        
        /** The Sprite to which all of the InventoryBags are attached. */
        protected var _bagCanvas:Sprite;        
        protected var _bagSlots:Array;
         /** Holds references to the currently open/displayed InventoryBags. */
        protected var _openBags:Array;
        protected var _lastIndex:uint;
                
        public function InventoryView() {
            super();
            
            _bagSlots = [];
            _openBags = [];
        }
        
        override protected function configUI():void {
            super.configUI();
                      
            stage.addEventListener( InventoryEvent.OPEN_BAG, handleOpenBagEvent, false, 0, true );
            stage.addEventListener( InventoryEvent.CLOSE_BAG, handleCloseBagEvent, false, 0, true );            
            
            if (_bagCanvas == null) { 
                _bagCanvas = new Sprite();
                addChild(_bagCanvas);
            }
            tabEnabled = tabChildren = false;
            
            setupBagSlot(b0, 0);
            setupBagSlot(b1, 1);
            setupBagSlot(b2, 2);
            setupBagSlot(b3, 3);
            setupBagSlot(b4, 4);
            
            DataBinding.createBinding("player.copper", setCopperAmount);
            DataBinding.createBinding("player.silver", setSilverAmount);
            DataBinding.createBinding("player.gold", setGoldAmount);
            addEventListener(Event.REMOVED_FROM_STAGE, clearBindings);
        }
        
        protected function setCopperAmount(value:Number):void { copper.text = value.toString(); }
        protected function setSilverAmount(value:Number):void { silver.text = value.toString(); }
        protected function setGoldAmount(value:Number):void { gold.text = value.toString(); }
        
        protected function clearBindings(e:Event):void {
            DataBinding.deleteBinding("player.copper", setCopperAmount);
            DataBinding.deleteBinding("player.silver", setSilverAmount);
            DataBinding.deleteBinding("player.gold", setGoldAmount);
        }
        
        protected function setupBagSlot(slot:InventoryBagSlot, idx:uint):void {
            slot.addEventListener(Event.SELECT, handleBagButtonToggled, false, 0, false);
            slot.index = idx;
			slot.binding = "bags.slot" + idx;
            _bagSlots.push(slot);
        }
        
        // Protected Methods:                       
        protected function handleBagButtonToggled(e:Event):void {
            var slot:InventoryBagSlot = e.target as InventoryBagSlot;
            var idx:int = slot.index;
            if (slot.selected) { openBag(idx); }
            else { closeBag(idx); }
        }
        
        protected function openBag(index:int):void {
            var data:Object = _bagSlots[index].data;
            if (!data) return;
            
            _lastIndex = index;
			createBag(data.id, data.name);
        }
        
        protected function createBag(id:String, name:String):void {
            var bag:InventoryBag = new InventoryBag();
			bag.bagName = name;
            bag.bagId = id;
            bag.index = _lastIndex;
            bag.invalidate();

            _openBags.push(bag);            
            reflowBagPositions();
            _bagCanvas.addChild(bag);
        }
        
        protected function closeBag(index:int):void {
            var data:Object = _bagSlots[index].data;
            if (!data) return;
            
            var found:Boolean = false;
            for (var i:int = 0; i < _openBags.length; i++) {
                if (_openBags[i].index == index) { 
                    found = true;
                    break;
                }
            }
            
            var bag:InventoryBag = _openBags[i];
            _openBags.splice(i, 1);
            _bagCanvas.removeChild(bag);
            reflowBagPositions();
        }
        
        // NFM: Ultimately, we want to invert this so new bags are stacked on top rather than below.
        protected function reflowBagPositions():void {
            var xshift:int = -16; // -60;
            for (var i:uint = 0; i < _openBags.length; i++) {
                if (i > 0) {
                    _openBags[i].y = _openBags[i - 1].y - _openBags[i].getHeight() - 46;// 80; /*(_openBags[i - 1].height*/;
                    var local:Point = new Point(_openBags[i].x, _openBags[i].y);
                    var global:Point = _bagCanvas.localToGlobal(local);
                    if (global.y < 0) {
                        _openBags[i].y = -_openBags[i].getHeight() - 8;
                        xshift -= 200;
                    }
                    _openBags[i].x = xshift;
                }
                else {
                    _openBags[i].y = -_openBags[i].getHeight() - 8;
                    _openBags[i].x = xshift; // NFM: TEMP TO MOVE AWAY FROM EDGE.
                }
            }
        }
               
        protected function handleOpenBagEvent(e:InventoryEvent):void {
            openBag(e.bagIdx);
        }
        
        protected function handleCloseBagEvent(e:InventoryEvent):void {
            closeBag(e.bagIdx);
            // If bag was closed via something else than slot toggle, then
            // explicitly set the slot's selected state.
            _bagSlots[e.bagIdx].selected = false;
        }
        
        override public function toString():String { 
            return "[MMOKit Inventory " + name + "]";
        }        
    }
}
