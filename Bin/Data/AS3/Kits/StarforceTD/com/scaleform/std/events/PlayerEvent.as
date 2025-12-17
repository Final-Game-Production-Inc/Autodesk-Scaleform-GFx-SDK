/**************************************************************************

Filename    :   PlayerEvent.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.events 
{
        
    import flash.events.Event;
    
    import com.scaleform.std.entities.Entity;
    import com.scaleform.std.entities.Tower;
    import com.scaleform.std.entities.TowerBuildSite;
    
    public class PlayerEvent extends Event {
        
    // Constants:
        public static const GOLD_CHANGE:String = "goldChange";
        public static const WAVE_CHANGE:String = "wavesChange";
        public static const LIVES_CHANGE:String = "livesChange";
        public static const WAVE_ALERT:String = "waveAlert";
        public static const VICTORY:String = "victory";
        
    // Public Properties:
        public var gold:uint = 0;
        public var lives:uint = 0;
        public var wave:uint = 0;
        
    // Protected Properties:
        
    // Initialization:
        public function PlayerEvent( type:String, bubbles:Boolean = true, cancelable:Boolean = true, 
                                    gold:uint = 0, lives:uint = 0, wave:uint = 0 ) 
        {
            super(type, bubbles, cancelable);
            
            this.gold = gold;
            this.lives = lives;
            this.wave = wave;
        }
        
    // Public Methods:
        override public function clone():Event {
            return new PlayerEvent(type, bubbles, cancelable);
        }
        
        override public function toString():String {
            return formatToString("PlayerEvent", "type", "bubbles", "cancelable");
        }
        
    // Protected Methods:
        
    }
    
}