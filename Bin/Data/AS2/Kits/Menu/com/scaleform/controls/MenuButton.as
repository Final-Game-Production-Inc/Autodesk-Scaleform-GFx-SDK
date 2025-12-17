/**************************************************************************

Filename    :   MenuButton.as
Content     :  Custom implementation of a Button for the Menu Kit.
                Contains some extra logic for handling mouse / keyboard
                input and focus indicators.
                
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
import gfx.controls.Button;

import gfx.motion.Tween;
import gfx.ui.InputDetails;
import gfx.ui.NavigationCode;

import mx.transitions.easing.Strong;

[InspectableList("disabled", "disableFocus", "visible", "toggle", "labelID", "disableConstraints", "enableInitCallback", "autoSize", "soundMap")]
class com.scaleform.controls.MenuButton extends Button {
	
// Public Properties:
    /** The target ._z value when the component is selected. */
    public var selectedZ:Number = -300;
    /** The target ._z value when the component is not selected. */
    public var normalZ:Number = 0;

// UI Elements:
    /** Reference to the small circular arrow MovieClip on the right side of the Button. */
    public var arrow:MovieClip;

// Initialization:
	public function MenuButton() { 
        super(); 
		Tween.init();
    }
    
// Public Properties:
    /** Override for handleInput that removes calling handlePress for a "keyHold" event. */
    public function handleInput(details:InputDetails, pathToFocus:Array):Boolean {
		switch(details.navEquivalent) {
			case NavigationCode.ENTER:
				var controllerIdx:Number = details.controllerIdx;
				if (details.value == "keyDown") {
					if (!pressedByKeyboard) { handlePress(controllerIdx); }
				} else {
					handleRelease(controllerIdx);
				}
				return true;
		}
		return false;
	}	

// Private Methods:
    private function configUI():Void {
        super.configUI();
        
        // Raise the Button's arrow a bit in the Z for a 3D effect.
        arrow._z = -450;
    }
    /** Override for handleMouseRollOver that also gives the component focus. */
	private function handleMouseRollOver(mouseIndex:Number):Void {
		if (_disabled) { return; }
		if ((!_focused && !_displayFocus) || focusIndicator != null) { setState("over"); } // Otherwise it is focused, and has no focusIndicator, so do nothing.
	    dispatchEventAndSound( { type:"rollOver", mouseIndex:mouseIndex } );
        
        // If the Button received a rollOver and it did not have focus, give it focus so any subsequent keyboard input will end up on the correct component.
        if ((!_focused && !_displayFocus) || focusIndicator != null) { Selection.setFocus(this); }
    }

    /** Override for changeFocus() that properly updates the focusIndicator and tweens the button on the Z based on .focused. */
	private function changeFocus():Void {
		if (focusIndicator == null) {			
            setState((_focused || _displayFocus) ? "over" : "out");
		}

        // Reset the state of pressedByKeyboard if focus leaves on keyPress.
        if (pressedByKeyboard && !_focused) {
            pressedByKeyboard = false;
        }
        
        // Tween the Button up/down (3Di) based on whether it has focus.
        if (focused) {                                			
            MovieClip(this).tweenTo(0.5, { _z:selectedZ }, Strong.easeOut);       
			playSound("buttonFocusIn");
        }
        else {				
            MovieClip(this).tweenTo(0.4, { _z:normalZ }, Strong.easeOut);                
        }

	}
}