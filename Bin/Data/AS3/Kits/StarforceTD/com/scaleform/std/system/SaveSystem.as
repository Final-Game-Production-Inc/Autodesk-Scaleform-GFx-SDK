/**************************************************************************

Filename    :   SaveSystem.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.system
{
    import com.scaleform.std.core.GameData;
    import flash.net.SharedObject;
    
    import com.scaleform.std.data.PlayerData;
    import com.scaleform.std.data.PlayerAchievements;
    
    public class SaveSystem 
    {
        
        protected static const GALAXY_PROGRESS_SO_PROP:String = "galaxyProgress";        
        protected static const ACHIEVEMENT_SO_PROP:String = "achievementProgress";
        protected static const TECH_TREE_SO_PROP:String = "techTree";
        
        protected static const SHOW_MENU_TUTORIAL_SO_PROP:String = "showMenuTutorial";
        protected static const SHOW_GAME_TUTORIAL_SO_PROP:String = "showGameTutorial";
        
        public static var galaxyProgress:Array = null;
        
        protected static var _playerDataSO:SharedObject = null;
        protected static var _achievementsSO:SharedObject = null;
        protected static var _techTreeSO:SharedObject = null;
        
        public function SaveSystem() {
            //
        }
        
        public static function save( pd:PlayerData ):void {
            // Write pd into SharedObject.
            _playerDataSO = SharedObject.getLocal("sb_player_data");
            galaxyProgress = pd.getGalaxyProgress();
            _playerDataSO.data[ GALAXY_PROGRESS_SO_PROP ] = galaxyProgress;
            _playerDataSO.data[ SHOW_MENU_TUTORIAL_SO_PROP ] = GameData.getShowMenuTutorial();
            _playerDataSO.data[ SHOW_GAME_TUTORIAL_SO_PROP ] = GameData.getShowGameTutorial();
            _playerDataSO.flush();
        }
        
        public static function load( pd:PlayerData ):void {
            _playerDataSO = SharedObject.getLocal("sb_player_data");
            galaxyProgress = _playerDataSO.data[ GALAXY_PROGRESS_SO_PROP ];
            pd.setGalaxyProgress( galaxyProgress );            
            GameData.setShowMenuTutorial( _playerDataSO.data[ SHOW_MENU_TUTORIAL_SO_PROP ]==null?true:_playerDataSO.data[ SHOW_MENU_TUTORIAL_SO_PROP ], false );
            GameData.setShowGameTutorial( _playerDataSO.data[ SHOW_GAME_TUTORIAL_SO_PROP ]==null?true:_playerDataSO.data[ SHOW_GAME_TUTORIAL_SO_PROP ], false );
        }
        
        public static function loadAchievements():Array {
            _achievementsSO = SharedObject.getLocal("sb_achievements");
            return _achievementsSO.data[ ACHIEVEMENT_SO_PROP ];
        }
        public static function saveAchievements( flatAchieveArray:Array ):void {
            _achievementsSO.data[ ACHIEVEMENT_SO_PROP ] = flatAchieveArray;
            _achievementsSO.flush();
        }
        
        public static function loadTechTree():Array {
            _techTreeSO = SharedObject.getLocal("sb_tech_tree");
            return _techTreeSO.data[ TECH_TREE_SO_PROP ];
        }
        public static function saveTechTree( flatTechTreeArray:Array ):void {
            _techTreeSO.data[ TECH_TREE_SO_PROP ] = flatTechTreeArray;
            _techTreeSO.flush();
        }
    }
}