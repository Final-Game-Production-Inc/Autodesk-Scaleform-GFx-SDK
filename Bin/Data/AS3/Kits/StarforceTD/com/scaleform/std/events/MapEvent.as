/**************************************************************************

Filename    :   MapEvent.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.events 
{
    
    import flash.events.Event;
    
    public class MapEvent extends Event {
        
    // Constants:
        public static const GALAXY_SELECTED:String = "galaxySelected";
        public static const MISSION_SELECTED:String = "missionSelected";
        public static const RETURN_TO_GALAX_MAP:String = "returnToGalaxyMap";
        public static const LAUNCH_MISSION:String = "launchMission";
        public static const RETURN_TO_MAIN_MENU:String = "returnToMainMenu";
        
        public static const GALAXY_TRANSITION_COMPLETE:String = "mapGTComplete";
        public static const MISSION_TRANISITION_COMPLETE:String = "mapMTComplete";
        
        public static const MISSION_INFO_CLOSED:String = "missionInfoClosed";
        
    // Public Properties:
        public var galaxyId:int = 0; // The galaxy that is currently selected.
        public var missionId:int = -1; // The mission that is currently selected.
        
    // Protected Properties:
        
    // Initialization:
        public function MapEvent(   type:String, bubbles:Boolean = true, cancelable:Boolean = true, 
                                    galaxyId:int = 0, missionId:int = 0) 
        {
            super(type, bubbles, cancelable);
            
            this.galaxyId = galaxyId;
            this.missionId = missionId;
        }
        
    // Public Methods:
        override public function clone():Event {
            return new MapEvent(type, bubbles, cancelable, galaxyId, missionId );
        }
        
        override public function toString():String {
            return formatToString("WaveEvent", "type", "bubbles", "cancelable");
        }
        
    // Protected Methods:
        
    }
    
}