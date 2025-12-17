/**************************************************************************

Filename    :   AbilityEvent.as

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
    
    public class AbilityEvent extends Event {
        
    // Constants:
        public static const ENABLED:String  = "abilityEnabled";
        public static const DISABLED:String = "abilityDisabled";
        public static const USED:String     = "abilityUsed";
        public static const READY:String    = "abilityReady";
        
    // Public Properties:
        public var abilityId:uint = 0;
        public var ax:Number = 0;
        public var ay:Number = 0;
        
    // Protected Properties:
        
    // Initialization:
        public function AbilityEvent(   type:String, bubbles:Boolean = true, cancelable:Boolean = true, 
                                        abilityId:uint = 0, ax:Number = 0, ay:Number = 0) {
            super(type, bubbles, cancelable);
            
            this.abilityId = abilityId;
            this.ax = ax;
            this.ay = ay;
        }
        
    // Public Methods:
        override public function clone():Event {
            return new AbilityEvent(type, bubbles, cancelable);
        }
        
        override public function toString():String {
            return formatToString("AbilityEvent", "type", "bubbles", "cancelable");
        }
        
    // Protected Methods:
        
    }
    
}