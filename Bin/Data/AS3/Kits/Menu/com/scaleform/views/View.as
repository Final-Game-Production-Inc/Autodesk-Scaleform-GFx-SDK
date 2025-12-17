/**************************************************************************

Filename    :   View.as
Content     :  Base implementation of a ActionScript view in the Menu Kit.
                
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package com.scaleform.views {
    
    import scaleform.clik.core.UIComponent;
    import flash.display.MovieClip;
    
    import scaleform.clik.core.CLIK;
    import scaleform.gfx.Extensions;
  
    dynamic public class View extends UIComponent {
            
        var container:MovieClip;
        var modalBG:MovieClip;

        public function View() {
            super();
        }
        
        /** Sets focus to the target MovieClip using Selection.setFocus(). */
        public function setSelectionFocus(mc:MovieClip):void {
            if (mc is UIComponent) {
                mc.focused = 1;
            }
            else if (mc) {
                stage.focus = mc;
            }
        }
    }
}