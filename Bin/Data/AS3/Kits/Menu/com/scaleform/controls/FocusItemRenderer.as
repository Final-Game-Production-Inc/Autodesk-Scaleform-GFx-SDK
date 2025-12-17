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
package com.scaleform.controls {
    
    import flash.display.MovieClip;
    import flash.display.DisplayObject;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.text.TextField;
    import flash.text.TextFieldAutoSize;
    
    import fl.transitions.easing.*;
    
    import scaleform.clik.events.ButtonEvent;
    import scaleform.gfx.MouseEventEx;
    
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.controls.ListItemRenderer;
    import scaleform.clik.controls.ScrollingList;
    import scaleform.clik.events.ComponentEvent;
    import scaleform.clik.motion.Tween;
    import scaleform.clik.ui.InputDetails;
    import scaleform.clik.constants.NavigationCode;
    
    public class FocusItemRenderer extends ListItemRenderer {
        
    // Constants:
    
    // Public Properties:
    
    // Protected Properties:
        protected var selectedZ:Number = -300;
        protected var normalZ:Number = 0;
        protected var activeTween:Tween;
        
    // UI Elements:
        public var arrow:MovieClip;
        
    // Initialization:
        public function FocusItemRenderer() {
            super();
        }
        
    // Public Getter / Setters
        override public function get index():uint { return _index; }
        override public function set index(value:uint):void { 
            _index = value;
        }
        
    // Public Methods:
        override public function setData(data:Object):void {
            // If we received null data, hide this renderer.
            this.visible = (data) ? true : false;
            if (!data) { return; }
            
            super.setData(data);
            
            // Immediately invalidate the renderer.
            invalidateData();
        }
        
        public function changeState( state:String ):void {
            _newFrame = state; // Override any state change that may be occuring simultaneously.
            gotoAndPlay( state );
            updateText();
        }
        
        /** 
         * Override that does not invalidateSize() during a invalidateState(). Required since our focusIndicator 
         * expands outside the bounds of the listItemRenderer itself which will cause timeline animations to stop 
         * and the itemRenderer itself to resize on a rollOver. 
         */
        override protected function draw():void {
            // State is invalid, and has been set (is not the default)
            if (isInvalid(InvalidationType.STATE)) {
                if (_newFrame) {
                    gotoAndPlay(_newFrame);
                    _newFrame = null;
                }
                
                if (_newFocusIndicatorFrame) {
                    if (focusIndicator != null) { 
                        focusIndicator.gotoAndPlay(_newFocusIndicatorFrame);
                    }
                    _newFocusIndicatorFrame = null;
                }
                
                updateAfterStateChange();
                dispatchEvent(new ComponentEvent(ComponentEvent.STATE_CHANGE));
                invalidate(InvalidationType.DATA); // , InvalidationType.SIZE); // NFM: Removed to avoid breaking timeline animations during a mouseOver.
            }
            
            // Data is invalid when label or autoSize changes.
            if (isInvalid(InvalidationType.DATA)) {
                updateText();
                if (autoSize != TextFieldAutoSize.NONE) {
                    alignForAutoSize();
                    invalidateSize();
                }
            }
        }
        
        // Override for handleMouseRollOut to fix a ROLL_OUT issue with the menu (see below)
        override protected function handleMouseRollOut(event:MouseEvent):void {
            var sfEvent:MouseEventEx = event as MouseEventEx;
            var index:uint = (sfEvent == null) ? 0 : sfEvent.mouseIdx;
            
            // DRAG_OUT
            if (event.buttonDown) {
                dispatchEvent(new ButtonEvent(ButtonEvent.DRAG_OUT));
                if (Boolean(_mouseDown & 1 << index)) {
                    if (stage != null) { // Button may have just been removed from stage, firing a MouseRollOut. Stage will be null in this case.
                        stage.addEventListener(MouseEvent.MOUSE_UP, handleReleaseOutside, false, 0, true);
                    }
                }
                
                if (lockDragStateChange || !enabled) { return; }
                if (_focused || _displayFocus) {
                    setState(_focusIndicator == null ? "release" : "kb_release"); // The user has pressed and dragged out of the component. Do a down->up transition.
                } else {
                    setState("out");
                }
            }
            
            // ROLL_OUT
            else {
                if (!enabled) { return; }
                if (_displayFocus || _focused) {
                    if (_focusIndicator != null) { setState("out"); }
                }
                else {
                    // Fix for the case where you roll over the last item in the list using mouse, move focus
                    // to the button below, move mouse out of the last item renderer. This change prevents the 
                    // "out" state from being played in this case (since the list isn't focused, it looks strange).
                    // setState("out"); 
                }
            }
        }
        
        /** Override for changeFocus that handles the focusIndicator and custom tweening for the itemRenderer. */
        override protected function changeFocus():void {
            if (!enabled) { return; }
            else {
                var bDisplayFocus:Boolean = (owner.focused > 0) && this.selected;
                _newFocusIndicatorFrame = bDisplayFocus ? "show" : "hide"; // State will be invalidated by setState call below.
                
                /*
                // AS2 Tween code for the selectedIndex.
                var t_duration:Number = bDisplayFocus ? 0.5 : 0.4;
                var t_z:Number = bDisplayFocus ? selectedZ : normalZ;
                if (activeTween) { activeTween.paused = true; }
                activeTween = new Tween( 500, this, { z:t_z }, { ease:Strong.easeOut } );
                */
                
                // NFM: Added as fix for case where last renderer is selected, move focus out, return focus to 2nd to last
                //      renderer, press down to select last renderer again -> last renderer plays "up" rather than "over.
                if (_focused == 0) { setState("out"); }
                else { setState("over"); }
            }
            
            // If focus is moved on keyboard press, the button needs to reset since it will not recieve a key up event.
            if (_pressedByKeyboard && !_focused) {
                setState("kb_release");
                _pressedByKeyboard = false;
            }
        }
    }
}