/**************************************************************************

Filename    :   TextBox.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
package {
    
    import flash.display.MovieClip;
    
    import scaleform.clik.constants.InputValue;
    import scaleform.clik.controls.TextInput;
    import scaleform.clik.events.InputEvent;
    import scaleform.clik.ui.InputDetails;

    public class TextBox extends TextInput
    {
        override public function handleInput(event:InputEvent):void {
            super.handleInput(event);
            
            var details:InputDetails = event.details;
            if (details.value == InputValue.KEY_DOWN && details.code == 13) {
                var myParent:MovieClip = parent as MovieClip;
                var myGrandparent:MovieClip = myParent.parent as MovieClip;
                myGrandparent.onSubmit(text);
            }
        }
    }
}