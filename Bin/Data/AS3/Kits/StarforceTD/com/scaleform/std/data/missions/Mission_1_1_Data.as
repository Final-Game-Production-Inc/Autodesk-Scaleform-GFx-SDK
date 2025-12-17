/**************************************************************************

Filename    :   Mission_1_1_Data.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.missions
{
    import flash.geom.Point;
    import com.scaleform.std.core.GameData;
    
    public class Mission_1_1_Data extends MissionData
    {
        
    // Constants:
        
    // Protected Properties
    
    // Initialization
        public function Mission_1_1_Data() {
            _missionId = GameData.MISSION_1_1;
            super();
        }
        
    // Public Functions:
        
    // Protected Functions
        override protected function loadData():void {
            _systemName = "Tethys System";
            _mapCoords = new Point( 270, -130 );
            
            // Mission
            _description = (
                            "There’s no time to enjoy your victory in the Athena System. Heavily-armored mercenaries " +
							"have already  surrounded our communications outpost on the ice planet of Metis in the " +
							"nearby Tethys System, in hopes of intercepting UEA transmissions. We cannot allow them to " +
							"discover our secret expeditionary forces on Vulcan."
                           );
            
            // Planet Description
            _planetName = "Metis";
            _pathFrame = "level3";
            _planetFrame = "level3";
            
            _climate = "ARCTIC";
            _temp = "-30°C (-22°F)";
            _mass = "1.281 EARTHS";
            
        }
        
    }

}