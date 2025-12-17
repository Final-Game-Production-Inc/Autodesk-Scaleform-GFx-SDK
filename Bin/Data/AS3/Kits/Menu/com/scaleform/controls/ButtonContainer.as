/**************************************************************************

Filename    :   ButtonContainer.as
Content     :  ButtonContainer class to allow Flash artist to manipulate
                soundMap for a component within Flash Studio even if its 
                been wrapped inside a container to allow for more complex
                tweening on the embedded component.
                
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.controls {
    
    import scaleform.clik.controls.Button;
    import scaleform.clik.core.UIComponent;
    
    public class ButtonContainer extends UIComponent {
    // Constants:
    
    // Public Properties:
        
    // UI Elements:
        /** Reference to the Button contained by this object. This Button's soundMap will match this object's soundMap. */
        public var btn:Button;

    // Initialization:
        public function ButtonContainer() {
            enableInitCallback = true;
            super();
        }
    }
}