/**************************************************************************

Filename    :   Ability.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
 
package com.scaleform.std.abilities
{
    import flash.events.Event;
    import flash.display.MovieClip;
    import flash.geom.Point;
    
    import com.scaleform.std.levels.Level;
    import com.scaleform.std.core.GameData;
    
    public class Ability extends MovieClip
    {
    // Protected Properties:
        protected var _level:Level; // Reference to the level that created us.
        protected var _coords:Point = null; // The coordinates where the ability was launched.
        
        // Stats
        protected var _duration:Number = 0; // The duration of the ability.
        
        // Lifetime
        protected var _destroyTime:Number = 0; // The level.levelTimer when this Ability was created.
        
    // Initialization
        public function Ability( level:Level ) {
            super();
            _level = level;
            configStats();
        }
        
    // Public Functions
        public function start( ax:Number, ay:Number ):void {
            _coords = new Point( ax, ay );
            _destroyTime = _level.levelTimer + _duration;
        }
        
        public function end():void {
            dispatchEvent(new Event(Event.COMPLETE, true, false));
        }

        public function notifyPaused( value:Boolean ):void {}
        
        public function tick( lmode:uint ):void {}
        
        protected function configStats():void {}
    }
}