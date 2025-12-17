/**************************************************************************

Filename    :   TowerEvent.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.events {
    
    import com.scaleform.std.entities.Entity;
    import com.scaleform.std.entities.Tower;
    import com.scaleform.std.entities.TowerBuildSite;
    import flash.events.Event;
    
    public class TowerEvent extends Event {
        
    // Constants:
        public static const TOWER_BUILT:String = "towerBuilt";
        public static const TOWER_DESTROYED:String = "towerDestroyed";
        public static const TOWER_UPGRADED:String = "towerUpgraded";
        
    // Public Properties:
        public var tower:Tower = null;
        public var site:TowerBuildSite = null;
        
    // Protected Properties:
        
    // Initialization:
        public function TowerEvent( type:String, bubbles:Boolean = true, cancelable:Boolean = true,
                                    tower:Tower = null, site:TowerBuildSite = null ) 
        {
            super(type, bubbles, cancelable);
            this.tower = tower;
            this.site = site;
        }
        
    // Public getter / setters:
        
    // Public Methods:
        override public function clone():Event {
            return new TowerEvent(type, bubbles, cancelable);
        }
        
        override public function toString():String {
            return formatToString("TowerEvent", "type", "bubbles", "cancelable");
        }
        
    // Protected Methods:
        
    }
    
}