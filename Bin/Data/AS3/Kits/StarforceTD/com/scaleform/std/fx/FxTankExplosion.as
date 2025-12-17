/**************************************************************************

Filename    :   FxTankExplosion.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.fx 
{
    
    import com.scaleform.std.events.FxEvent;
    import flash.events.Event;
    import flash.display.MovieClip;
    
    public class FxTankExplosion extends FxBase 
    {
        
        public var fx:MovieClip;
        
        public function FxTankExplosion() {
            super();
            
            addEventListener( Event.ENTER_FRAME, tick, false, 0, true );
            addEventListener( Event.ADDED_TO_STAGE, handleAddedToStage, false, 0, true );
        }
        
        public function playFx():void {
            fx.gotoAndPlay( "on" );
        }
        
        public function handleAddedToStage( e:Event ):void {
            removeEventListener( Event.ADDED_TO_STAGE, handleAddedToStage, false );
            playFx();
        }
        
        override protected function tick( e:Event ):void {
            if (fx.currentFrame == fx.totalFrames) {
                removeEventListener( Event.ENTER_FRAME, tick, false );
                dispatchEvent( new FxEvent( FxEvent.COMPLETE ) );
            }
        }
        
    }

}