/**************************************************************************

Filename    :   TowerStateDisplay.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.hud {
    
    import flash.display.DisplayObject;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.events.TouchEvent;
    import flash.text.TextField;
    
    import scaleform.gfx.FocusEventEx;
    import scaleform.gfx.MouseEventEx;
    
    import com.scaleform.std.utils.Constants;
    import com.scaleform.std.controls.SBButton;
    import com.scaleform.std.core.GameData;
    
    public class TowerStatDisplay extends MovieClip {
        
    // Constants:
        public static const TRANK_FRAME_RANK1:String = "rank1";
        public static const TRANK_FRAME_RANK2:String = "rank2";
        public static const TRANK_FRAME_RANK3:String = "rank3";
        
        public static const TICON_FRAME_APC:String = "apc";
        public static const TICON_FRAME_TANK:String = "tank";
        public static const TICON_FRAME_MECH:String = "mech";
        public static const TICON_FRAME_OST:String = "ost";
        
    // Public Properties:
        
    // Protected Properties:
        
    // UI Elements:
        public var Damage:TextField;
        public var Speed:TextField;
        public var Range:TextField;
        public var Special:TextField;
        
        public var TowerRank:MovieClip;
        public var TowerIcon:MovieClip;
        public var TowerIconRank:MovieClip; // Grade-based Icon.

    // Initialization:
        public function TowerStatDisplay() {
            super();
        }
        
    // Public Getter / Setters:
        public function setData( towerType:Number, towerGrade:Number, dmgString:String, spdString:String, rngString:String, spcString:String ):void {
            var typeFrame:String;
            switch ( towerType ) {
                case GameData.TOWER_TYPE_APC:
                    typeFrame = TICON_FRAME_APC;
                    break;
                case GameData.TOWER_TYPE_MECH:
                    typeFrame = TICON_FRAME_MECH;
                    break;
                case GameData.TOWER_TYPE_TANK:
                    typeFrame = TICON_FRAME_TANK;
                    break;
                case GameData.TOWER_TYPE_OST:
                    typeFrame = TICON_FRAME_OST;
                    break;
                default:
                    break;
            }
            TowerIcon.gotoAndPlay( typeFrame );
			TowerIconRank = TowerIcon.TowerTankIcon;
            
            var gradeFrame:String = TRANK_FRAME_RANK1;
            if ( towerGrade == 1 ) { gradeFrame = TRANK_FRAME_RANK2; }
            else if ( towerGrade == 2 ) { gradeFrame = TRANK_FRAME_RANK3; }
            TowerRank.gotoAndPlay( gradeFrame );
            TowerIconRank.gotoAndPlay( gradeFrame );
            
            Damage.text = dmgString;
            Speed.text = spdString;
            Range.text = rngString;
            Special.text = spcString;
        }

    // Public Functions
        /** @exclude */
        override public function toString():String { 
            return "[SB MapLocation" + name + "]";
        }
        
    // Protected Methods:
       
    }
}