/**************************************************************************

Filename    :   OptionItemRenderer.as
Content     :  ??? NFM: Can we delete this?
                
Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
import com.scaleform.controls.FocusItemRenderer;

import gfx.ui.InputDetails;
import gfx.ui.NavigationCode;
import gfx.utils.Delegate;
import gfx.utils.Constraints;

[InspectableList("disabled", "visible", "labelID", "disableConstraints"]
class com.scaleform.controls.OptionItemRenderer extends FocusItemRenderer {
// Constants:

// Public Properties:
    private var hit:MovieClip;
    public var optionstepper:gfx.controls.OptionStepper;
    
// Initialization:
    public function OptionItemRenderer() { super(); }

// Public Methods:
    public function get selected():Boolean { return _selected; }
    public function set selected(value:Boolean):Void {
        super.selected = value;
        if (optionstepper) {
            optionstepper.gotoAndPlay( (value) ? "focused" : "default" );
        }
    }
    
    public function setData(data:Object):Void {
        super.setData(data);
        
        //Set the dataProvider for the optionStepper
        if (optionstepper) {
            optionstepper.selectedIndex = data.optIndex;
            optionstepper.dataProvider = data.dataProvider;
        }
    }
    
    public function handleInput(details:InputDetails, pathToFocus:Array):Boolean {
        var nextItem:MovieClip = MovieClip(pathToFocus.shift());
        var handled:Boolean;
        if (nextItem != null) {
            handled = nextItem.handleInput(details, pathToFocus);
            if (handled) { return true; }
        }

        if (details.navEquivalent == "left" || details.navEquivalent == "right") {
            handled = optionstepper.handleInput(details, pathToFocus);
            if (handled) { return true; }
        }
        
        return false; // or true if handled
    }
    
    //Event for when the option stepper's value has been changed.
    function onValueChange(event:Object) {    
        data.optIndex = event.target.selectedIndex;
        data.selection = event.target.dataProvider[data.optIndex];
    }
    
       /** @exclude */
    public function toString():String {
        return "[Scaleform OptionItemRenderer " + _name + "]";
    }
    
// Private Methods:
    private function configUI():Void {
        constraints = new Constraints(this, false);
        if (!_disableConstraints) {
            constraints.addElement(textField, Constraints.ALL);
        }
        
        if (_autoSize != "none") {
            sizeIsInvalid = true;
        }
        
        hit.onRollOver = Delegate.create(this, handleMouseRollOver);
        hit.onRollOut = Delegate.create(this, handleMouseRollOut);
        hit.onPress = Delegate.create(this, handleMousePress);
        hit.onRelease = Delegate.create(this, handleMouseRelease);
        hit.onDragOver = Delegate.create(this, handleDragOver);
        hit.onDragOut = Delegate.create(this, handleDragOut);
        hit.onReleaseOutside = Delegate.create(this, handleReleaseOutside);
        
        focusTarget = owner;
        
        if (optionstepper) {
            optionstepper.addEventListener("change", this, "onValueChange");
            optionstepper.tabEnabled = false;
        }
        
        updateAfterStateChange();
    }
}