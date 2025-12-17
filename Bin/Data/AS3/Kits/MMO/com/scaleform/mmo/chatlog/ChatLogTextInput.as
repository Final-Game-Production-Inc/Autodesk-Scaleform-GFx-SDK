/**************************************************************************

Filename    :   ChatLogTextInput.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo.chatlog {
	
    import flash.text.TextField;
    import flash.events.MouseEvent;
    import flash.text.TextFormat;
    
    import scaleform.clik.events.InputEvent;
    import scaleform.clik.controls.TextInput;
    import scaleform.clik.utils.Constraints;
    import scaleform.clik.utils.ConstrainedElement;
        
    public class ChatLogTextInput extends TextInput {
    
    // Public Properties:
        public var inputTextFormat:TextFormat;
	
    // Protected Properties:        
        protected var _prefixText:String;
        
    // UI Elements:
        public var prefix:TextField;
        
        public function ChatLogTextInput() {
            super();
            prefix.visible = false;
            inputTextFormat = textField.getTextFormat();
        }        
        
        public function setPrefix(p:String):void {
            prefix.visible = true;
            _prefixText = p;
            invalidateData();
        }
        public function clearPrefix():void {
            if (!prefix.visible) return;
            prefix.visible = false;
            invalidateData();
        }
        public function isPrefixVisible():Boolean {
            return prefix.visible;
        }
        
        override protected function configUI():void {           
            constraints.addElement("textField", textField, Constraints.ALL);
            constraints.addElement("prefix", prefix, Constraints.ALL);
            
            addEventListener(InputEvent.INPUT, handleInput, false, 0, true);
            addEventListener(MouseEvent.MOUSE_DOWN, handleMouseDown, false, 0, true);
            setState(defaultState, "default");
            
            constraintsDisabled = false;            
        }
         
        protected function reflowContent():void {
            var pad:Number = 5;
            
            var element:ConstrainedElement = constraints.getElement("textField");
            if (prefix.visible) {
                element.left = prefix.x + prefix.textWidth + pad;
            } else {
                element.left = prefix.x;

            }
            
            if (!constraintsDisabled) {
                constraints.update(_width, _height);
            }
        }        
            
        override protected function updateText():void {
            super.updateText();
            if (prefix.visible) {
                prefix.htmlText = _prefixText;
                prefix.setTextFormat(inputTextFormat);
            } else {
                prefix.htmlText = "";
            }
            textField.defaultTextFormat = inputTextFormat;
            
            reflowContent();
        }        
        
        override protected function updateAfterStateChange():void {
            if (!initialized) { return; }
            constraints.updateElement("prefix", prefix);
            
            super.updateAfterStateChange();
        }
    }
}
