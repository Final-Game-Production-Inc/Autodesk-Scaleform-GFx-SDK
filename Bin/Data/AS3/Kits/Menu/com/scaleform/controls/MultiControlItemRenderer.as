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
package com.scaleform.controls {

    import flash.display.MovieClip;
    import flash.display.DisplayObject;
    import flash.events.Event;
    import flash.events.MouseEvent;
    
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.controls.ScrollingList;
    import scaleform.clik.events.InputEvent;
    import scaleform.clik.events.ListEvent;
    import scaleform.clik.events.ButtonEvent;
    import scaleform.clik.interfaces.IListItemRenderer;
    import scaleform.clik.constants.InputValue;
    
    import scaleform.clik.ui.InputDetails;
    import scaleform.clik.constants.NavigationCode;
    import scaleform.clik.utils.Constraints;
    
    import com.scaleform.events.MenuListEvent;
    import com.scaleform.controls.FocusItemRenderer;
    import com.scaleform.controls.MenuOptionStepper;
    import scaleform.clik.controls.CheckBox;
    
    public class MultiControlItemRenderer extends FocusItemRenderer {
        
    // Constants:
        static public var TYPE_Boolean:uint   = 0;
        static public var TYPE_Integer:uint   = 1;
        static public var TYPE_Float:uint     = 2;
        
    // Public Properties:
        public var type:uint;
        public var optionIndex:Number;
        
    // Private Properties
        protected var _bDataUpdate:Boolean = false;
        
    // UI Elements:
        public var hit:MovieClip;
        public var optionstepper:MenuOptionStepper;
        public var checkbox:MenuCheckBox;
        public var control:UIComponent;
        public var item:MovieClip;
        
    // Initialization:
        public function MultiControlItemRenderer() {
            super();
        }
        
    // Getter / Setters:
        /** Override for set enabled() that keeps mouseChildren true for MultiControlItemRenderer. */
        [Inspectable(defaultValue="true")]
        override public function get enabled():Boolean { return super.enabled; }
        override public function set enabled(value:Boolean):void {
            super.enabled = value;
            mouseChildren = true; // Need mouseChildren to be true for MultiControlItemRenderer so children can be interactive.
        }
        
    // Public Methods:
        override protected function changeFocus():void {
            super.changeFocus();
            var state:String = (_selected) ? "focused" : "default";
            
            // Update the sub-component to reflect the state of the list item renderer.
            if (control && control.enabled) {
                // Checkbox doesn't use "focused". Instead, need to check whether it's selected since that's PART of its state.
                if (control == checkbox) { 
                    control.gotoAndPlay( (checkbox.selected) ? "selected_selecting" : "default" );
                } else {
                    control.gotoAndPlay( state );
                }
            }
            // "control" may be null / undefined the first time this method is called.
            else if (optionstepper.enabled) {
                optionstepper.gotoAndPlay(state);
            }
        }
        
        override public function setData(data:Object):void {
            if (data) {
                type = data.type;
                optionIndex = data.optIndex;
            }
            
            super.setData(data);
        }
        
        override protected function draw():void {
            super.draw();
            if (isInvalid(InvalidationType.DATA)) {
                switch (type) {
                    case (TYPE_Boolean):
                        control = checkbox;
                        checkbox.visible = true;
                        optionstepper.visible = false;
                        
                        if (data) { 
                            checkbox.selected = data.bIsEnabled; 
                        }
                        break;
                    
                    case (TYPE_Integer):
                    case (TYPE_Float):
                        control = optionstepper;
                        checkbox.visible = false;
                        optionstepper.visible = true;
                    
                        // Remove the "change" listener so we don't fire a change event when the renderer is updating.
                        optionstepper.removeEventListener(Event.CHANGE, onStepper_change);
                        
                        // Update the optionStepper with the latest dataProvider and selectedIndex.
                        // These updates will be disregarded by the component if they are redundant. 
                        if (data) {
                            optionstepper.dataProvider = data.dataProvider;
                            optionstepper.selectedIndex = data.optIndex;
                        }
                        
                        // Re-add the "change" listener so we listen for user changes.
                        optionstepper.addEventListener(Event.CHANGE, onStepper_change, false, 0, true);
                        break;
                    
                    default:
                        break;
                }
            }
        }
        
        override public function handleInput(event:InputEvent):void {
                if (event.isDefaultPrevented()) { return; }
                var details:InputDetails = event.details;
                var index = details.controllerIndex;
                
                var handled:Boolean = false;
                switch (details.navEquivalent) {
                    case NavigationCode.LEFT:
                    case NavigationCode.RIGHT:
                        // If the user presses LEFT or RIGHT and we're displaying an OptionStepper, pass it to the OptionStepper.
                        if (type == TYPE_Float || type == TYPE_Integer) {
                            optionstepper.handleInput(event);
                            //if (handled) { event.handled = true;}
                        }
                        break;
                        
                    case NavigationCode.ENTER:
                    case NavigationCode.GAMEPAD_A:
                        // If the user presses ENTER and we're displaying a check box, toggle the check box.
                        if (details.value == InputValue.KEY_DOWN && details.code != 32) {
                            if (type == TYPE_Boolean) {
                                checkbox.selected = !checkbox.selected;
                                event.handled = true;
                            }
                        }
                        break;
                        
                    default:
                        break;
                }
            }
        
        
        /** @exclude */
        override public function toString():String {
            return "[Scaleform MultiControlItemRenderer " + name + "]";
        }
        
    // Private Methods:
        override protected function configUI():void {
            if (!constraintsDisabled) {  
                constraints.addElement("textField", textField, Constraints.ALL);
            }
            
            if (optionstepper != null) {
                if (!constraintsDisabled) { 
                    constraints.addElement("optionstepper", optionstepper, Constraints.RIGHT);
                }
                
                optionstepper.focusTarget = owner;
                optionstepper.addEventListener(Event.CHANGE, onStepper_change, false, 0, true);
                optionstepper.nextBtn.addEventListener(MouseEvent.ROLL_OVER, onSubComponent_rollOver, false, 0, true);
                optionstepper.prevBtn.addEventListener(MouseEvent.ROLL_OVER, onSubComponent_rollOver, false, 0, true);
            }
            
            if (checkbox != null) {
                if (!constraintsDisabled) { 
                    constraints.addElement("checkbox", checkbox, Constraints.RIGHT);
                }
                
                checkbox.focusTarget = owner;
                checkbox.addEventListener(Event.SELECT, onCheckBox_select, false, 0, true);
                checkbox.addEventListener(MouseEvent.ROLL_OVER, onSubComponent_rollOver, false, 0, true);
            }
            
            if (hit != null) {
                hit.mouseEnabled = false;
                hit.mouseChildren = false;
            }
        }
        
        // Event for when the option stepper's value has been changed.
        protected function onStepper_change(event:Event) {
            if (!data) { return; }
            if (data.optIndex == event.target.selectedIndex) { return; }
            
            data.optIndex = event.target.selectedIndex;
            data.selection = event.target.dataProvider[data.optIndex];
            
            // Dispatch a change event to alert the list our selectedIndex has changed.
            // var event:Object = { type:"change" };
            // dispatchEvent(event);
            // playSound("change"); // playSound() since dispatchEventAndSound() seems not to work in this case.
        }
        
        protected function onCheckBox_select(event:Event) {
            if (!data) { return; }
            data.bIsEnabled = checkbox.selected;
            
            // Avoid letting the checkBox steal focus if the mouse wasn't there. This can happen if its .selected
            // property changes during a dataProvider update when the user is uninvolved.
            if (checkbox.focused > 0) {
                owner.focused = 1;
            }
        }
        
        protected function onSubComponent_rollOver(event:MouseEvent) {
            (owner as ScrollingList).selectedIndex = index;
        }
    }
}