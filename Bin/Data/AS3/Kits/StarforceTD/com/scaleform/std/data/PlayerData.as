/**************************************************************************

Filename    :   PlayerData.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data
{
    
    import com.scaleform.std.core.GameData;
    
    public class PlayerData 
    {
        protected var _talentTree:PlayerTalents = null;
        protected var _galaxyProgress:Array = null;
        
        public function PlayerData() {
            //
        }
        
        public function updateProgress():void {
            if ( _galaxyProgress == null ) { 
                _galaxyProgress = [];
            }
            
            _galaxyProgress = [ 
                                "g0",
                                    GameData.MISSION_0_0,
                                        GameData.getMissionNumStars( GameData.MISSION_0_0 ),
                                "g1",
                                    GameData.MISSION_1_0,
                                        GameData.getMissionNumStars( GameData.MISSION_1_0 ),
                                "g2",
                                    GameData.MISSION_1_1,
                                        GameData.getMissionNumStars( GameData.MISSION_1_1 ),
                                "g3",
                                    GameData.MISSION_2_0,
                                        GameData.getMissionNumStars( GameData.MISSION_2_0 )
                              ];
        }
        
        public function getGalaxyProgress():Array {
            return _galaxyProgress;
        }
        public function setGalaxyProgress( arr:Array ):void {
            _galaxyProgress = arr;
        }
        
        public function getTalentTree():Array {
            return _talentTree.getFlatArray();
        }
        public function setTalentTree( arr:Array ):void {
            // _talentTree.loadFlatArray( arr );
        }
    }

}