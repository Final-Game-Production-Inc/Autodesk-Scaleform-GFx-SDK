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
package com.scaleform.controls {
    
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.controls.OptionStepper;
    import scaleform.clik.events.IndexEvent;

    public class MenuOptionStepper extends OptionStepper {
    
    // Constants:
        
    // Public Properties:
        
    // UI Elements:

    // Initialization:
        public function MenuOptionStepper() {
            super();
        }
        
    // Private Methods
        /** Override for configUI() that immediately updates the buttons' _visible property based on the selectedIndex. */
        override protected function configUI():void {
            super.configUI();
            addEventListener(IndexEvent.INDEX_CHANGE, updateButtonVisibility, false, 0, true);
            updateButtonVisibility(null);
        }
        
        /** Updates the _visible property of the prevBtn / nextBtn based on the selectedIndex to show the user if previous / next options exist. */
        private function updateButtonVisibility(e:IndexEvent):void {
            nextBtn.visible = (_selectedIndex == _dataProvider.length - 1) ? false : true;
            prevBtn.visible = (_selectedIndex == 0) ? false : true;
        }
    }
}