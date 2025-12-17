/**************************************************************************

Filename    :   EnemyDrone.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.entities
{
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.geom.Point;

    import com.scaleform.std.levels.Level;
    
    import scaleform.clik.motion.Tween;
    import com.scaleform.std.fx.FxDeathVehicle

    import com.scaleform.std.system.SoundSystem;
    
    public class EnemyDrone extends Enemy
    {
        public function EnemyDrone( level:Level, pathIdx:uint, laneIdx:uint ) {
            super( level, pathIdx, laneIdx );
        }
        
        override protected function configStats():void {
            // Damage
            minDamage = 8 * ( 1 + _statMod );
            
            // Speed
            speed = 100;
            maxSpeed = 100;
            
            // Health
            health = 80 * ( 1 + _statMod );
            maxHealth = 80 * ( 1 + _statMod );
            
            // Gold
            goldValue = 15;
			
			// Attach Speed
			attackSpeed = 1.0;
            
            playerDamage = 3;
        }
        
        override protected function playDeathAnimation():void {
            var fxDeath:FxDeathVehicle = new FxDeathVehicle();
            fxDeath.x = x;
            fxDeath.y = y;
            fxDeath.scaleX = fxDeath.scaleY = .5;
            _level.addToBottomLayer( fxDeath );
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "DeathDrone.mp3");
        }
		
		override protected function attack( e:MoveEntity ):void {
			super.attack(e);

			SoundSystem.playSound(SoundSystem.SOUND_FX, "Infantry_Attack.mp3");
        }				
        
    }

}