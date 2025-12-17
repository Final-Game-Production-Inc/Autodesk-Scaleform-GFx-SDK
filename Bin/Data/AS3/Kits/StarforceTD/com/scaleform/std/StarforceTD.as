/**************************************************************************

Filename    :   StarforceTD.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std 
{
    
    import com.scaleform.std.core.MenuGameState;
    import com.scaleform.std.menu.AboutWindow;
    import com.scaleform.std.tutorial.Tutorial;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.events.TouchEvent;
    import flash.display.StageScaleMode;
    import flash.display.Loader;
    import flash.net.URLRequest;
    import flash.ui.Multitouch;
    import flash.ui.MultitouchInputMode;
    import scaleform.gfx.SystemEx;
    
    import com.scaleform.std.core.GameState;
    import com.scaleform.std.core.LevelGameState;
    import com.scaleform.std.utils.Constants;
    import com.scaleform.std.events.MapEvent;
    import com.scaleform.std.events.LoadingEvent;
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.system.SoundSystem;
    import com.scaleform.std.data.PlayerAchievements;
    import com.scaleform.std.data.PlayerTechTree;
    
    import com.scaleform.std.events.GameEvent;
    
    import scaleform.gfx.Extensions;

    public class StarforceTD extends MovieClip 
    {
        
        protected var _gameState:GameState = null;
        protected var _loadScreenLoader:Loader = null;
        protected var _gameStateCreateFunc:Function = null;
        
        public function StarforceTD() {
            super();
            
            Extensions.enabled = true;
            
            SystemEx.setBackgroundAlpha(0);
            Extensions.noInvisibleAdvance = true;
            
            GameData.init();
            SoundSystem.init( stage );
            PlayerAchievements.init( stage );
            PlayerTechTree.init();
            Constants.initialize( stage ); // Set Constants.FRAME_DELTA based on FPS.
            Multitouch.inputMode = MultitouchInputMode.TOUCH_POINT;
            stage.scaleMode = StageScaleMode.NO_BORDER;
            
            addEventListener( Event.ADDED_TO_STAGE, configUI, false, 0, true );
        }
        
        public function configUI( e:Event ) {
            removeEventListener( Event.ADDED_TO_STAGE, configUI, false );
            
            trace("Multitouch.inputMode: " + Multitouch.inputMode );
            trace("isTouchEnabled: " + Constants.isTouchEnabled() );
            
            /*
            stage.addEventListener( TouchEvent.TOUCH_BEGIN, traceTouchInput, false, 0, true );
            stage.addEventListener( TouchEvent.TOUCH_MOVE, traceTouchInput, false, 0, true );
            
            stage.addEventListener( MouseEvent.MOUSE_DOWN, traceMouseInput, false, 0, true );
            stage.addEventListener( MouseEvent.MOUSE_MOVE, traceMouseInput, false, 0, true );
            */
            
            stage.addEventListener( GameEvent.LAUNCH_MISSION, handleLaunchMissionEvent, false, 0, true );
            stage.addEventListener( GameEvent.OPEN_MAIN_MENU, handleReturnToMainMenuEvent, false, 0, true );
            
            // stage.addEventListener( LoadingEvent.LOAD_START, handleLoadStart, false, 0, true );
            stage.addEventListener( LoadingEvent.LOAD_COMPLETE, handleLoadComplete, false, 0, true );
            
            stage.addEventListener( LoadingEvent.LOAD_OPEN_COMPLETE, handleLoadOpenComplete, false, 0, true );
            stage.addEventListener( LoadingEvent.LOAD_CLOSE_COMPLETE, handleLoadCloseComplete, false, 0, true );
            
            stage.addEventListener( GameEvent.SHOW_MENU_TUTORIAL, handleShowMenuTutorial, false, 0, true);
            stage.addEventListener( GameEvent.SHOW_GAME_TUTORIAL, handleShowGameTutorial, false, 0, true);
            stage.addEventListener( GameEvent.SHOW_ABOUT_DIALOG, handleShowAboutDialog, false, 0, true);
            
            _gameState = new MenuGameState( stage );
            //_gameState = new LevelGameState( stage );
			
            addEventListener( Event.ENTER_FRAME, tick, false, 0, true ); // Hack to not tick until the game is loaded.
        }
        
        protected function traceTouchInput( te:TouchEvent ):void {
            trace( "TOUCH: " + te );
        }
        protected function traceMouseInput( me:MouseEvent ):void {
            trace( "MOUSE: " + me );
        }
        
        protected function handleShowMenuTutorial(e:GameEvent):void {
            var t:Tutorial = new Tutorial(true, e.tutorialCallback, 6);
            addChild(t);
            t.open(e.delayedTutorial);
        }
        protected function handleShowGameTutorial(e:GameEvent):void {
            var t:Tutorial = new Tutorial(false, e.tutorialCallback, 9);
            addChild(t);
            t.open(e.delayedTutorial);
        }
        
        protected function handleShowAboutDialog(e:GameEvent):void {
            var a:AboutWindow = new AboutWindow;
            addChild(a);
            a.open();
        }
        
        protected function showLoadingScreen( animate:Boolean = true ):void {
            if ( _loadScreenLoader == null ) { 
                _loadScreenLoader = new Loader();
                _loadScreenLoader.load( new URLRequest( "LoadingView.swf" ) );
            }
            
            if ( animate ) {
                stage.addChild( _loadScreenLoader );    
            } else {
                _loadScreenLoader.contentLoaderInfo.addEventListener( Event.COMPLETE, handleLoadInit, false, 0, true );
            }
            
        }
        // @TODO: Add a Start State at the first launch. Showing the loading screen didn't work too well.
        // @TODO: Probably need a Splash.
        protected function handleLoadInit( e:Event):void {
            _loadScreenLoader.contentLoaderInfo.removeEventListener( Event.COMPLETE, handleLoadInit, false );
            _loadScreenLoader.content["Loading"].gotoAndPlay("on");
            stage.addChild( _loadScreenLoader );
            stage.dispatchEvent( new LoadingEvent( LoadingEvent.LOAD_COMPLETE, true, true ) );
        }
        protected function handleLoadComplete( le:LoadingEvent ):void {
            //
        }
        protected function handleLoadCloseComplete( le:LoadingEvent ):void {
            stage.removeChild( _loadScreenLoader );
            _loadScreenLoader.unloadAndStop( true );
            _loadScreenLoader = null; 
        }
        
        protected function handleLoadOpenComplete( le:LoadingEvent ):void {
            if ( _gameState != null ) { 
                _gameState.destroy();
            }
            if ( _gameStateCreateFunc != null ) {
                _gameStateCreateFunc();
                _gameStateCreateFunc = null;
            }
        }
        
        protected function handleLaunchMissionEvent( me:GameEvent ):void {
            showLoadingScreen(); 
            GameData.setCurrentGalaxyId( me.galaxyId );
            GameData.setCurrentMissionId( me.missionId );
                  
            
            _gameStateCreateFunc = loadLevel;
        }
        protected function handleReturnToMainMenuEvent( me:GameEvent ):void {
            showLoadingScreen(); 
            _gameStateCreateFunc = loadMenu;
        }
        
        protected function loadLevel():void {
            _gameState = new LevelGameState( stage );
        }
        protected function loadMenu():void {
            _gameState = new MenuGameState( stage );
        }
        
        public function tick( e:Event ):void {
            _gameState.tick();
        }
        
    }

}