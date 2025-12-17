/**************************************************************************

Filename    :   PlayerTalents.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data
{
    import com.scaleform.std.core.GameData;
    
    public class PlayerTalents 
    {
        
        protected var _talentsAPC:int = 0;
        protected var _talentsMech:int = 0;
        protected var _talentsTank:int = 0;
        protected var _talentsOST:int = 0;
        
        public function PlayerTalents() {
            //
        }
        
        public function learnTalent( towerType:uint, talentIdx:uint ):void {
            switch ( towerType ) {
                case GameData.TOWER_TYPE_APC:
                    _talentsAPC = talentIdx;
                    break;
                case GameData.TOWER_TYPE_MECH:
                    _talentsMech = talentIdx;
                    break;
                case GameData.TOWER_TYPE_OST:
                    _talentsOST = talentIdx;
                    break;
                case GameData.TOWER_TYPE_TANK:
                    _talentsTank = talentIdx;
                    break;
                default:
                    break;
            }
        }
        
        public function loadFlatArray( arr:Array ):void {
            if ( arr.length == 4 ) {
                _talentsAPC = arr[ GameData.TOWER_TYPE_APC ];
                _talentsMech = arr[ GameData.TOWER_TYPE_MECH ];
                _talentsTank = arr[ GameData.TOWER_TYPE_TANK ];
                _talentsOST = arr[ GameData.TOWER_TYPE_OST ];
            }
            else {
                throw new Error( "Error:: PlayerTalents.loadFlatArray(), Array length does not match number of available talents");
            }
        }
        
        public function getFlatArray():Array {
            return [ _talentsAPC, _talentsMech, _talentsTank, _talentsOST ];
        }
    }

}