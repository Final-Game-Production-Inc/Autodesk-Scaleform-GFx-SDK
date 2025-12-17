/**************************************************************************

Filename    :   Mission_0_0_Data.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.missions
{
    import flash.geom.Point;
    import com.scaleform.std.core.GameData;
    
    public class Mission_0_0_Data extends MissionData 
    {
        
    // Constants:
        
    // Protected Properties
    
    // Initialization
        public function Mission_0_0_Data() {
            _missionId = GameData.MISSION_0_0;
            super();
        }
        
    // Public Functions:
        
    // Protected Functions
        override protected function loadData():void {
            _systemName = "Ares System";
            _mapCoords = new Point( -250, 40 );
            
            // Mission
            _description =  
							"With the threat of Civil War looming over the galaxy, the Outlier Forces have been " +
							"getting bolder. Reports have just come in that the enemy is gathering in the Ares System. " +
							"They’re preparing for a siege on the United Earth Alliance (UEA) facility on Aura. " + 
							"Mobilize your forces quickly and use the scouting reports to plan your attack on the enemy."
            
            // Planet Description
            _planetName = "Aura";
            _pathFrame = "level1";
            _planetFrame = "level1";
            
            _climate = "TEMPERATE";
            _temp = "21°C (75°F)";
            _mass = "0.963 EARTHS";
        }
        
    }

}