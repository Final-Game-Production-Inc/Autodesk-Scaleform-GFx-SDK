/**************************************************************************

Filename    :   GalaxyOneData.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.galaxies
{
    import com.scaleform.std.core.GameData;
    
    import flash.geom.Point;
    
    public class GalaxyOneData extends GalaxyData
    {
        
    // Constants:
        
    // Protected Properties
        
    // Initialization
        public function GalaxyOneData() {
            // 
        }
        
    // Public Functions:
        
    // Protected Functions
        override protected function loadData():void {
            _name = "Erebos Nebula";
            _mapCoords = new Point( -330, -100 );
            
            _nebulaFrame = 3;
            _missionIds = [ GameData.MISSION_1_0, GameData.MISSION_1_1 ];   
            
            _numStars = 0;
            _maxStars = _missionIds.length * 3;
        }
    }

}