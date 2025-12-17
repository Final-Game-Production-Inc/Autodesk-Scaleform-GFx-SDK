/**************************************************************************

Filename    :   Enemy.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.entities
{
	import com.scaleform.std.utils.Constants;
    import flash.geom.Point;
    import flash.events.Event;
    import flash.display.MovieClip;
    import flash.utils.Dictionary;
    
    import fl.motion.easing.*;
    
    import scaleform.clik.motion.Tween;
    
    import com.scaleform.std.levels.Level;
    import com.scaleform.std.events.EnemyEvent;
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.data.towers.TowerData;
    import com.scaleform.std.events.AttackEvent;
    import com.scaleform.std.fx.FxDeathHuman;
    import com.scaleform.std.fx.FxCombatText;
    
    import com.scaleform.std.effects.Effect;
    
    public class Enemy extends MoveEntity
    {
        // State constant defintions.
        public static const STATE_IDLE:uint     = 0;
        public static const STATE_ATTACK:uint   = 1;
        public static const STATE_RUN:uint      = 2;
		public static const STATE_REACHED_DESTINATION:uint = 3;
        
        // Labels for state keyframes of _modelState.
        public static const LABEL_STATE_IDLE:String     = "idle1";
        public static const LABEL_STATE_ATTACK:String   = "attack";
        public static const LABEL_STATE_RUN:String      = "run";
        
        public var pathIdx:uint = 0;
        public var laneIdx:uint = 0;
        
        protected var _statMod:Number = 0;
        
        protected var _laneCoords:Array = null;
        protected var _wpIdx:uint = 0; // The index of the next waypoint we're traveling to.
        
		protected var _nextAttackTime:Number = 0; // The next time we can attack.		
		
        protected var _effectUpdateDelay:Number = 1;
        protected var _nextEffectProcessTime:Number = 0;
        protected var _effectActive:Boolean = false;
        
        protected var _effectSlowQueue:Dictionary = null;
        protected var _effectSlowActive:Boolean = false;
        protected var _effectSlowValue:Number = 0;
        
        protected var _effectDOTQueue:Dictionary = null;
        protected var _effectDOTActive:Boolean = false;
        
        protected var _effectAmpDmgQueue:Dictionary = null;
        protected var _effectAmpDmgActive:Boolean = false;
        protected var _effectAmpValue:Number = 0;
        
        public var model:MovieClip;
        
        public function Enemy( level:Level, pathIdx:uint, laneIdx:uint, statMod:Number = 0 ) {
            super( level );
            
            _effectSlowQueue = new Dictionary();
            _effectDOTQueue = new Dictionary();
            _effectAmpDmgQueue = new Dictionary();
            
            _statMod = statMod;
            
            configStats();
            _laneCoords = _level.paths[ pathIdx ][ laneIdx ];
            
            var startCoord:Point = _laneCoords[0];
            x = startCoord.x;
            y = startCoord.y;
        }
        
        override protected function configUI():void {
            super.configUI();
            
            updateDestination();
            eState = STATE_RUN;
        }
        
        override protected function configStats():void {
            goldValue = 15;
        }
        
        public function notifyPaused( gamePaused:Boolean ):void {
            if ( gamePaused ) { 
                eState = STATE_IDLE;
            }
            else {
                if ( _target ) { 
                    eState = STATE_ATTACK;
                }
                else { 
                    eState = STATE_RUN;
                    updateDestination();
                }
            }
            updateGraphics();
        }
        
        public function handleDamage( dmg:Number, isCrit:Boolean ):void {
            health -= ( dmg * ( 1 + _effectAmpValue ) );
            
            //if (isCrit) {
            //    var combatText:FxCombatText = new FxCombatText( isCrit ? "CRIT" : String(dmg), "#aa0000" );
            //    combatText.x = this.x;
            //    combatText.y = this.y;
            //    _level.addToFXLayer( combatText );
            //}
            
            if ( health <= 0 ) {
                playDeathAnimation(); // Need to do this before we send out the UNIT_DIED event or the Level will move us off-stage.
                dispatchEvent( new EnemyEvent( EnemyEvent.ENEMY_DIED, true, true, goldValue ) );
                dispatchEvent( new EnemyEvent( EnemyEvent.UNIT_DIED, true, true ) );
            }
        }
        
        override public function handleAttackEvent( e:AttackEvent ):void {
            handleDamage( e.damage, e.isCrit);
            
            trace( this + " :: " + e.type + ", eType: " + e.effectType + ", eValue: " + e.effectValue );
             
            if ( health <= 0 ) { return; }
            // If we were attacked by a moving unit, we need to stop and fire back.
            else if ( e.type == AttackEvent.UNIT_ATTACK ) {
                // Only switch targets if we don't have an active target.
                if ( _target == null ) {
                    // Stop our movement.
                    if ( eState == STATE_RUN ) { 
                        speed = 0;
                        dispatchEvent( new EnemyEvent( EnemyEvent.STOPPED_MOVING, true, true ) );
                    }
                    
                    // Turn and attack the unit that attacked us.
                    eState = STATE_ATTACK; 
                    
                    _target = e.target as MoveEntity; // e.target is the attacker.
                    _target.addEventListener( EnemyEvent.UNIT_DIED, onTargetDeath, false, 0, true );
                    
                    updateGraphics();
                }
            }
            else if ( e.type == AttackEvent.TOWER_ATTACK ) {
               
                // If the attack has an meta ability with a value > 0...
                if ( e.effectValue != 0 ) {
                    var cTime:Number = _level.levelTimer;
                    var eType:uint = e.effectType;
                    var newEffect:Effect = new Effect( e.effectValue, e.effectDuration + cTime, e.effectTickSpeed );
                    switch ( eType ) {
                        case TowerData.EFFECT_SLOW:
                            _effectSlowQueue[ e.sourceId ] = newEffect;
                             processEffects( _level.levelTimer );
                            break;
                            
                        case TowerData.EFFECT_DOT:
                            _effectDOTQueue[ e.sourceId ] = newEffect;
                            _effectActive = true; // Ideally, doing it this way will give the DOT a very short delay.
                            break;
                            
                        case TowerData.EFFECT_AMP_DMG:
                            _effectAmpDmgQueue[ e.sourceId ] = newEffect;
                            processEffects( _level.levelTimer );
                            break;
                            
                        default:
                            break;
                    }
                   
                }
            }
        }
        
        override public function tick( lmode:uint ):void {
            var cTime:Number = _level.levelTimer;
            update(lmode);
            
            if (lmode != Level.LMODE_MOVABLE) return;
            
            if ( _effectActive ) { 
                processEffects( cTime );
            }
        }
        
        protected function processEffects( levelTime:Number ):void {
            trace(" >> " + this + " :: processEffects()");
            
            var e:Effect;
            
            // Slow
            var slowActiveCounter:int = 0;
            var maxSlow:Number = 0;
            for (var slowKey:Object in _effectSlowQueue) { 
                e = _effectSlowQueue[ slowKey ];
                if ( e == null ) { continue; }
                if ( e.destroyTime < levelTime ) { 
                    _effectSlowQueue[ slowKey ] = null;
                } else {
                    if ( e.value > maxSlow ) { 
                        maxSlow = e.value;
                    }
                    slowActiveCounter++;
                }
            }
            if ( slowActiveCounter > 0 )  { 
                speed = maxSpeed * ( 1 - maxSlow );
                _effectSlowValue = maxSlow;
                updateDestination();
            }
            else if ( speed != maxSpeed ) {
                speed = maxSpeed;
                updateDestination();
            }
            
            // Amplify Damage
            var ampActiveCounter:int = 0;
            var maxAmp:Number = 0;
            for (var ampKey:Object in _effectAmpDmgQueue) { 
                e = _effectAmpDmgQueue[ ampKey ];
                if ( e == null ) { continue; }
                if ( e.destroyTime < levelTime ) { 
                    _effectAmpDmgQueue[ ampKey ] = null;
                }
                else {
                    if ( e.value > maxAmp ) { 
                        maxAmp = e.value;
                    }
                    ampActiveCounter++;
                }
            }
            if ( ampActiveCounter > 0 ) { 
                _effectAmpValue = maxAmp;
            } else {
                _effectAmpValue = 0;
            }
            
            // DOT
            var dotActiveCounter:int = 0;
            var dotTotalDamage:Number = 0;
            for (var dotKey:Object in _effectDOTQueue) { 
                // If the DOT should be ticked, or should have been ticked by now, add it.
                e = _effectDOTQueue[ dotKey ];
                if ( e == null ) { continue; }
                if ( e.nextTickTime < levelTime ) { 
                    dotTotalDamage += e.value;
                    e.nextTickTime = levelTime + e.tickSpeed;
                }
                
                // Check if the DOT should be removed.
                if ( e.destroyTime < levelTime ) { 
                    _effectDOTQueue[ dotKey ] = null;
                }
                else
                {
                    dotActiveCounter++;
                }
                
            }
            if ( dotTotalDamage > 0 ) {
                handleDamage( dotTotalDamage, false );
            }
            
            _effectActive = dotActiveCounter > 0 || slowActiveCounter > 0 || ampActiveCounter > 0;
        }
        
        public function update(lmode:uint):void {
			var cTime:Number = _level.levelTimer;
			
			//trace("update");
            if ( _eState == STATE_ATTACK ) {
                
                if (lmode != Level.LMODE_MOVABLE) return;
                
				if ( _nextAttackTime <= cTime ) {				
					if ( _target != null ) { 
                        
						attack( _target );
						updateGraphics();
					} 
					else { 
						eState = STATE_RUN;
						updateDestination();
					}
				}
            }
			else if (eState == STATE_RUN) {
				updatePosition();
			}
        }
	
		protected function updatePosition():void {
			//trace("updatePosition: " + _dsteps + " (x,y): " + x + ", " + y);
			if (_dsteps > 0) {
				x = x + _dx;
				y = y + _dy;
			}
			_dsteps--;
			if (_dsteps <= 0) {
				reachedWaypoint();
			}
		}
		
        protected function attack( e:MoveEntity ):void {
			_nextAttackTime = _level.levelTimer + attackSpeed;
            dispatchEvent( new AttackEvent( AttackEvent.UNIT_ATTACK, true, true, _target, 0, minDamage, 0 ) );
        }
        
        protected function onTargetDeath( e:EnemyEvent ):void {
            if ( _target != null ) {
                _target.removeEventListener( EnemyEvent.UNIT_DIED, onTargetDeath, false);
                _target = null;
            }
            
            speed = maxSpeed;
            // updateGraphics();
        }
        
        protected function findNextWaypoint( nextWaypointIdx:uint ):Boolean {
            if ( nextWaypointIdx >= _laneCoords.length ) { return false; }
            _curDest = _laneCoords[ nextWaypointIdx ];
            
            return true;
        }
        
        override protected function updateDestination():void {
			//trace("updateDestination");
            if ( findNextWaypoint( _wpIdx ) ) {
				
				_curDest = _curDest.add(new Point(Utils.generateRandomBetween( -6, 6), 
												  Utils.generateRandomBetween( -6, 6)));
				
				var dist:Number = Utils.calcDistanceCoords( x, y, _curDest.x, _curDest.y );
				_dsteps = dist / (speed / Constants.SPEED_STEP);
				_dx = (_curDest.x - x) / _dsteps;
				_dy = (_curDest.y - y) / _dsteps;
				//trace("steps: " + _dsteps + ", (dx, dy): " + _dx + ", " + _dy);
				
                updateGraphics();
            }
            else {
				eState = STATE_REACHED_DESTINATION;

                dispatchEvent( new EnemyEvent( EnemyEvent.ENEMY_REACHED_DESTINATION, true, true ) );
            }
        }
        
        protected function reachedWaypoint( /*t:Tween*/ ):void {
            if ( isAlive ) { 
                _wpIdx++; // Increment the Waypoint Index so they seek the next one out.
                dispatchEvent( new EnemyEvent( EnemyEvent.REACHED_WP, true, true ) );
                updateDestination();
                updateGraphics();
            }
        }
        
        override protected function updateGraphics():void {
            var stateChanged:Boolean = false;
            var newFrame:String;
            
            if ( _gxState != _eState ) {
                _gxState = _eState;
                stateChanged = true;
                switch ( _eState ) {
                    case STATE_ATTACK: 
                        newFrame = LABEL_STATE_ATTACK;
                        break;
                    case STATE_RUN: 
                        newFrame = LABEL_STATE_RUN;
                        break;
                }
                _modelState.gotoAndPlay( newFrame );
                updateGraphicRefs();
            }
            
            var point:Point = new Point( _curDest.x, _curDest.y );
            if ( _eState == STATE_ATTACK && _target != null ) {
                point = new Point( _target.x, _target.y ); 
            }
            
            updateDirection( point.x, point.y );
        }
        
        override protected function updateGraphicRefs():void {
            _modelLevel = null;
            _modelState = this;
            _modelDirection = model;
        }
        
        override public function toString():String {
            return "[SB Enemy " + name + "]";
		}
        
    }

}