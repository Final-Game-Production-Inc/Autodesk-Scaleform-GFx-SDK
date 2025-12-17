/**************************************************************************

Filename    :   LevelWaveData.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.data.levels
{
    public class LevelWaveData 
    {
    // Constants:
    
    // Protected Properties
        public var enemyType:uint = 0;
		public var spawnDelay:Number = 0;
        public var relSpawnTime:Number = 0;
        public var pathIdx:uint = 0;
        public var numEnemies:uint = 0;
        public var nextSpawnTime:Number = 0;
        
    // Initialization
        public function LevelWaveData( enemyType:uint, spawnDelay:Number, relSpawnTime:Number, pathIdx:uint, numEnemies:uint = 1 ) {
            this.enemyType = enemyType;
			this.spawnDelay = spawnDelay;
            this.relSpawnTime = relSpawnTime;
            this.pathIdx = pathIdx;
            this.numEnemies = numEnemies;
        }
	}
}