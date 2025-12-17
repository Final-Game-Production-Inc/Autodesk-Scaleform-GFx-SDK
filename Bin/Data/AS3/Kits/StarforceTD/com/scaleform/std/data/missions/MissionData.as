/**************************************************************************

Filename    :   MissionData.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.missions
{
    import flash.geom.Point;
    import com.scaleform.std.core.GameData;
    
    public class MissionData 
    {
        
    // Constants:
        
    // Protected Properties
        protected var _missionId:uint = 0;
        protected var _systemName:String = null;    
        protected var _numStars:Number = 0;
        protected var _maxStars:Number = 0;
        protected var _mapCoords:Point = null;
        protected var _description:String = null;
        
        // MissionInfo view data
        protected var _planetName:String = null;
        protected var _pathFrame:String = null;
        protected var _planetFrame:String = null;
        protected var _climate:String = null;
        protected var _temp:String = null;
        protected var _mass:String = null;
        
    // Initialization
        public function MissionData() {
            loadData();
        }
        
    // Public Functions:
        // Set
        public function setNumStars( numStars:uint ):void {
            _numStars = numStars;
        }
        
        // Get
        public function getIsAvailable():Boolean {
            if ( _missionId == GameData.MISSION_0_0 ) { 
                return true;
            }
            else {
                return ( GameData.getMissionNumStars( _missionId - 1 ) > 0 );    
            }
        }
        
        public function getSystemName():String {
           return _systemName;
        }
        public function getDescription():String {
           return _description;
        }
        public function getNumStars():Number {
            return _numStars;
        }
        public function getMaxStars():Number {
            return _maxStars;
        }
        public function getMapCoords():Point {
            return _mapCoords;
        }
        public function getPlanetName():String {
            return _planetName;
        }
        public function getPathFrame():String {
            return _pathFrame;
        }
        public function getPlanetFrame():String {
            return _planetFrame;
        }
        public function getClimateString():String {
            return _climate;
        }
        public function getTempString():String {
            return _temp;
        }
        public function getMassString():String {
            return _mass;
        }
        
    // Protected Functions
        protected function loadData():void {}
        
    }

}