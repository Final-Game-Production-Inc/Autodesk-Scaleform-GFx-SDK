/**************************************************************************

Filename    :   GameEvent.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.events 
{
    
    import flash.events.Event;
    
    public class GameEvent extends Event {
        
    // Constants:d
        public static const INIT_GAME:String = "initGame";
        public static const PAUSE_GAME:String = "systemPauseGame";
        public static const RESUME_GAME:String = "systemResumeGame";
        
        public static const LAUNCH_MISSION:String = "gameLaunchMission";
        public static const OPEN_MAIN_MENU:String = "gameOpenMainMenu";
        
        public static const SHOW_MENU_TUTORIAL:String = "showMenuTutorial";
        public static const SHOW_GAME_TUTORIAL:String = "showGameTutorial";
        public static const SHOW_ABOUT_DIALOG:String = "showAboutDialog";
        
    // Public Properties:
        public var galaxyId:int = 0; // The galaxy that is currently selected.
        public var missionId:int = 0; // The mission that is currently selected.

        public var tutorialCallback:Function = null;
        public var delayedTutorial:Boolean = false;
        
    // Protected Properties:
        
    // Initialization:
        public function GameEvent( type:String, bubbles:Boolean = true, cancelable:Boolean = true,
                                   galaxyId:int = 0, missionId:int = 0, tutorialCallback:Function = null, 
                                   delayedTutorial:Boolean = false ) {
            super( type, bubbles, cancelable );
            
            this.galaxyId = galaxyId;
            this.missionId = missionId;
            this.tutorialCallback = tutorialCallback;
            this.delayedTutorial = delayedTutorial;
        }
        
    // Public getter / setters:
        
    // Public Methods:
        override public function clone():Event {
            return new GameEvent( type, bubbles, cancelable, galaxyId, missionId, tutorialCallback, delayedTutorial );
        }
        
        override public function toString():String {
            return formatToString( "GameEvent", "type", "bubbles", "cancelable" );
        }
        
    // Protected Methods:
        
    }
    
}