/**************************************************************************

Filename    :   EnemyEvent.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.events {
    
    import flash.events.Event;
    
    public class EnemyEvent extends Event {
        
    // Constants:
        public static const ENEMY_REACHED_DESTINATION:String = "enemyRDest";
        public static const ENEMY_DIED:String = "enemyDied";
        public static const UNIT_DIED:String = "unitDied";
        public static const REACHED_WP:String = "enemReachedWP";
        public static const STOPPED_MOVING:String = "enemStoppedMoving";
        
    // Public Properties:
        /** A reference to the related IListItemRenderer. */
        public var goldValue:Number = 0;
        
    // Protected Properties:
        
    // Initialization:
        public function EnemyEvent( type:String, bubbles:Boolean = true, cancelable:Boolean = true,
                                    goldValue:uint = 0 ) 
        {
            super(type, bubbles, cancelable);
            this.goldValue = goldValue;
            
        }
        
    // Public getter / setters:
        
    // Public Methods:
        override public function clone():Event {
            return new EnemyEvent(type, bubbles, cancelable, goldValue);
        }
        
        override public function toString():String {
            return formatToString("EnemyEvent", "type", "bubbles", "cancelable", "goldValue");
        }
        
    // Protected Methods:
        
    }
    
}