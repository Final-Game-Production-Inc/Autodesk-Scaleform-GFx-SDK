/**************************************************************************

Filename    :   Intro.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.menu
{
	import com.scaleform.std.events.MainMenuEvent;
	import flash.display.MovieClip;
	import flash.events.TouchEvent;
	import flash.events.MouseEvent;
	import flash.events.Event;
	import scaleform.clik.motion.Tween;
	
	import fl.transitions.easing.*;
	
	import com.scaleform.std.utils.Constants;	
	import com.scaleform.std.system.SoundSystem;
	
	public class Intro extends MovieClip
	{
		public function Intro() {
			super();
		}
		
		public function open():void {
			visible = true;
			gotoAndPlay("on");
			
			// Start music
            SoundSystem.playSound(SoundSystem.SOUND_MUSIC, "BGM_TitleScreen.mp3", true);			
			
			//var eventType:String = Constants.isTouchEnabled() ? TouchEvent.TOUCH_BEGIN : MouseEvent.MOUSE_DOWN;
            //stage.addEventListener( eventType, handleIntroTap, false, 0, true );
			
			var t:Tween = new Tween( 1000, this, { alpha: 1 }, { ease:Strong.easeOut } );			
		}
		public function close():void {
			//var eventType:String = Constants.isTouchEnabled() ? TouchEvent.TOUCH_BEGIN : MouseEvent.MOUSE_DOWN;
            //stage.removeEventListener( eventType, handleIntroTap, false );
			SoundSystem.fadeOutBgMusic(800);
			var t:Tween = new Tween( 1000, this, { alpha: 0 }, { ease:Strong.easeOut, onComplete:onFadeOutEnd } );            
		}
		
        /*
		protected function handleIntroTap( e:Event ):void {
            close();			
		}*/
		
		protected function onFadeOutEnd( t:Tween ) {
			gotoAndStop("off");
			visible = false;
            
            dispatchEvent(new MainMenuEvent(MainMenuEvent.INTRO_TO_MAP_TRANSITION, true, true));
        }
		
	}

}