/**************************************************************************

Filename    :   CrazyLevel.as

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
    
    import com.scaleform.std.core.GameData;
    
    public class CrazyLevel extends Level
    {
        
        public function Level1() {
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
            
            priorityProperty = PRIORITIZE_NEG_X;
            gold = 450;
            
            _waveSourceDirection = 1;
            
            numWaves = 100;
            for ( var i:uint = 0; i < 100; i++ ) {
                enemyCount[i] = 20;
            }
            for ( var j:uint = 0; j < 100; j++ ) {
                var r:Number = j % 5;
                switch ( r ) {
                    case 0:
                        enemyTypes[j] = GameData.ENEMY_ARMORED_INFANTRY;
                        break;
                    case 1:
                        enemyTypes[j] = GameData.ENEMY_DRONE;
                        break;    
                    case 2:
                        enemyTypes[j] = GameData.ENEMY_MECH;
                        break;
                    case 3:
                        enemyTypes[j] = GameData.ENEMY_TANK;
                        break;
                    case 4:
                        enemyTypes[j] = GameData.ENEMY_INFANTRY;
                        break;
                }
            }
            
            for ( var k:uint = 0; k < 100; k++ ) {
                enemyStaggerDelay[ k ] = 1;
            }

        }*/
        
        override public function configLevel():void {
            super.configLevel();
        }
        
    }

}