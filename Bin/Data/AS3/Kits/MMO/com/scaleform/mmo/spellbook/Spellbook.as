/**************************************************************************

Filename    :   Spellbook.as
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
    import scaleform.clik.controls.ScrollingList;
    import scaleform.clik.events.ListEvent;
    
    import com.scaleform.mmo.GameDataModel;
    import com.scaleform.mmo.utils.TooltipManager;
    
    public class Spellbook extends UIComponent {
        
    // Constants:
    
    // Public Properties:
    
    // Protected Properties:
        
    // UI Elements:
        public var groupView:SpellbookGroupView;
        public var tabData:Array;
        public var list:ScrollingList;
        
    // Initialization:
        public function Spellbook() {
            super();
        }
        
        override protected function initialize():void {
            super.initialize();
        }
        
    // Public Getter / Setters:

    // Public Methods:
        override protected function configUI():void {
            super.configUI();
            
            tabData = [];
            if ( GameDataModel.getSpellbookGroups != null ) {
                tabData = GameDataModel.getSpellbookGroups();
            }

            list.dataProvider = new DataProvider(tabData);
            list.addEventListener(ListEvent.INDEX_CHANGE, handleTabChange, false, 0, true);			
            list.selectedIndex = 0;
        }
        
    // Protected Methods:        
        protected function handleTabChange(e:ListEvent):void {
            var data:Object = tabData[e.index];
            if (data != null) { 
                groupView.groupID = data.id;
            }
        }
        
        override public function toString():String { 
            return "[MMOKit Spellbook " + name + "]";
        }
    }
}
