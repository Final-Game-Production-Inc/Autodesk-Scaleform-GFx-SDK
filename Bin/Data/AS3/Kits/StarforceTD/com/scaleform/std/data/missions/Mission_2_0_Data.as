/**************************************************************************

Filename    :   Mission_2_0_Data.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.missions
{
    import flash.geom.Point;
    import com.scaleform.std.core.GameData;
    
    public class Mission_2_0_Data extends MissionData
    {
        
    // Constants:
        
    // Protected Properties
    
    // Initialization
        public function Mission_2_0_Data() {
            _missionId = GameData.MISSION_2_0;
            super();
        }
        
    // Public Functions:
        
    // Protected Functions
        override protected function loadData():void {
            _systemName = "Hyperion System";
            _mapCoords = new Point( -210, -60 );
            
            // Mission
            _description = (
							"Despite your victory on Metis the enemy has still somehow discovered Vulcan. " +
							"Luckily the volcanic surface and constant eruptions on the planet have slowed their deployment, " +
							"but there is little time. Our expeditionary forces have discoved an alien technology " +
							"that could change the entire course of the war for whoever controls it."
                           );
            
            // Planet Description
            _planetName = "Vulcan";
            _pathFrame = "level5";
            _planetFrame = "level5";
            
            _climate = "VOLCANIC";
            _temp = "64°C (147°F)";
            _mass = "1.137 EARTHS";
            
        }
        
    }

}