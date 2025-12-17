/**************************************************************************

Filename    :   AchievementEvent.as

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
    
    public class AchievementEvent extends Event {
        
    // Constants:
        public static const EARNED:String = "achEarned";
        public static const PROGRESS:String = "achProgress";
        
    // Public Properties:
        public var name:String = "";
        public var desc:String = "";
        public var progress:Number = 0;
        public var max:Number = 0;
        public var value:Number = 0;
        public var iconPath:String = "";
        
    // Protected Properties:
        
    // Initialization:
        public function AchievementEvent(   type:String, bubbles:Boolean = true, cancelable:Boolean = true, 
                                            name:String = "", desc:String = "", progress:Number = 0, 
                                            max:Number = 0, value:Number = 0, iconPath:String = "" ) 
        {
            super(type, bubbles, cancelable);
            
            this.name = name;
            this.desc = desc;
            this.progress = progress;
            this.max = max;
            this.iconPath = iconPath;
            this.value = value;
        }
        
    // Public Methods:
        override public function clone():Event {
            return new AchievementEvent(type, bubbles, cancelable, name, desc, progress, max, value);
        }
        
        override public function toString():String {
            return formatToString("AchievementEvent", "type", "bubbles", "cancelable", "name", "desc", "progress", "max", "value");
        }
        
    // Protected Methods:
        
    }
    
}