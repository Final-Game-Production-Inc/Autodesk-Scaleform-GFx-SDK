/**************************************************************************

Filename    :   PaperDollStatLine.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.paperdoll {
    
    import flash.events.FocusEvent;
    import flash.text.TextField;
    import flash.display.MovieClip;
    
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.data.ListData;
    import scaleform.clik.controls.ListItemRenderer;
    import scaleform.clik.constants.InvalidationType;
    
    public class PaperDollStatLine extends MovieClip {
        
    // Constants:
        
    // Public Properties:
        
    // Protected Properties:
        protected var _name:String;
        protected var _value:String;
        
    // UI Elements:
        public var tf_statName:TextField;
        public var tf_statVal:TextField;
    
    // Initialization:
        public function PaperDollStatLine() {
            super();
        }
        
    // Public getter / setters:
        public function get statName():String { return _name; }
        public function set statName(value:String):void {
            _name = value;
            tf_statName.text = _name;
        }
        
        public function get statVal():String { return _value; }
        public function set statVal(value:String):void {
            _value = value;
            tf_statVal.text = _value;
        }

        
    // Public Methods:
        override public function toString():String {
            return "[CLIK PaperDollStatLine " + name + "]";
        }
        
    // Protected Methods:
        
    }
}