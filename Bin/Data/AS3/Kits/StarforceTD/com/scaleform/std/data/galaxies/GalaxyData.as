/**************************************************************************

Filename    :   GalaxyData.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.galaxies
{
    import com.scaleform.std.data.missions.MissionData;
    import flash.geom.Point;
    import flash.utils.Dictionary;
    
    import com.scaleform.std.core.GameData;
    
    public class GalaxyData 
    {
        
    // Constants:
        
    // Protected Properties
        protected var _name:String = null;    
        protected var _numStars:Number = 0;
        protected var _maxStars:Number = 0;
        protected var _mapCoords:Point = null;
        protected var _missionIds:Array = null;
        protected var _nebulaFrame:uint = 0;
        
    // Initialization
        public function GalaxyData() {
            loadData();
        }
        
    // Public Functions:
        public function getName():String {
           return _name;
        }
        public function getNumStars():Number {
            var totalStars:uint = 0;
            var numMissions:uint = _missionIds.length;
            for ( var i:uint = 0; i < numMissions; i++ ) {
                totalStars += GameData.getMissionNumStars( _missionIds[ i ] );
            }
            return totalStars;
        }
        public function getMaxStars():Number {
            return _missionIds.length * 3;
        }
        public function getMapCoords():Point {
            return _mapCoords;
        }
        public function getNumMissions():uint {
            return _missionIds.length;
        }
        public function getMissionIds():Array { 
            return _missionIds;
        }
        public function getNebulaFrame():uint {
            return _nebulaFrame;
        }
        
    // Protected Functions
        protected function loadData():void {}
        
    }

}