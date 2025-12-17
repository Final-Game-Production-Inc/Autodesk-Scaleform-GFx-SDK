/**************************************************************************

Filename    :   SelectionInfo.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.hud
{
    import flash.display.MovieClip;
    import com.scaleform.std.core.GameData;

    public class SelectionInfo extends MovieClip
    {
        public static const LABEL_INFANTRY_ICON:String  = "inf";
        public static const LABEL_MECH_ICON:String      = "mech";
        public static const LABEL_TANK_ICON:String      = "tank";
        public static const LABEL_OST_ICON:String       = "ost";
        
        public function SelectionInfo() {
            super();
        }
        
        public function setData( type:uint, grade:uint, minDamage:Number, maxDamage:Number, attackSpeed:Number, effectType:uint, effectValue:Number ) {
            setIcon( type, grade );
        }

        public function setIcon( type:uint, grade:uint ):void {
            var frameName:String;
            
            switch ( type ) {
                case GameData.TOWER_TYPE_APC:
                    frameName = LABEL_INFANTRY_ICON;
                    break;
                case GameData.TOWER_TYPE_MECH:
                    frameName = LABEL_MECH_ICON;
                    break;
                case GameData.TOWER_TYPE_TANK:
                    frameName = LABEL_TANK_ICON;
                    break;
                case GameData.TOWER_TYPE_OST:
                    frameName = LABEL_OST_ICON;
                    break;
                default:
                    throw( new Error("Error :: SelectionInfo, Tower class associated with type " + type + " not found!") );
                    break;
            }
            
        }
    }

}