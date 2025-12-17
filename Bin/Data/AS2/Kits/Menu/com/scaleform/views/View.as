/**************************************************************************

Filename    :   View.as
Content     :  Base implementation of a ActionScript view in the Menu Kit.
                
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
import gfx.core.UIComponent;

class com.scaleform.views.View extends UIComponent
{
// Initialization:
    public function View() {
        super();
    }
    
// Public Methods:
    /** Sets focus to the target MovieClip using Selection.setFocus(). */
    public function setSelectionFocus(mc:MovieClip):Void {
        Selection.setFocus(mc);
    }
}