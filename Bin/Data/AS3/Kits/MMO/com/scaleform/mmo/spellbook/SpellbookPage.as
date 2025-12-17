/**************************************************************************

Filename    :   SpellbookPage.as
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

    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.controls.DragSlot;
    import scaleform.clik.controls.Button;
    import scaleform.clik.data.DataProvider;
    import scaleform.clik.events.ButtonEvent;
    
    import com.scaleform.mmo.GameDataModel;
    import com.scaleform.mmo.utils.TooltipManager;
    
    public class SpellbookPage extends UIComponent {
        
    // Constants:
    
    // Public Properties:
    
    // Protected Properties:
        protected var _delayDataInit:Boolean = false;
		protected var _spells:Array;
		protected var _startIdx:Number;
		protected var _len:Number;
        protected var _entries:Array;
    
    // UI Elements:
        
    // Initialization:
        public function SpellbookPage() {
            super();
            _entries = [];
        }
        
    // Public Getter / Setters:

    // Public Methods:
        public function setData(spells:Array, startIdx:uint, len:uint):void {
			_spells = spells;
			_startIdx = startIdx;
			_len = len;
			if (!initialized) {
				_delayDataInit = true;
				return;
			}
			var l:uint = Math.min(startIdx + len, _entries.length);
            for (var i:uint = startIdx; i < l; i++) {     
				var spell:Object = spells[i];
                _entries[i].setData( spell );
            }
        }
        
        // Protected Methods:
        override protected function configUI():void {
            var entryCounter:uint = 0;
            var entry:SpellbookEntry = getChildByName( "entry" + entryCounter++ ) as SpellbookEntry; 
            while (entry != null) {
                _entries.push( entry );
                entry = getChildByName( "entry" + entryCounter++ ) as SpellbookEntry; 
            }            
        }
        
        override protected function draw():void {
            super.draw();
			
			if (_delayDataInit) {
				
				var l:uint = Math.min(_startIdx + _len, _entries.length);
				for (var i:uint = _startIdx; i < l; i++) {     
					var spell:Object = _spells[i];
					_entries[i].setData( spell );
				}
				
				_delayDataInit = false;
			}
        }
        
        override public function toString():String { 
            return "[MMOKit SpellbookPage " + name + "]";
        }
    }
}
