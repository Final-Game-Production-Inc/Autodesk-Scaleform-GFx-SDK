/**************************************************************************

Filename    :   LevelEvent.as

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
    
    public class LevelEvent extends Event {
        
    // Constants:
        public static const VICTORY:String      = "levelVictory";
        public static const DEFEAT:String       = "levelDefeat";
        public static const GOLD_CHANGE:String  = "levelGoldChange";
        public static const WAVE_CHANGE:String  = "levelWavesChange";
        public static const LIVES_CHANGE:String = "levelLivesChange";
        public static const WAVE_ALERT:String   = "levelWaveAlert";
        
    // Public Properties:
        public var gold:Number = 0;
        public var lives:uint = 0;
        public var wave:uint = 0;
        public var stars:uint = 0;
        
    // Protected Properties:
        
    // Initialization:
        public function LevelEvent( type:String, bubbles:Boolean = true, cancelable:Boolean = true, 
                                    gold:Number = 0, lives:uint = 0, wave:uint = 0, stars:uint = 0 ) 
        {
            super(type, bubbles, cancelable);
            
            this.gold = gold;
            this.lives = lives;
            this.wave = wave;
            this.stars = stars;
        }
        
    // Public Methods:
        override public function clone():Event {
            return new LevelEvent(type, bubbles, cancelable);
        }
        
        override public function toString():String {
            return formatToString("LevelEvent", "type", "bubbles", "cancelable");
        }
        
    // Protected Methods:
        
    }
    
}