/**************************************************************************

Filename    :   AbilityButton.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.controls {
    
    import flash.display.DisplayObject;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.events.TouchEvent;
    import flash.text.TextField;
    
    import scaleform.gfx.FocusEventEx;
    import scaleform.gfx.MouseEventEx;
    
    import com.scaleform.std.utils.Constants;
    
    public class AbilityButton extends SBButton {
        
    // Constants:
        public static const ICON_STRIKE:String = "strike";
        public static const ICON_DROPSHIP:String = "merc";
        
    // Public Properties:
        
    // Protected Properties:
        protected var _timeSinceCooldown:Number = 0;
        protected var _cooldownFinishedTime:Number = 0;
        protected var _abilityReady:Boolean = true;
        protected var _lastPercComplete:Number = 0;
        protected var _paused:Boolean = false;
        
    // UI Elements:
        public var Cooldown:MovieClip;

    // Initialization:
        public function AbilityButton() {
            super();
            
        }
        
        public function get paused():Boolean { return _paused; }
        public function set paused(value:Boolean):void {
            _paused = value;
        }
        
        public function tick( e:Event ):void {
            if (_paused) return;
            if ( !_abilityReady ) { 
                _timeSinceCooldown += Constants.FRAME_DELTA;
                var percComplete:Number = _timeSinceCooldown / _cooldownFinishedTime;
                if ( percComplete != _lastPercComplete ) { 
                    if ( percComplete >= 1 ) { 
                        removeEventListener( Event.ENTER_FRAME, tick, false );
                        _abilityReady = true;
                        Cooldown.gotoAndPlay( "off" );
                    }
                    else {
                        var targetFrame:Number = Math.floor( percComplete * 300 );
                        Cooldown.gotoAndStop( targetFrame );
                    }
                }
            }
        }
        
    // Public Getter / Setters:
        public function setCooldownTime( time:Number ):void {
            _timeSinceCooldown = 0;
            _cooldownFinishedTime = time;
            _abilityReady = false;
            selected = false;
            addEventListener( Event.ENTER_FRAME, tick, false, 0, true );
        }

    // Public Functions
        /** @exclude */
        override public function toString():String { 
            return "[SB AbilityButton" + name + "]";
        }
        
    // Protected Methods:
       
    }
}