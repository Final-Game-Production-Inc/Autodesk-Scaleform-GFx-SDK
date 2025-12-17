/**************************************************************************

Filename    :   FxAbilityStrikeScreen.as

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
    
    public class FxAbilityStrikeScreen extends FxBase 
    {
        
        public function FxAbilityStrikeScreen() {
            super();
            
            addEventListener( Event.ENTER_FRAME, tick, false, 0, true );
            gotoAndPlay( "on" );
        }
        
        override protected function tick( e:Event ):void {
            if (this.currentFrame == this.totalFrames) {
                removeEventListener( Event.ENTER_FRAME, tick, false );
                dispatchEvent( new FxEvent( FxEvent.COMPLETE ) );
            }
        }
        
    }

}