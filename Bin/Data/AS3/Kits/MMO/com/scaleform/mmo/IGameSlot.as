/**************************************************************************

Filename    :   IGameSlot.as

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.mmo {
       
    public interface IGameSlot {

        function highlight():void;
		function unhighlight():void;
		function updateCooldown(pct:Number):void;
		function endCooldown():void;
		
    }
}
