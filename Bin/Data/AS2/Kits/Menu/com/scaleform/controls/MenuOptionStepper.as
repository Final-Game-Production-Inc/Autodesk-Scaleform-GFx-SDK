/**************************************************************************

Filename    :   MenuOptionStepper.as
Content     :  Extension of the standard OptionStepper that shows/hides
                the prevBtn and nextBtn based on the selectedIndex compared
                to the length of the dataProvider.
                
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
import gfx.controls.OptionStepper;

[InspectableList("disabled", "visible", "enableInitCallback", "soundMap")]
class com.scaleform.controls.MenuOptionStepper extends OptionStepper {

// Constants:

// Public Properties:
    
// UI Elements:

// Initialization:
    public function MenuOptionStepper() {
        super();
    }
    
// Public Methods:
    public function get selectedIndex():Number { return _selectedIndex; }
    public function set selectedIndex(value:Number):Void {
        var newIndex:Number = Math.max(0, Math.min(_dataProvider.length-1, value));
        if (newIndex == _selectedIndex) { return; }
        _selectedIndex = newIndex;
        
        // If we've changed the selectedIndex, update the buttons.
        updateButtonVisibility();
        updateSelectedItem();
    }
    
// Private Methods
    /** Override for configUI() that immediately updates the buttons' _visible property based on the selectedIndex. */
    private function configUI():Void {
        super.configUI();
        updateButtonVisibility();
    }
    
    /** Updates the _visible property of the prevBtn / nextBtn based on the selectedIndex to show the user if previous / next options exist. */
    private function updateButtonVisibility():Void {
        nextBtn._visible = (_selectedIndex == dataProvider.length - 1) ? false : true;
        prevBtn._visible = (_selectedIndex == 0) ? false : true;
    }
}