/**************************************************************************

Filename    :   InventoryBag.as
Content    :   
                

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.inventory {

    import flash.display.DisplayObject;
    import flash.display.MovieClip;
    import flash.display.SimpleButton;
    import flash.display.Sprite;
    import flash.display.Stage;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.text.TextField;
    import flash.text.TextFieldAutoSize;
    
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.controls.Button;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.events.ButtonEvent;
    import scaleform.clik.interfaces.IListItemRenderer;
    import scaleform.clik.data.DataBinding;
    
    import com.scaleform.mmo.events.TooltipEvent;
    import com.scaleform.mmo.core.DragTargetList;
    import com.scaleform.mmo.inventory.InventorySlot;
	import com.scaleform.mmo.GameDataModel;
    
    public class InventoryBag extends DragTargetList {
        
        protected var _index:int;
        protected var _bagName:String;        
        protected var _bagId:String;
        
    // UI Elements:
        public var window:MovieClip;
        public var titleBar:MovieClip;
        
        public var titleTF:TextField;
        public var closeBtn:Button;
        public var closeButton:Button;
        public var textField:TextField;
    
    // Initialization:
        public function InventoryBag() {
            super();
            
            itemRendererName = "com.scaleform.mmo.inventory.InventorySlot";
            
            titleTF = textField;
            titleTF.autoSize = TextFieldAutoSize.LEFT;
            closeBtn = closeButton;
            
            visible = false;
        }
     
    // Public Getter / Setters:
        public function get index():int { return _index; }
        public function set index(value:int):void { _index = value; }
        
		public function get bagName():String { return _bagName; }
		public function set bagName(value:String):void {
			_bagName = value;
			titleTF.text = _bagName;
		}
		
        public function get bagId():String { return _bagId; }
        public function set bagId(value:String):void {
            _bagId = value;
            setBagData();
        }
        
        protected function setBagData():void {
            dataProvider = new DataProvider(GameDataModel.getBagSlotBindings(_bagId));
            invalidate();
        }
        
    // Public Methods:
        public function getHeight():Number {
            return _rendererHeight * (_dataProvider.length / 4);
        }
        
        override protected function configUI():void {
            super.configUI();
            if (closeBtn) {
                closeBtn.addEventListener(ButtonEvent.CLICK, handleCloseButtonClick, false, 0, true);
            }
        }
        
        override protected function draw():void {
            visible = true;
            super.draw();
            if (isInvalid(InvalidationType.RENDERERS, InvalidationType.SIZE)) {
                window.height = container.height + 46 + 8; 
            }
        }
        
        protected function handleCloseButtonClick(e:ButtonEvent):void {
            dispatchEvent( new InventoryEvent( InventoryEvent.CLOSE_BAG, true, false, _index ) );
        }
        
        override public function toString():String { 
            return "[MMOKit InventoryBag " + name + "]";
        }
    }
}