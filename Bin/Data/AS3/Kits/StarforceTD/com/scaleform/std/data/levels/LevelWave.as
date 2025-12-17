/**************************************************************************

Filename    :   LevelWave.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.levels
{
    public class LevelWave
    {
    // Constants:
    
    // Protected Properties
    public var data:Vector.<LevelWaveData>;
    public var startTime:Number;
    public var startBonus:uint;
        
    // Initialization
        public function LevelWave() {
            data = new Vector.<LevelWaveData>;
        }
        
        public function addWaveData(wd:LevelWaveData):void {
            data.push(wd);
        }
	}
}