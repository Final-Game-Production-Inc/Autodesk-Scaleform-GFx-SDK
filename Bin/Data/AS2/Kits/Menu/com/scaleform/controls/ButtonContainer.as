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
import gfx.core.UIComponent;

[InspectableList("enableInitCallback", "soundMap")]
class com.scaleform.controls.ButtonContainer extends UIComponent {
    
// Constants:

// Public Properties:
    /** Mapping between events and sound process. */
    [Inspectable(type="Object", defaultValue="theme:default,focusIn:focusIn,focusOut:focusOut,select:select,rollOver:rollOver,rollOut:rollOut,press:press,doubleClick:doubleClick,click:click")]
    public var soundMap:Object = { theme:"default", focusIn:"focusIn", focusOut:"focusOut", select:"select", rollOver:"rollOver", rollOut:"rollOut", press:"press", doubleClick:"doubleClick", click:"click" };
    
// UI Elements:
    /** Reference to the Button contained by this object. This Button's soundMap will match this object's soundMap. */
    public var btn:MovieClip;

// Initialization:
    public function ButtonContainer() {
        super();
        btn.soundMap = this.soundMap;
    }
}