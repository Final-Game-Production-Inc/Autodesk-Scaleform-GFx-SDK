/**************************************************************************

Filename    :   TestLevel.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.levels
{
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.geom.Point;
    import flash.display.DisplayObject;
    
    import com.scaleform.std.entities.Tower;
    import com.scaleform.std.entities.Enemy;
    import com.scaleform.std.utils.PathMarker;
    
    import com.scaleform.std.entities.EnemyInfantry;
    import com.scaleform.std.entities.EnemyArmoredInfantry;
    import com.scaleform.std.entities.EnemyDrone;
    import com.scaleform.std.entities.EnemyMech;
    import com.scaleform.std.entities.EnemyTank;
    
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.data.levels.LevelWaveData;
    
    public class TestLevel extends Level
    {
        
        public function TestLevel() {
            super("conf/level1.xml");
        }
        
        override public function configUI( e:Event ):void {
            super.configUI( e );
        }
        
		/*
        override public function loadLevelData():void {
            super.loadLevelData();
            
            discoverPaths();
            discoverTowers();
            
			waveData = [
							[
							 	new LevelWaveData( GameData.ENEMY_TEST, 0, 0, 1 )
							]
						];
        }
		*/
        
        override protected function handleLevelComplete():void {
            super.handleLevelComplete();
            if ( _starScore >= 3 ) {
                // PlayerAchievements.
            }
        }		
		
        override public function configLevel():void {
            super.configLevel();
        }
        
    }

}