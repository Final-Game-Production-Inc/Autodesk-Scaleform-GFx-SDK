/**************************************************************************

Filename    :   Effect.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
 
package com.scaleform.std.effects
{
    public class Effect 
    {
        public var value:Number = 0;
        public var destroyTime:Number = 0;
        public var tickSpeed:Number = 0;
        public var nextTickTime:Number = 0;
        
        public function Effect( value:Number, destroyTime:Number, tickSpeed:Number = 0 ) {
            this.value = value;
            this.destroyTime = destroyTime;
            this.tickSpeed = tickSpeed;
        }
    }

}