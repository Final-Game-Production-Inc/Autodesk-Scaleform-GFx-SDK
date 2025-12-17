/**************************************************************************

Filename    :   MultiControlItemRenderer.as
Content     :  Implementation of a ListItemRenderer with multiple 
                sub-components (checkBox / optionStepper). 
                
                Used in the SettingsView of the Menu Kit.
                
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
class com.scaleform.controls.MultiControlItemRenderer extends FocusItemRenderer {

// Constants:
    static public var TYPE_Boolean:Number   = 0;
    static public var TYPE_Integer:Number   = 1;
    static public var TYPE_Float:Number     = 2;

// Public Properties:
    public var control:MovieClip;
    public var optionstepper:gfx.controls.OptionStepper;
    public var checkbox:gfx.controls.CheckBox;
    
    public var type:Number;
    public var optionIndex:Number;
    
// Private Properties
    private var _bDataUpdate:Boolean = false;
    private var hit:MovieClip;
    
// Initialization:
    public function MultiControlItemRenderer() { super(); }

// Public Methods:
    public function get selected():Boolean { return _selected; }
    public function set selected(value:Boolean):Void {
        super.selected = value;
    }
    
    private function changeFocus():Void {
        super.changeFocus();
        var state:String = (_selected) ? "focused" : "default";
        
        // Update the sub-component to reflect the state of the list item renderer.
        if (!control.disabled && control != null)
        {
            control.gotoAndPlay(state);
        }
        // "control" may be null / undefined the first time this method is called.
        else if (!optionstepper.disabled)
        {
            optionstepper.gotoAndPlay(state);
        }
    }
    
    public function setData(data:Object) {
        if (data) {
            this.type = data.type;
            this.optionIndex = data.optIndex;
        }
        super.setData(data);
    }
    
    public function draw() {
        super.draw();
        switch (type)
        {
            case (TYPE_Boolean):
                control = checkbox;
                checkbox._visible = true;
                optionstepper._visible = false;
                
                checkbox.selected = data.bIsEnabled;
                break;
            
            case (TYPE_Integer):
            case (TYPE_Float):
                control = optionstepper;
                checkbox._visible = false;
                optionstepper._visible = true;
            
                // Remove the "change" listener so we don't fire a change event when the renderer is updating.
                optionstepper.removeEventListener("change", this, "onStepper_change");
                
                // Update the optionStepper with the latest dataProvider and selectedIndex.
                // These updates will be disregarded by the component if they are redundant. 
                optionstepper.dataProvider = data.dataProvider;
                optionstepper.selectedIndex = data.optIndex;
                
                // Re-add the "change" listener so we listen for user changes.
                optionstepper.addEventListener("change", this, "onStepper_change");
                break;
            
            default:
                break;
        }
    }
    
    public function handleInput(details:InputDetails, pathToFocus:Array):Boolean {
        var nextItem:MovieClip = MovieClip(pathToFocus.shift());
        var handled:Boolean;
        if (nextItem != null) {
            handled = nextItem.handleInput(details, pathToFocus);
            if (handled) { return true; }
        }
        
        // If the user presses ENTER and we're displaying a check box, toggle the check box.
        if (details.navEquivalent == NavigationCode.GAMEPAD_A && details.value == "keyDown" && details.code != 32)
        {
            if (type == TYPE_Boolean)
            {
                // Fire a press sound event since none will be fired by the button.
                playSound("embeddedCompChanged");
                checkbox.selected = !checkbox.selected;
                return true;
            }
        }

        // If the user presses LEFT or RIGHT and we're displaying an OptionStepper, pass it to the OptionStepper.
        if ( (details.navEquivalent == "left" || details.navEquivalent == "right") && (details.value == "keyDown" || details.value == "keyHold")) {
            if (type == TYPE_Float || type == TYPE_Integer)
            {
                // Fire a press sound event since none will be fired by the button.
                handled = optionstepper.handleInput(details, pathToFocus);
                if (handled) { 
					playSound("embeddedCompChanged"); 
					return true; 
				}
            }
        }
        
        return false; // or true if handled
    }
    
    // Event for when the option stepper's value has been changed.
    public function onStepper_change(event:Object) {
        if (data.optIndex == event.target.selectedIndex) { return; }
        
        data.optIndex = event.target.selectedIndex;
        data.selection = event.target.dataProvider[data.optIndex];
        
        // Dispatch a change event to alert the list our selectedIndex has changed.
        // var event:Object = { type:"change" };
        // dispatchEvent(event);
        // playSound("change"); // playSound() since dispatchEventAndSound() seems not to work in this case.
    }
    
    public function onCheckBox_select(event:Object) {
        data.bIsEnabled = checkbox.selected;
        		
        // Avoid letting the checkBox steal focus if the mouse wasn't there. This can happen if its .selected
        // property changes during a dataProvider update when the user is uninvolved.
        if (checkbox.focused > 0) {
            owner.focused = 1;
        }
    }
    
    private function onSubComponent_rollOver(event:Object) {
        owner.selectedIndex = index;
    }
    
    /** @exclude */
    public function toString():String {
        return "[Scaleform MultiControlItemRenderer " + _name + "]";
    }
    
// Private Methods:
    private function configUI():Void {
        constraints = new Constraints(this, true);
        if (!_disableConstraints) {
            constraints.addElement(textField, Constraints.ALL);
        }
        
        // Force dimension check if autoSize is set to true
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
        hit.tabEnabled = false;
        
        focusTarget = owner;
        
        if (optionstepper != null) {
            optionstepper.tabEnabled = false;
            optionstepper.tabChildren = false;
            optionstepper.focusEnabled = false;
            
            optionstepper.nextBtn.addEventListener("rollOver", this, "onSubComponent_rollOver");
            optionstepper.prevBtn.addEventListener("rollOver", this, "onSubComponent_rollOver");
            optionstepper.nextBtn.addEventListener("press", this, "onSubComponent_rollOver");
            optionstepper.prevBtn.addEventListener("press", this, "onSubComponent_rollOver");
			optionstepper.nextBtn.soundMap.theme = "optStep";
            optionstepper.prevBtn.soundMap.theme = "optStep";
			
        }
        
        if (checkbox != null) {
            checkbox.addEventListener("select", this, "onCheckBox_select");
            checkbox.addEventListener("rollOver", this, "onSubComponent_rollOver");
            
            checkbox.tabEnabled = false;
            checkbox.tabChildren = false;
            checkbox.focusEnabled = false;
            checkbox.focusTarget = owner;
			checkbox.soundMap.theme = "check";
        }
        
        updateAfterStateChange();
    }
}