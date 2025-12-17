/**************************************************************************

Filename    :   FxBase.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.fx 
{
    
    import flash.events.Event;
    import flash.display.MovieClip;
    
    import com.scaleform.std.events.FxEvent;
    
    public class FxBase extends MovieClip 
    {
        /** The layer (Fx or Bottom) that this Fx will be added to. Defined in FxEvent. */
        protected var _layerType:uint = 0;
        
        public function FxBase() {
            super();
            
            mouseEnabled = mouseChildren = false;
        }
        
        protected function tick( e:Event ):void { }
        override public function toString():String {
            return "[ FxBase " + name + " ]";
        }
        
    }

}