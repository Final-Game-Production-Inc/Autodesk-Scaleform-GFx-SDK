/**************************************************************************

Filename    :   MenuButton.as
Content     :  Custom implementation of a Button for the Menu Kit.
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
    
    import scaleform.clik.controls.Button;
    
    import scaleform.clik.motion.Tween;
    import scaleform.clik.ui.InputDetails;
    import scaleform.clik.constants.NavigationCode;
    
    public class MenuButton extends Button {
    // Constants:
    
    // Public Properties:
        /** The target ._z value when the component is selected. */
        public var selectedZ:Number = -300;
        /** The target ._z value when the component is not selected. */
        public var normalZ:Number = 0;
        
    // Protected Properties:
        protected var activeTween:Tween;
        
    // UI Elements:
        /** Reference to the small circular arrow MovieClip on the right side of the Button. */
        public var arrow:MovieClip;
    
    // Initialization:
        public function MenuButton() {
            enableInitCallback = true;
            constraintsDisabled = true;
            super();
        }

// Private Methods:
        override protected function configUI():void {
            super.configUI();
            
            // Raise the Button's arrow a bit in the Z for a 3D effect.
            if (arrow != null) { /* arrow.z = -450; */ }
        }
    
        /** Override for handleMouseRollOver that moves focus to this item. */
        override protected function handleMouseRollOver(event:MouseEvent):void {
            super.handleMouseRollOver(event);
            focused = 1; // Set focus to the Button.
        }
    
        override protected function changeFocus():void {
            if (!enabled) { return; }
            else {
                // _newFocusIndicatorFrame = (_focused > 0) ? "show" : "hide";
                // var t_duration:Number = (_focused > 0) ? 0.5 : 0.4;
                // var t_z:Number = (_focused > 0) ? selectedZ : normalZ;
                
                // if (activeTween) { activeTween.paused = 1; }
                // activeTween = new Tween(500, this, { z:t_z }, {ease:Strong.easeOut} );
                
                if (_focused == 0) { setState("out"); }
                else { setState("over"); }
                
                invalidateState();
                draw();
            }
            
            // If focus is moved on keyboard press, the button needs to reset since it will not recieve a key up event.
            if (_pressedByKeyboard && !_focused) {
                setState("kb_release");
                _pressedByKeyboard = false;
            }
            
            // Tween the Button up/down (3Di) based on whether it has focus.
            /*
            if (focused > 0) {
                // MovieClip(this).tweenTo(0.5, { _z:selectedZ }, Strong.easeOut);
            }
            else {
                // MovieClip(this).tweenTo(0.4, { _z:normalZ }, Strong.easeOut);
            }
            */
        }
    }
}
