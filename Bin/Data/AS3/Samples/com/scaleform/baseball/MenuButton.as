/**************************************************************************

Filename    :   MenuButton.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
 
package com.scaleform.baseball {
	
	import flash.text.TextField;
	import flash.text.TextFieldAutoSize;
    
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.controls.Button;
	import scaleform.clik.events.ComponentEvent;
	import scaleform.clik.utils.Constraints;
    
    public class MenuButton extends Button {
        
    // Constants:
        
    // Public Properties:
        
    // Protected Properties:
        
    // Initialization:
        public function MenuButton() {
            super();
        }
        
    // Protected Methods:
        override protected function draw():void {
            // State is invalid, and has been set (is not the default)
            if (isInvalid(InvalidationType.STATE)) {
                if (_newFrame) {
                    gotoAndPlay(_newFrame);
                    _newFrame = null;
                }
                
                if (_newFocusIndicatorFrame) {
                    focusIndicator.gotoAndPlay(_newFocusIndicatorFrame);
                    _newFocusIndicatorFrame = null;
                }
                
                updateAfterStateChange();
                dispatchEvent(new ComponentEvent(ComponentEvent.STATE_CHANGE));
                invalidate(InvalidationType.DATA); // , InvalidationType.SIZE);
            }
            
            // Data is invalid when label or autoSize changes.
            if (isInvalid(InvalidationType.DATA)) {
                updateText();
                if (autoSize != TextFieldAutoSize.NONE) {
                    invalidateSize();
                }
            }
            
            // Resize and update constraints
            if (isInvalid(InvalidationType.SIZE)) {
                alignForAutoSize();
                // setActualSize(_width, _height);
                if (!constraintsDisabled) {
                    constraints.update(_width, _height);
                }
            }
        }
    }
}