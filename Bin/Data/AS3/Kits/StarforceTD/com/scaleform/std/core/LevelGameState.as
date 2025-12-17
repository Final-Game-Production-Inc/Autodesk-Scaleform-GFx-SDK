/**************************************************************************

Filename    :   LevelGameState.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.core
{
    import flash.display.MovieClip;
    import flash.display.Stage;
    import flash.events.Event;
    
    import com.scaleform.std.core.GameState;
    import com.scaleform.std.levels.Level;
    import com.scaleform.std.levels.Level1;
    import com.scaleform.std.levels.Level2;
    import com.scaleform.std.levels.Level3;
	import com.scaleform.std.levels.Level4;
	import com.scaleform.std.levels.Level5;
    import com.scaleform.std.levels.TestLevel;
    import com.scaleform.std.events.GameEvent;
    import com.scaleform.std.system.SoundSystem;

    public class LevelGameState extends GameState
    {
        public var level:Level;
        
        public function LevelGameState( stage:Stage ) {
            super( stage );
        }
        
        override public function init():void {
            
            if ( level != null ) {
                _stage.removeChildAt( 0 );
            } 
            
            SoundSystem.loadSound("tutorial_gamePlay_001.mp3");
            SoundSystem.loadSound("tutorial_gamePlay_002.mp3");
            SoundSystem.loadSound("tutorial_gamePlay_003.mp3");
            SoundSystem.loadSound("tutorial_gamePlay_004.mp3");
            SoundSystem.loadSound("tutorial_gamePlay_005.mp3");
            SoundSystem.loadSound("tutorial_gamePlay_006.mp3");
            SoundSystem.loadSound("tutorial_gamePlay_007.mp3");
            SoundSystem.loadSound("tutorial_gamePlay_008.mp3");
            SoundSystem.loadSound("tutorial_gamePlay_009.mp3");
            
            // @TODO: Move Mission CLass to the Data and use GetDefinitonByName (or just use a Class ref) so GameData isn't linked to Levels.
            var missionId:uint = GameData.getCurrentMissionId();
            switch ( missionId ) {
                case GameData.MISSION_0_0:
                    level = new Level1();
                    break;
                case GameData.MISSION_1_0:
                    level = new Level2();
                    break;
                case GameData.MISSION_1_1:
                    level = new Level3();					
                    break;
				case GameData.MISSION_2_0:
					level = new Level5();
					break;
                default:
                    throw( new Error(" Error :: LevelGameState, MissionId does not match a Level class." ) );
                    break;
            }
            
            _stage.addChildAt( level, 0 );
            
            if (GameData.getShowGameTutorial()) {
                GameData.setShowGameTutorial(false);                
                level.dispatchEvent(new GameEvent(GameEvent.SHOW_GAME_TUTORIAL, true, false, 0, 0, null, true));
            }
        }
        
        override public function destroy():void {
            _stage.removeChild( level );
            level = null;
        }
        
        override public function tick():void {
            level.tick();
        }
        
    }

}