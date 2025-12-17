/**************************************************************************

Filename    :   DropshipInfantry.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.entities
{
    import flash.display.MovieClip;
    import flash.geom.Point;
    import flash.events.Event;
    
    import fl.motion.easing.*;
    
    import com.scaleform.std.levels.Level;
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.events.EnemyEvent;
    import com.scaleform.std.events.AttackEvent;
    import com.scaleform.std.core.GameData;
	import com.scaleform.std.data.towers.TowerAPCData;
    
    import scaleform.clik.motion.Tween;
    
    public class DropshipInfantry extends Infantry
    {
        
        public function DropshipInfantry( level:Level ) {
            trace(" DropshipInfatry :: ctor " );
            super( level );
            
            isAlive = false; // Wait until we're fully beamed in before taking care of this.
        }
        
        override protected function configStats():void {
            // @TODO: Replace for DropshipInfantry
            
            // Speed is irrelevant for DropshipInfantry since they won't move.s
            speed = 100; // Speed is movement speed, not attack speed. Attack speed is always .5 for Infantry.
			
			// Attack Speed
			attackSpeed = GameData.getAbilityAttackSpeed( GameData.ABILITY_DROPSHIP );
            
            range =     GameData.getAbilityRange( GameData.ABILITY_DROPSHIP );
            minDamage = GameData.getAbilityMinDamage( GameData.ABILITY_DROPSHIP );
            maxDamage = GameData.getAbilityMaxDamage( GameData.ABILITY_DROPSHIP );
            maxHealth = GameData.getAbilityHealth( GameData.ABILITY_DROPSHIP );
			
			regenRate = TowerAPCData.INFANTRY_REGEN_RATE;
        }
        
        override protected function update(lmode:uint):void {
			var cTime:Number = _level.levelTimer;
			
            if (lmode != Level.LMODE_MOVABLE) return;
            
            // trace( this + " :: update() ");
            // If we have a target...
            if ( _target != null ) {
                // Is the target still alive?
                if ( _target.health <= 0 ) { 
                    _target = null; // If our target is dead, it's no longer valid.
                }
                // Is the target shooting us? If not, is there another target we can stop?
                else if ( _target.getTarget() != this ) { 
                    _target = null;
                }
                // Is the target still in range?
                else {
                    var tdist:Number = Utils.calcDistanceEntity( this, _target );
                    if (tdist > range) { 
                        _target = null; 
                    }
                }
            }
            
            // If we don't have a target, try find one.
            if (_target == null) {
                _target = findTarget();
            }
            
            // If we now have a valid target, attack him.
            if (_target != null) {
                eState = STATE_ATTACK;
				_nextRegenTime = -1;
                updateDirection( _target.x, _target.y );
				if ( _nextAttackTime <= cTime ) {
					attack( _target );
                }				
            }
            else {
				var firstRegenDelay:Number = 0;
				if (eState != STATE_IDLE) {
					firstRegenDelay = 2;
					eState = STATE_IDLE;
				}
                if ( _nextRegenTime > 0 && _nextRegenTime < cTime) { 
                    health += regenRate;
                    _nextRegenTime = -1; // Set it so we don't end up here again until we have health < healthMax.
                }
				if ( _nextRegenTime == -1 && health < maxHealth ) {
					_nextRegenTime = cTime + _regenDelay + firstRegenDelay;
				}
            }
        }
        
        public function destroy():void {
            dispatchEvent( new EnemyEvent( EnemyEvent.UNIT_DIED, true, true ) );
        }
        
    }

}