/**************************************************************************

Filename    :   WindowMenuButton.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.windowmenu {
    
    import flash.display.DisplayObject;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.text.TextField;
    import flash.text.TextFieldAutoSize;
    
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.controls.Button;
    import scaleform.clik.events.ButtonEvent;

    public class WindowMenuButton extends Button {
        
    // Constants:
        
    // Public Properties:
        
        
    // Protected Properties:
        protected var _index:int;
        
    // UI Elements:
        public var menuButtonIcon:MovieClip;
        
    // Initialization:
        public function WindowMenuButton() {
            super();
        }
    
    // Public Methods:
        override public function toString():String { 
            return "[MMOKit WindowMenuButton " + name + "]";
        }
    
    // Protected Methods:
        override protected function configUI():void {
            super.configUI();
            toggle = true;
            // _disableFocus = true;
        }
        
        override protected function draw():void {
            super.draw();
            
            if (isInvalid(InvalidationType.STATE)) {            
                
            }
        }
    }
}