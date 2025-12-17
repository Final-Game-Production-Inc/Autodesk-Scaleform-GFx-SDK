/**************************************************************************

Filename    :   SBButton.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.controls {
    
	import com.scaleform.std.system.SoundSystem;
    import flash.display.DisplayObject;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    
    import flash.events.TouchEvent;
    import flash.ui.Multitouch;
    import flash.ui.MultitouchInputMode;
    
    import scaleform.clik.controls.Button;
    import scaleform.clik.constants.InvalidationType;
    import scaleform.clik.constants.ControllerType;
    import scaleform.clik.constants.InputValue;
    import scaleform.clik.constants.NavigationCode;
    import scaleform.clik.core.UIComponent;
    import scaleform.clik.events.InputEvent;
    import scaleform.clik.events.ButtonEvent;
    import scaleform.clik.events.ComponentEvent;
    import scaleform.clik.ui.InputDetails;
    import scaleform.clik.utils.Constraints;
    import scaleform.clik.utils.ConstrainedElement;
    
    import scaleform.gfx.FocusEventEx;
    import scaleform.gfx.MouseEventEx;
    import scaleform.clik.constants.ConstrainMode;
    
    import com.scaleform.std.utils.Constants;
    
    public class SBButton extends Button {
        
    // Constants:
        
    // Public Properties:
	public var pressSound:String = "ButtonPress.mp3";
        
    // Protected Properties:
        protected var _configured:Boolean = false;
        protected var _touched:Boolean = false;
        
    // UI Elements:
        public var icon:MovieClip;

    // Initialization:
        public function SBButton() {
            constraintsDisabled = true;
            super();
        }
        
    // Public Getter / Setters:
        /** @exclude */
        override public function toString():String { 
            return "[SB SBButton " + name + "]";
        }
        
    // Protected Methods:
        override protected function configUI():void {
            if (_configured) { return; }
            _configured = true;
            tabEnabled = (_focusable && enabled && tabEnabled);
            mouseChildren = tabChildren = false;
            
            // None of the Buttons should keep focus.
            focusable = false;
            
            if ( Constants.isTouchEnabled() ) {
                addEventListener(TouchEvent.TOUCH_ROLL_OUT, handleRollOut, false, 0, true);
                addEventListener(TouchEvent.TOUCH_BEGIN, handleButtonPress, false, 0, true);
                addEventListener(TouchEvent.TOUCH_TAP, handleButtonRelease, false, 0, true);
            }
            else {
                addEventListener(MouseEvent.ROLL_OVER, handleRollOver, false, 0, true);
                addEventListener(MouseEvent.ROLL_OUT, handleRollOut, false, 0, true);
                addEventListener(MouseEvent.MOUSE_DOWN, handleButtonPress, false, 0, true);
                addEventListener(MouseEvent.CLICK, handleButtonRelease, false, 0, true);
                addEventListener(MouseEvent.DOUBLE_CLICK, handleButtonRelease, false, 0, true);
            }
            
            addEventListener(InputEvent.INPUT, handleInput, false, 0, true);
        }
        
        // OPTI -- If all of our entities are going to override this ridiculous class, need to minimize the aount of times this nonsense is called.
        // OPTI -- Maybe Entity should implement it's own implementation of Button rather than the full class.
        override protected function draw():void {
            // State is invalid, and has been set (is not the default)
            if (isInvalid(InvalidationType.STATE)) {
                if (_newFrame) {
                    gotoAndPlay(_newFrame);
                    _newFrame = null;
                }
                
                if (_newFocusIndicatorFrame) {
                    focusIndicator.gotoAndPlay(_newFocusIndicatorFrame);
                    _newFocusIndicatorFrame = null;
                }
                
                updateAfterStateChange();
                dispatchEvent(new ComponentEvent(ComponentEvent.STATE_CHANGE));
                // NFM: Should size be invalidated here by default? It can cause problems with timeline animations, 
                //      especially with focusIndictators that extend beyond the size of the original MovieClip. Instead, 
                //      perhaps let subclasses call invalidate(InvalidationType.SIZE) as necessary instead.
                invalidate(InvalidationType.DATA);
            }
            
            // Data is invalid when label or autoSize changes.
            if (isInvalid(InvalidationType.DATA)) {
                updateText();
            }
        }
        
        protected function handleInputReleaseOutside( event:Event ) :void {
            if ( enabled && _touched ) { 
                var mouseIdx:uint = 0;
                var btnIdx:uint = 0; 
                
                if (contains( event.target as DisplayObject) ) { return; }
                stage.removeEventListener( TouchEvent.TOUCH_END, handleReleaseOutside, false );
                
                _mouseDown |= 1 << mouseIdx;
                _touched = false;
                
                dispatchEvent( new ButtonEvent( ButtonEvent.RELEASE_OUTSIDE ) );
                setState( focusIndicator == null ? "release" : "kb_release" );
            }
        }
        
         override protected function changeFocus():void {
            // Nothing should have focus in this game.
        }
        
        protected function handleRollOver( event:Event ):void {
            if ( event is MouseEventEx || event is MouseEvent) {
                super.handleMouseRollOver( event as MouseEvent );
            }
            else if ( event is TouchEvent ) {
                //
            }
        }
        
        protected function handleRollOut( event:Event ):void {
			
            if ( event is MouseEventEx || event is MouseEvent) {
                super.handleMouseRollOut( event as MouseEvent );
            }
            else if ( event is TouchEvent ) {
                if ( enabled && _touched ) {
                    dispatchEvent( new ButtonEvent( ButtonEvent.DRAG_OUT ) );
                    
                    if (_focused || _displayFocus) {
                        setState(_focusIndicator == null ? "release" : "kb_release"); // The user has pressed and dragged out of the component. Do a down->up transition.
                    } else {
                        setState("out");
                    }
                }
            }
        }
        
        protected function handleButtonPress(event:Event):void {
            if ( event is MouseEventEx || event is MouseEvent ) {
                super.handleMousePress( event as MouseEvent );
            }
            else if ( event is TouchEvent ) {
                if ( enabled ) { 
                    var mouseIdx:uint = 0; // NFM: Multitouch based?
                    var btnIdx:uint = 0; 
                    
                    _mouseDown |= 1 << mouseIdx;
                    _touched = true;
					
                    setState("down");
                    var sfButtonEvent:ButtonEvent = new ButtonEvent(ButtonEvent.PRESS, true, false, mouseIdx, btnIdx, false, false);
                    dispatchEvent(sfButtonEvent);
                }
            }
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, pressSound);				
        }
        
        protected function handleButtonRelease(event:Event):void {
            if ( event is MouseEventEx || event is MouseEvent ) {
                super.handleMouseRelease( event as MouseEvent );
            }
            else if ( event is TouchEvent ) {
                if (enabled && _touched) {   
                    var mouseIdx:uint = 0; // NFM: Multitouch based?
                    var btnIdx:uint = 0; 
                    
                    _mouseDown ^= 1 << mouseIdx;
                    
                    setState( "out" );
                    handleClick( mouseIdx );
                    
                    var sfButtonEvent:ButtonEvent = new ButtonEvent(ButtonEvent.CLICK, true, false, mouseIdx, btnIdx, false, false);
                    dispatchEvent(sfButtonEvent);
                }
            }
        }
    }
}