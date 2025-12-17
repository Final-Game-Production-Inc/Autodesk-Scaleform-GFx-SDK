/**********************************************************************
 Filename   :   SpellbookGroupView.as
 Content    :   
                
 Copyright   :  (c) 2005-2011 Scaleform Corp. All Rights Reserved.

 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
 THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.
 **********************************************************************/
package com.scaleform.mmo.spellbook {
    
    import com.scaleform.mmo.GameDataModel;
    import flash.display.MovieClip;
    import flash.text.TextField;
    import scaleform.clik.core.UIComponent;
    
    import scaleform.clik.controls.Button;
    
    public class SpellbookGroupView extends UIComponent {
        
    // Constants:
    
    // Public Properties:
        public var page:SpellbookPage;
        public var pageText:TextField;
        public var pagePrevious:Button;
        public var pageNext:Button;
    
    // Protected Properties:
        protected var _groupId:Number;
        protected var _spells:Array;
    
    // UI Elements:
        
    // Initialization:
        public function SpellbookGroupView() {
            super();
        }
        
    // Public Getter / Setters:
        public function get groupID():Number { return _groupId; }
        public function set groupID(value:Number):void {
            _groupId = value;
            updateView();
        }

    // Public Methods:
        override public function toString():String { 
            return "[MMOKit SpellbookGroupView " + name + "]";
        }	
        
    // Protected Methods:
        protected function updateView():void {
            if (GameDataModel.getSpellsByGroup != null) {
                _spells = GameDataModel.getSpellsByGroup(_groupId);
                // TODO: Handle page management; for now we're assuming 1 page
                page.setData(_spells, 0, _spells.length);
            }			
        }
    
        override protected function configUI():void {
            super.configUI();
            
            if (_groupId) updateView();
        }
    }
}
