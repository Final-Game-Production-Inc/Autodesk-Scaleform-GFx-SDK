/**************************************************************************

Filename    :   SpellbookEntry.as
Content    :   
                

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.mmo.spellbook {
    
    import flash.display.Sprite;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.text.TextField;

    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.controls.DragSlot;
    import scaleform.clik.controls.Button;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.events.ButtonEvent;
    
    import com.scaleform.mmo.GameDataModel;
    import com.scaleform.mmo.events.TooltipEvent;
    
    public class SpellbookEntry extends UIComponent {
        
    // Constants:
    
    // Public Properties:
    
    // Protected Properties:
        protected var _data:Object;
        
    // UI Elements:
        public var slot:SpellbookSlot;
        public var abilityRank:TextField;
        public var abilityName:TextField;
        
    // Initialization:
        public function SpellbookEntry() {
            super();
            configUI();
        }
        
    // Public Getter / Setters:

    // Public Methods:
        public function setData( data:Object ):void {
            _data = data;
            if (_data) {
                slot.binding = data.binding;
            }
            
            //dispatchEvent( new TooltipEvent( TooltipEvent.REGISTER_ELEMENT, true, false, slot ) );
            
            invalidateData();
            validateNow();
        }
        
        override public function get enabled():Boolean { return super.enabled; }
        override public function set enabled(value:Boolean):void {
            super.enabled = value;
            gotoAndPlay( (enabled) ? "on" : "off" );			
            slot.enabled = value;
			slot.validateNow();
        }
        
        override protected function configUI():void {
            //
        }
        
        // Protected Methods:
        override protected function draw():void {
            super.draw();
            
            if (isInvalid(InvalidationType.DATA)) {
                if (_data) {
                    enabled = !_data.disabled;
                    abilityName.text = _data.name;
                    abilityRank.text = _data.message;
                }
                visible = (_data != null);
            }
        }
        
        override public function toString():String { 
            return "[MMOKit SpellbookEntry " + name + "]";
        }
    }
}