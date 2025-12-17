/**************************************************************************

Filename    :   PlayerAchievements.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data
{
    import flash.display.Stage;
    import flash.events.Event;
    import flash.utils.Dictionary;
    
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.events.AchievementEvent;
    import com.scaleform.std.system.SaveSystem;
    
    import com.scaleform.std.data.achievements.AchieveData;
    import com.scaleform.std.data.achievements.Ach_Complete_MOne;
    import com.scaleform.std.data.achievements.Ach_Strategist;
    import com.scaleform.std.data.achievements.Ach_Tactician;
    
    public class PlayerAchievements 
    {
    // Static Constants
        public static const ACHV_FIRST_BLOOD:String = "firstBlood";
        
        public static const ACHV_COMPLETE_M1:String = "completeM1";
        public static const ACHV_COMPLETE_M2:String = "completeM2";
        public static const ACHV_COMPLETE_M3:String = "completeM3";
        public static const ACHV_COMPLETE_M4:String = "completeM4";
        public static const ACHV_COMPLETE_M5:String = "completeM5";
        public static const ACHV_COMPLETE_DEMO:String = "completeDemo";
        
        public static const ACHV_ALL_STARS:String = "allStars";
        public static const ACHV_FULL_TOWER_UPGRADE:String = "fullTowerUpgrade";
        public static const ACHV_NO_LIVES_LOST:String = "noLivesLost";
        
        public static const ACHV_M1_NO_MECH:String = "m1NoMech";
        public static const ACHV_M2_NO_TANK:String = "m2NoTank";
        
        public static const ACHV_ALL_ACHVS:String = "allAchievements";
        public static const ACHV_DEFEAT_BOSS:String = "defeatBoss";
        public static const ACHV_FULL_RUSH:String = "fullRush";
        
        public static const ACHV_STRATEGIST:String = "strategist";
        public static const ACHV_TACTICIAN:String = "tactician";
        
    // Protected Variables
        protected static var _achievements:Dictionary = null;
        protected static var _stage:Stage = null;
        
    // Initialization
        public function PlayerAchievements() {
            //
        }
        
        // If PlayerAchievements has a reference to the stage, it can dispatch an event rather if complete rather
        // than having it handled by every individual call to setAchievementProgress throughout the game. Not the
        // strongest design, but marginally easier for implementing many achievements and avoiding redundant code.
        public static function init( stage:Stage ):void {
            loadAchievementData();
            loadAchievementProgress();
            
            _stage = stage;
        }
        
    // Public Functions
        public static function setAchievementProgress( achievementID:String, progress:Number ):void {
            var ad:AchieveData = _achievements[ achievementID ];
            if ( ad.isComplete() ) { return; }
            else {
                var isComplete:Boolean = ad.setProgress( progress );
                if ( isComplete ) {
                    _stage.dispatchEvent( new AchievementEvent( AchievementEvent.EARNED, true, true, ad.getName(), 
                                                                ad.getDesc(), ad.getProgress(), ad.getMaxProgress(), 
                                                                ad.getValue(), ad.getIconPath() ) ); 
                }
            }
        }
        
        public static function getAchievementProgress( achievementID:String ):Number {
            var ad:AchieveData = _achievements[ achievementID ];
            return ( ad != null ) ? ad.getProgress() : 0;
        }
        
        public static function getAchievementComplete( achievementID:String ):Boolean {
            return _achievements[ achievementID ].isComplete();
        }
        
        public static function saveAchievementProgress():void {
            SaveSystem.saveAchievements( getFlatAchievements() );
        }
        
    // Protected Functions
        protected static function getFlatAchievements():Array {
            var sharedObjectArray:Array = [];
            var idx:uint = 0;
            for ( var achvKey:String in _achievements ) {
                sharedObjectArray[ idx++ ] = achvKey;
                sharedObjectArray[ idx++ ] = _achievements[ achvKey ].getProgress();
            }
            // trace("Saving Achievements... sharedObjectArray: " + sharedObjectArray);
            return sharedObjectArray;
        }
        
        protected static function loadAchievementData():void {
            _achievements = new Dictionary();
            _achievements[ ACHV_COMPLETE_M1 ] = new Ach_Complete_MOne();
            _achievements[ ACHV_STRATEGIST ] = new Ach_Strategist();
            _achievements[ ACHV_TACTICIAN ] = new Ach_Tactician();
        }
        
        protected static function loadAchievementProgress():void {
            var sharedObjectArray:Array = SaveSystem.loadAchievements();
            if ( sharedObjectArray != null ) {
                for ( var i:uint = 0; i < sharedObjectArray.length; i++ ) {
                    var achievementId:String = sharedObjectArray[i];
                    var ad:AchieveData = _achievements[ ad ] as AchieveData;
                    if ( ad != null ) {
                        ad.setProgress( sharedObjectArray[++i] );
                    }
                }
            }
        }
    }
}