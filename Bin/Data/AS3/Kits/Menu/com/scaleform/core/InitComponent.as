/**************************************************************************

Filename    :   InitComponent.as
Content     :  
                
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.core {
    
    import scaleform.clik.core.UIComponent
    
    public class InitComponent extends UIComponent {
    
    // Constants:
    // Public Properties:
    // Protected Properties:
    // UI Elements:
    
    // Initialization:
        public function InitComponent() {
            enableInitCallback = true;
            super();
        }
    }
}
