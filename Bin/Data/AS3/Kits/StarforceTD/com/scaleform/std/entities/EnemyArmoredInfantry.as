/**************************************************************************

Filename    :   EnemyArmoredInfantry.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.entities
{
    import com.scaleform.std.system.SoundSystem;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.geom.Point;

    import com.scaleform.std.levels.Level;
	import com.scaleform.std.fx.FxDeathHuman;
	import com.scaleform.std.utils.Utils;
    
    import scaleform.clik.motion.Tween;
    
    public class EnemyArmoredInfantry extends Enemy
    {
        public function EnemyArmoredInfantry( level:Level, pathIdx:uint, laneIdx:uint ) {
            super( level, pathIdx, laneIdx );
        }
        
        override protected function configStats():void {
            // Damage
            minDamage = 22 * ( 1 + _statMod );
            
            // Speed
            speed = 40;
            maxSpeed = 40;
            
            // Health
            health = 200 * ( 1 + _statMod );
            maxHealth = 200 * ( 1 + _statMod );
            
            // Gold
            goldValue = 15;
			
			// Attach Speed
			attackSpeed = 1.0;
            
            playerDamage = 5;
        }
		
        override protected function playDeathAnimation():void {
            var fxDeath:FxDeathHuman = new FxDeathHuman();
            fxDeath.x = x;
            fxDeath.y = y;
            _level.addToBottomLayer( fxDeath );
			
			var s:Number = Utils.generateRandomBetween(0, 3);
			var sound:String = "DeathInfantry1.mp3";
			switch (s) {
				case 0: sound = "DeathInfantry2.mp3"; break;
				case 1: sound = "DeathInfantry3.mp3"; break;
			}
            
            SoundSystem.playSound(SoundSystem.SOUND_FX, sound);
        }	
		
        override protected function attack( e:MoveEntity ):void {
			super.attack(e);
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "Infantry_Attack.mp3");
        }		
        
    }

}