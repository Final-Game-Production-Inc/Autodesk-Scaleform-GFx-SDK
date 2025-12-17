/**************************************************************************

Filename    :   WaveEnemyIcon.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.hud
{
    import flash.display.MovieClip;
    import com.scaleform.std.core.GameData;
    import flash.text.TextField;

    public class WaveEnemyIcon extends MovieClip
    {
        public static const LABEL_INFANTRY_ICON:String          = "inf";
        public static const LABEL_ARMORED_INFANTRY_ICON:String  = "ainf";
        public static const LABEL_DRONE_ICON:String             = "drone";
        public static const LABEL_MECH_ICON:String              = "mech";
        public static const LABEL_TANK_ICON:String              = "tank";
        
        public var textField:TextField;
        
        public function WaveEnemyIcon() {
            super();
        }

        public function setValues( enemyType:Number, num:int ):void {
            trace(" WaveEnemyIcon :: setIcon( " + enemyType + ", " + num + " )");
            
            textField.text = num.toString();
            
            var frameName:String;
            
            switch ( enemyType ) {
                case GameData.ENEMY_INFANTRY:
				case GameData.ENEMY_TEST:
                    frameName = LABEL_INFANTRY_ICON;
                    break;
                case GameData.ENEMY_ARMORED_INFANTRY:
                    frameName = LABEL_ARMORED_INFANTRY_ICON;
                    break;
                case GameData.ENEMY_DRONE:
                    frameName = LABEL_DRONE_ICON;
                    break;
                case GameData.ENEMY_MECH:
                    frameName = LABEL_MECH_ICON;
                    break;
                case GameData.ENEMY_TANK:
                    frameName = LABEL_TANK_ICON;
                    break;
                default:
                    throw( new Error("Error :: WaveEnemyIcon, Enemy class associated with type " + enemyType + " not found!") );
                    break;
            }
            
            gotoAndStop( frameName );
        }
            
            
            
            
        
    }

}