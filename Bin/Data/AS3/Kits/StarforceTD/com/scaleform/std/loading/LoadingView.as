/**************************************************************************

Filename    :   LoadingView.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.loading 
{
    
    import flash.display.MovieClip;
    import flash.events.Event;
    
    import com.scaleform.std.events.LoadingEvent;
    
    import com.scaleform.std.system.SoundSystem;

    public class LoadingView extends MovieClip 
    {
        
        public var Loading:MovieClip;
        
        protected var _loadComplete:Boolean = false;
        protected var _loadOpenComplete:Boolean = false;
        
        public function LoadingView() {
            super();
            addEventListener( Event.ADDED_TO_STAGE, configUI, false, 0, true );
        }
        
        protected function configUI( e:Event ):void {
            removeEventListener( Event.ADDED_TO_STAGE, configUI, false );
            stage.addEventListener( LoadingEvent.LOAD_COMPLETE, handleLoadComplete, false, 0, true );
            stage.addEventListener( LoadingEvent.LOAD_OPEN_COMPLETE, handleLoadOpenComplete, false, 0, true );
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "LoadingTransitionOpen.mp3");
        }
        
        protected function handleLoadOpenComplete( le:LoadingEvent ):void {
            stage.removeEventListener( LoadingEvent.LOAD_OPEN_COMPLETE, handleLoadOpenComplete, false );
            _loadOpenComplete = true;
            if ( _loadComplete ) { closeLoadingScreen(); }
        }
        
        protected function handleLoadComplete( le:LoadingEvent ):void {
            stage.removeEventListener( LoadingEvent.LOAD_COMPLETE, handleLoadComplete, false );            
            _loadComplete = true;
            if ( _loadOpenComplete ) { closeLoadingScreen(); }
        }
        
        protected function closeLoadingScreen():void {
            stage.removeEventListener( LoadingEvent.LOAD_COMPLETE, handleLoadComplete, false );
            Loading.gotoAndPlay( "close" );
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "LoadingTransitionClose.mp3");
        }
        
    }

}