/**************************************************************************

Filename    :   EnemyTank.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.entities
{
    import com.scaleform.std.fx.FxDeathVehicle;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.geom.Point;

    import com.scaleform.std.levels.Level;
    
    import scaleform.clik.motion.Tween;
    import com.scaleform.std.fx.FxDeathVehicle;

	import com.scaleform.std.system.SoundSystem;
    
    public class EnemyTank extends Enemy
    {
        public function EnemyTank( level:Level, pathIdx:uint, laneIdx:uint ) {
            super( level, pathIdx, laneIdx );
            _healthBarOffsetY = -40;
        }
        
        override protected function configStats():void {
            // Damage
            minDamage = 75 * ( 1 + _statMod );
            
            // Speed
            speed = 25;
            maxSpeed = 25;
            
            // Health
            health = 2500 * ( 1 + _statMod );
            maxHealth = 2500 * ( 1 + _statMod );
            
            // Gold
            goldValue = 50;
			
			// Attach Speed
			attackSpeed = 2.0;
            
            playerDamage = 25;
        }
        
        override protected function playDeathAnimation():void {
            var fxDeath:FxDeathVehicle = new FxDeathVehicle();
            fxDeath.x = x;
            fxDeath.y = y;
            // fxDeath.scaleX = fxDeath.scaleY = .5;
            _level.addToBottomLayer( fxDeath );
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "DeathMechTank.mp3");
        }
        
		override protected function attack( e:MoveEntity ):void {
			super.attack(e);
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "Tank_Attack.mp3");
        }		
    }

}