/**************************************************************************

Filename    :   GameState.as

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

    public class GameState 
    {
        public var _stage:Stage;
        
        public function GameState( stage:Stage ) {
            super();
            
            _stage = stage;
            init();
        }
        
        public function init():void {}
        public function destroy():void {}
        public function tick():void {}
        
    }

}