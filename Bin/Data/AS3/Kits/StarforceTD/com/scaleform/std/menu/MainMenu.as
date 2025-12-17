/**************************************************************************

Filename    :   MainMenu.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package  com.scaleform.std.menu
{
	import com.scaleform.std.events.MainMenuEvent;
    import com.scaleform.std.events.MapEvent;
    import com.scaleform.std.system.SoundSystem;
    import com.scaleform.std.utils.Constants;
	import com.scaleform.std.utils.Utils;
	import flash.display.MovieClip;
	import flash.events.Event;
	import scaleform.clik.motion.Tween;
	import flash.display.StageScaleMode;
    import flash.display.Loader;
    import flash.net.URLRequest;
    
	import fl.transitions.easing.*;
    import flash.ui.Multitouch;
    import flash.ui.MultitouchInputMode;
    import flash.events.TouchEvent;
	import com.scaleform.std.events.LoadingEvent;
    import com.scaleform.std.events.GameEvent;
	
	public class MainMenu extends MovieClip
	{
		public var MapScreen:MainMenuView;
		public var Startup:Intro;
        
        protected var _logoScreenLoader:Loader = null;
		
		public function MainMenu() {
			super();
			
			addEventListener( Event.ADDED_TO_STAGE, configUI, false, 0, true );
		}
		
		public function configUI( e:Event ) {
            removeEventListener( Event.ADDED_TO_STAGE, configUI, false );
            
			stage.addEventListener(MainMenuEvent.LOGO_TO_INTRO_TRANSITION, handleLogoToIntro, false, 0, true);
			stage.addEventListener(MainMenuEvent.INTRO_TO_MAP_TRANSITION, handleIntroToMap, false, 0, true);
			stage.addEventListener(MainMenuEvent.INTRO_TO_MAP_COMPLETE, handleIntroToMapComplete, false, 0, true);
			stage.addEventListener(MainMenuEvent.MAP_TO_INTRO_TRANSITION, handleMapToIntro, false, 0, true);			
			
			// @TODO: This can all be handled by the primary Starforce Battlement manager.
            Multitouch.inputMode = MultitouchInputMode.TOUCH_POINT;
            stage.scaleMode = StageScaleMode.NO_BORDER;
			
			if (Utils.showIntro == true && Constants.isDebugMode == false) {
                MapScreen.visible = false;                
                
                _logoScreenLoader = new Loader();
                _logoScreenLoader.load( new URLRequest( "gameware_logo.swf" ) );
                _logoScreenLoader.addEventListener(Event.COMPLETE, handleLogoEnd, false, 0, true);                
				stage.addChild( _logoScreenLoader );
                
			} else {
				MapScreen.open();
			}
			
			stage.dispatchEvent( new LoadingEvent( LoadingEvent.LOAD_COMPLETE ) );
		}        
                
        protected function handleLogoEnd(e:Event):void {
            stage.removeChild(_logoScreenLoader);
            _logoScreenLoader = null;
            
                        
            Startup.open();
        }
        
		protected function handleLogoToIntro(e:MainMenuEvent):void {
            
            
			Startup.open();
		}
		
		protected function handleIntroToMap(e:MainMenuEvent):void {

            SoundSystem.loadSound("tutorial_starMap_001.mp3");
            SoundSystem.loadSound("tutorial_starMap_002.mp3");
            SoundSystem.loadSound("tutorial_starMap_003.mp3");
            SoundSystem.loadSound("tutorial_starMap_004.mp3");
            SoundSystem.loadSound("tutorial_starMap_005.mp3");
            SoundSystem.loadSound("tutorial_starMap_006.mp3");
            
            this.alpha = 0;
            var t:Tween = new Tween( 500, this, { alpha: 1 }, { ease:Strong.easeIn, onComplete: onFadeInEnd } );
            //SoundSystem.fadeInBgMusic(500);
            SoundSystem.restoreBgMusic();
            MapScreen.visible = true;
			MapScreen.open();
            
		}
        
        protected function onFadeInEnd( t:Tween ) {
            dispatchEvent(new MainMenuEvent(MainMenuEvent.INTRO_TO_MAP_COMPLETE, true, true));
        }
        
		protected function handleIntroToMapComplete(e:MainMenuEvent):void {
            
			Utils.showIntro = false;
		}
		
		protected function handleMapToIntro(e:MainMenuEvent):void {
            stage.dispatchEvent(new MapEvent(MapEvent.MISSION_INFO_CLOSED));
            stage.dispatchEvent(new MapEvent(MapEvent.RETURN_TO_GALAX_MAP));
            
			MapScreen.visible = false;
            //MapScreen.
			Startup.open();
		}
		
	}

}