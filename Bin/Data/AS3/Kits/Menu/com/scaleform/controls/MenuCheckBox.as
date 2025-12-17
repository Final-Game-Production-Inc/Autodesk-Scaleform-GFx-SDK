/**********************************************************************

 Filename    :  MenuCheckBox.as
 Content     :  
 Authors     :  Prasad Silva, Nate Mitchell
 Copyright   :  (c) 2005-2011 Scaleform Corp. All Rights Reserved.

 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
 THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

 **********************************************************************/
package com.scaleform.controls {
    
    import flash.text.TextFieldAutoSize;
    
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.controls.CheckBox;
    
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.events.ComponentEvent;
    
    public class MenuCheckBox extends CheckBox {
    // Constants:

    // Public Properties:
        
    // UI Elements:

    // Initialization:
        public function MenuCheckBox() {
            super();
        }
        
        // Override for draw() that removes the invalidate(InvalidationType.SIZE) within InvalidationType.STATE
        // and all the other unnecessary code from Button.
        override protected function draw():void {
            if (isInvalid(InvalidationType.STATE)) {
                if (_newFrame) {
                    gotoAndPlay(_newFrame);
                    _newFrame = null;
                }
                
                updateAfterStateChange();
                dispatchEvent(new ComponentEvent(ComponentEvent.STATE_CHANGE));
                invalidate(InvalidationType.DATA); // , InvalidationType.SIZE);
            }
        }
    }
}