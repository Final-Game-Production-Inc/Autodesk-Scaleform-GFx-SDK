/**************************************************************************

Filename    :   Tutorial.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.tutorial 
{
    import flash.events.Event;
	import flash.display.MovieClip;
    import flash.utils.setTimeout;

    import com.scaleform.std.system.SoundSystem;
	
    public class Tutorial extends MovieClip 
    {
        public var TutorialWindow:MovieClip;
        public var TutorialGuy:MovieClip;
        
        protected var _showMenuTutorial:Boolean = true;
        protected var _closeCallback:Function = null;
        protected var _numFrames:uint = 0;
		
		private var currentAudioURL: String = "";
        
        public function Tutorial(showMenuTutorial:Boolean, closeCallback:Function, numFrames:uint) 
        {
            _showMenuTutorial = showMenuTutorial;
            _closeCallback = closeCallback;
            _numFrames = numFrames;
            
            addEventListener( Event.COMPLETE, notifyCloseEnd, false, 0, true );
            
            addEventListener( Event.ADDED_TO_STAGE, configUI, false, 0, true );
        }

        public function get showMenuTutorial():Boolean { return _showMenuTutorial; }
        public function get numFrames():uint { return _numFrames; }

        public function open(delayed:Boolean):void {
            gotoAndPlay(delayed ? "openDelayed" : "open");
            
            setTimeout(playOpenSound, 300 + (delayed ? 1000 : 0));
        }
        
        protected function playOpenSound():void {
            SoundSystem.playSound(SoundSystem.SOUND_FX, "PausePopupOpen.mp3");
        }
        
        public function notifyCloseStart():void {
			stopCurrentAudio();            
            
            SoundSystem.playSound(SoundSystem.SOUND_FX, "PausePopupClose.mp3");            
        }
        
        public function notifyCloseEnd(e:Event):void {
            removeEventListener( Event.COMPLETE, notifyCloseEnd, false );
            if (_closeCallback != null) {
                _closeCallback();
            }
            this.parent.removeChild(this);
        }
        
        protected function configUI( e:Event ) {
            removeEventListener( Event.ADDED_TO_STAGE, configUI, false );
        }
		
		public function stopCurrentAudio():void
		{
			(this as MovieClip).TutorialGuy.Mouth.gotoAndPlay("end");
			
            SoundSystem.stopSound(currentAudioURL);
		}
		
		public function playAudio(url: String):void
		{
			if (currentAudioURL != "")
				SoundSystem.stopSound(currentAudioURL);
			
			currentAudioURL = url;
            SoundSystem.playSound(SoundSystem.SOUND_FX, url, false, tutorialCallback);

		}
		
		public function tutorialCallback(e: Event):void
		{
			(this as MovieClip).TutorialGuy.Mouth.gotoAndPlay("end");
			
			e.currentTarget.removeEventListener(e.type, arguments.callee);
		}
        
    }

}