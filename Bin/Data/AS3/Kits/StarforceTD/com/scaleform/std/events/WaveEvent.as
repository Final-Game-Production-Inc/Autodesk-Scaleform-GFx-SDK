/**************************************************************************

Filename    :   WaveEvent.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.events 
{
    
    import com.scaleform.std.data.levels.LevelWave;
    import flash.events.Event;
    import flash.geom.Point;
    
    public class WaveEvent extends Event {
        
    // Constants:
        public static const WAVE_ALERT:String = "waveAlert";
        public static const GROUP_ALERT:String = "groupAlert";
        public static const FORCE_WAVE:String = "forceWave";
        public static const FORCE_GROUP:String = "forceGroup";
        public static const WAVE_LAUNCH:String = "waveLaunch";
        public static const GROUP_LAUNCH:String = "groupLaunch";
        public static const WAVE_COUNTDOWN:String = "waveCountdown";
        
    // Public Properties:
        public var waveIdx:uint = 0; // The index of the wave this event referes to.
        public var timeTilWave:Number = 0; // The time until the wave begins.
        public var waveCoords:Point = null; // Coordinates where the next wave will begin.
        
        public var groupIdx:uint = 0; // The index of the group this event refers to.
        public var timeTilGroup:Number = 0; // The time until the group begins.
        public var groupCoords:Point = null; // Coordinates where the next group will beegin.
        
        public var levelWave:LevelWave = null; // Types of enemies that will be arriving this wave.
        public var direction:uint = 0; // The direction the wave is coming from. 0 - North, 3 -West.
        
    // Protected Properties:
        
    // Initialization:
        public function WaveEvent(  type:String, bubbles:Boolean = true, cancelable:Boolean = true, 
                                    waveIdx:uint = 0, timeTilWave:Number = 0, waveCoords:Point = null,
                                    groupIdx:uint = 0, timeTilGroup:Number = 0, groupCoords:Point = null,
                                    levelWave:LevelWave = null, direction:uint = 0 ) 
        {
            super(type, bubbles, cancelable);
            
            this.waveIdx = waveIdx;
            this.timeTilWave = timeTilWave;
            this.waveCoords = waveCoords;
            
            this.groupIdx = groupIdx;
            this.timeTilGroup = timeTilGroup;
            this.groupCoords = groupCoords;
            
            this.levelWave = levelWave;
            this.direction = direction;
        }
        
    // Public Methods:
        override public function clone():Event {
            return new WaveEvent(type, bubbles, cancelable, waveIdx, timeTilWave, waveCoords, groupIdx, timeTilGroup, groupCoords );
        }
        
        override public function toString():String {
            return formatToString("WaveEvent", "type", "bubbles", "cancelable");
        }
        
    // Protected Methods:
        
    }
    
}