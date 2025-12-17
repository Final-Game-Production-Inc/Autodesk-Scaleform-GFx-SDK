/**************************************************************************

Filename    :   FocusItemRenderer.as
Content     :  The base itemRenderer implementation for the Menu Kit.
                Contains some extra logic for handling mouse / keyboard
                input and focus indicators.
                
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
import gfx.controls.ListItemRenderer;

import gfx.controls.ScrollingList;

import gfx.motion.Tween;
import gfx.ui.InputDetails;
import gfx.ui.NavigationCode;

import mx.transitions.easing.Strong;

[InspectableList("disabled", "visible", "labelID", "disableConstraints")]
class com.scaleform.controls.FocusItemRenderer extends ListItemRenderer {
    
    var owner:ScrollingList;
    var selectedZ:Number = -300;
    var arrow:MovieClip;
    var normalZ:Number = 0;
    
    public function FocusItemRenderer() { 
        super(); 
        Tween.init();
    }

    public function setListData(index:Number, label:String, selected:Boolean):Void {
        super.setListData(index, label, selected);
    }
    
    // A public version of parent method updateAfterStateChange() (Button.as).
    public function updateAfterStateChange():Void {
        // Redraw should only happen AFTER the initialization.
        if (!initialized) { return; }
        validateNow();// Ensure that the width/height is up to date.
        
        arrow._z = -450;
        
        if (textField != null && _label != null) { textField.text = _label; }
        if (constraints != null) { 
            constraints.update(width, height);
        }
        dispatchEvent({type:"stateChange", state:state});
    }
    
    public function setData(data:Object):Void {
        // If we received null data, hide this renderer.
        if (data == undefined) { 
            this._visible = false;
            return; 
        }
        
        this.data = data;
        invalidate();
        this._visible = true;
    }
    
    // Overides handleMouseRollOver to include Selection.setFocus(this) when appropriate.
    // #state RollOver is only called by mouse interaction. Focus change happens in changeFocus.
    private function handleMouseRollOver(mouseIndex:Number):Void {
        if (_disabled) { return; }
        if ((!_focused && !_displayFocus) || focusIndicator != null) { setState("over"); } // Otherwise it is focused, and has no focusIndicator, so do nothing.
        dispatchEventAndSound( { type:"rollOver", mouseIndex:mouseIndex } );
        if ((!_focused && !_displayFocus) || focusIndicator != null) { Selection.setFocus(this); }
    }
    
    /** Override for handleInput that removes calling handlePress for a "keyHold" event. */
    public function handleInput(details:InputDetails, pathToFocus:Array):Boolean {
        switch(details.navEquivalent) {
            case NavigationCode.ENTER:
                var controllerIdx:Number = details.controllerIdx;
                if (details.value == "keyDown") { // A more generic solution may be required for Button
                    if (!pressedByKeyboard) { handlePress(controllerIdx); }
                } else {
                    handleRelease(controllerIdx);
                }
                return true; // Even though the press may not have handled it (automatic=repeat presses), we want to indicate that we did, since the Button handles repeat, but it won't respond to multiple.
        }
        return false;
    }
    
    /*
     * Customized logic in changeFocus() to:
     * A. Display a focusIndicator.
     * B. Tween on the Z.
     */
    private function changeFocus():Void {
        if (_disabled) { return; }
        if (focusIndicator == null) {
            setState((_focused || _displayFocus) ? "over" : "out");
        }
        if (focusIndicator != null) {
            if (owner.focused && this.selected) {
                focusIndicator.gotoAndPlay("show");
                MovieClip(this).tweenTo(0.5, { _z:selectedZ }, Strong.easeOut);
            }
            else {
                focusIndicator.gotoAndPlay("hide");
                MovieClip(this).tweenTo(0.4, { _z:normalZ }, Strong.easeOut);
            }
            if (pressedByKeyboard && !_focused) {
                setState("kb_release"); 
                pressedByKeyboard = false;
            }
        }
    }
}