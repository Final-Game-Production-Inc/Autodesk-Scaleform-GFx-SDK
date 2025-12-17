/**************************************************************************

Filename    :   TestEnemy.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.test
{
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.geom.Point;

    import com.scaleform.std.levels.Level;
    import com.scaleform.std.entities.Enemy;
    
    import scaleform.clik.motion.Tween;
    
    public class TestEnemy extends Enemy
    {
        public function TestEnemy( level:Level, pathIdx:uint, laneIdx:uint ) {
            super( level, pathIdx, laneIdx );
        }
        
        override protected function configStats():void {
            // Damage
            minDamage = 10;
            
            // Speed
            speed = 50;
            maxSpeed = 100;
            
            // Health
            health = 10000;
            maxHealth = 10000;
        }
        
    }

}