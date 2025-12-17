/**************************************************************************

Filename    :   SystemUI.as
Content     :  
                
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.views {
    
    import flash.display.MovieClip;
    
    import scaleform.clik.core.CLIK;
    import scaleform.clik.core.UIComponent;
    
    import scaleform.gfx.Extensions;
  
    dynamic public class SystemUI extends View {
        
        public function SystemUI() {
            if (Extensions["CLIK_addedToStageCallback"]) {
                Extensions["CLIK_addedToStageCallback"]("ErrorDialog", "ErrorDialog", this);
            }
            super();
        }
        
    }
}