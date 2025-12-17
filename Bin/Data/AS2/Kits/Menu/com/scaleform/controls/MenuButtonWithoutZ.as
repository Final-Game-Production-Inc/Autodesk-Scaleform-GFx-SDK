/**************************************************************************

Filename    :   MenuButtonWithoutZ.as
Content     :  A MenuButton without the Z tweening for the Menu Kit's
                dialogs.
                
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
import com.scaleform.controls.MenuButton;

[InspectableList("disabled", "disableFocus", "visible", "toggle", "labelID", "disableConstraints", "enableInitCallback", "autoSize", "soundMap")]
class com.scaleform.controls.MenuButtonWithoutZ extends MenuButton {
	
// Public Properties:

// UI Elements:

// Initialization:
	public function MenuButtonWithoutZ() { 
        super(); 
    }
    
// Public Properties:

// Private Methods:
    /** Override for changeFocus() that properly updates the focusIndicator and tweens the button on the Z based on .focused. */
	private function changeFocus():Void {
		if (focusIndicator == null) {			
            setState((_focused || _displayFocus) ? "over" : "out");
		}

        // Reset the state of pressedByKeyboard if focus leaves on keyPress.
        if (pressedByKeyboard && !_focused) {
            pressedByKeyboard = false;
        }
	}
}