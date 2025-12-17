/**************************************************************************

Filename    :   PaperDoll.as
Content    :   
                

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.paperdoll {
    
    import com.scaleform.mmo.events.TooltipEvent;
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.events.Event;
    import flash.events.MouseEvent;

    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.controls.DragSlot;
    import scaleform.clik.controls.Button;
    import scaleform.clik.controls.ScrollingList;
    import scaleform.clik.controls.ScrollBar;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.events.ButtonEvent;
    
    import com.scaleform.mmo.events.TooltipEvent;
	import scaleform.clik.data.DataBinding;
    
    public class PaperDoll extends UIComponent {
        
    // Constants:
        public static const NUM_PAPER_DOLL_SLOTS:uint = 16;
        
    // Public Properties:
    
    // Protected Properties:
        protected var _slots:Array;
		protected var _playerName:String, _playerLevel:String, _playerRace:String, _playerClass:String;
        
    // UI Elements:
        public var list1:ScrollingList;
        public var list2:ScrollingList;
        public var statItem1:MovieClip;
        public var statItem2:MovieClip;
        public var statItem3:MovieClip;
        
        public var paperDollHeader:MovieClip;
        public var scrolly:ScrollBar;
        
    // Initialization:
        public function PaperDoll() {
            super();
        }
        
        override protected function initialize():void {
            super.initialize();
            
            _slots = [];
            tabEnabled = tabChildren = false;
        }
        
    // Public Getter / Setters:

    // Public Methods:
        override protected function configUI():void {
            super.configUI();

            for (var i:uint = 0; i < NUM_PAPER_DOLL_SLOTS; i++) {
                var slot:PaperDollSlot = getChildByName( "slot" + i ) as PaperDollSlot;
				slot.index = i;
				slot.binding = "equipment.slot" + i;
                _slots.push( slot );
                
                dispatchEvent( new TooltipEvent( TooltipEvent.REGISTER_ELEMENT, true, false, slot ) );
            }
            
			DataBinding.createBinding("player.name", setPlayerName);
			DataBinding.createBinding("player.level", setPlayerLevel);
			DataBinding.createBinding("player.race", setPlayerRace);
			DataBinding.createBinding("player.class", setPlayerClass);
			addEventListener(Event.REMOVED_FROM_STAGE, clearBindings);
            
            if (list1) { 
                list1.dataProvider = new DataProvider( [    { label:"Character",    isDisabled:false },
                                                            { label:"Currencies",   isDisabled:true },
                                                            { label:"Reputation",   isDisabled:true },
                                                            { label:"Titles",       isDisabled:true } ] );
                list1.selectedIndex = 0;
            }
        }
        
		protected function setPlayerName(value:String):void { _playerName = value; updateTitle(); }
		protected function setPlayerLevel(value:Number):void { _playerLevel = value.toString(); updateTitle(); }
		protected function setPlayerRace(value:String):void { _playerRace = value; updateTitle(); }
		protected function setPlayerClass(value:String):void { _playerClass = value; updateTitle(); }
			
		protected function clearBindings(e:Event):void {
			DataBinding.deleteBinding("player.name", setPlayerName);
			DataBinding.deleteBinding("player.level", setPlayerLevel);
			DataBinding.deleteBinding("player.race", setPlayerRace);
			DataBinding.deleteBinding("player.class", setPlayerClass);
		}
		
        // Protected Methods:
        override protected function draw():void {
            super.draw();
        }
        
		protected function updateTitle():void {
			if (paperDollHeader.textField) {
                    paperDollHeader.textField.text = _playerName + " | Level " + _playerLevel + " " + _playerRace + " " + _playerClass;
                }
		}
		
        override public function toString():String { 
            return "[MMOKit PaperDoll " + name + "]";
        }
    }
}