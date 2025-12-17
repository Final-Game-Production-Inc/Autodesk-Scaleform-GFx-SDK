/**************************************************************************

Filename    :   GalaxyTwoData.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.galaxies
{
    import com.scaleform.std.core.GameData;
    
    import flash.geom.Point;
    
    public class GalaxyTwoData extends GalaxyData
    {
        
    // Constants:
        
    // Protected Properties
        
    // Initialization
        public function GalaxyTwoData() {
            // 
        }
        
    // Public Functions:
        
    // Protected Functions
        override protected function loadData():void {
            _name = "Arcas Nebula";
            _mapCoords = new Point( -110, -180 );
            
            _nebulaFrame = 5;
            _missionIds = [ GameData.MISSION_2_0 ];   
            
            _numStars = 0;
            _maxStars = _missionIds.length * 3;
        }
    }

}