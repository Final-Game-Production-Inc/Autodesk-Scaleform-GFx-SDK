/**************************************************************************

Filename    :   Mission_1_0_Data.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.missions
{
    import flash.geom.Point;
    import com.scaleform.std.core.GameData;
    
    public class Mission_1_0_Data extends MissionData
    {
        
    // Constants:
        
    // Protected Properties
    
    // Initialization
        public function Mission_1_0_Data() {
            _missionId = GameData.MISSION_1_0;
            super();
        }
        
    // Public Functions:
        
    // Protected Functions
        override protected function loadData():void {
            _systemName = "Athena System";
            _mapCoords = new Point( -70, 100 );
            
            
            // Mission
            _description = (
                            "The Outlier Forces have suffered enormous casualties after you defeated them on Aura. " +
							"But scouting reports are showing that this was only the first wave of a strategic attack plan. " +
							"Their next target is the UEA’s mining operation on Perses which serves as the key supplier of " +
							"Lotidium ore required for our hyperspace technology." 
                           );
            
            // Planet Description
            _planetName = "Perses";
            _pathFrame = "level2";
            _planetFrame = "level2";
            
            _climate = "ARID";
            _temp = "50°C (122°F)";
            _mass = "0.547 EARTHS";
            
        }
        
    }

}