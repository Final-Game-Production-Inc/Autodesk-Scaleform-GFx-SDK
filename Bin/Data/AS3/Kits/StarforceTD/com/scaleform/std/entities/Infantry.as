/**************************************************************************

Filename    :   Infantry.as

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
    
    import scaleform.clik.motion.Tween;
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.data.towers.TowerAPCData;
	import com.scaleform.std.utils.Constants;
    import com.scaleform.std.fx.FxCombatText;  
    import com.scaleform.std.fx.FxDeathHuman;
    
    import com.scaleform.std.system.SoundSystem;
    
    public class Infantry extends MoveEntity
    {
        
        // State constant defintions.
        public static const STATE_IDLE:uint     = 0;
        public static const STATE_ATTACK:uint   = 1;
        public static const STATE_RUN:uint      = 2;
        
        // Labels for grade keyframes of _modelLevel.
        public static const LABEL_GRADE_1:String = "level1";
        public static const LABEL_GRADE_2:String = "level2";
        public static const LABEL_GRADE_3:String = "level3";
        
        // Labels for state keyframes of _modelState.
        public static const LABEL_STATE_IDLE:String     = "idle1";
        public static const LABEL_STATE_ATTACK:String   = "attack";
        public static const LABEL_STATE_RUN:String      = "run";
        
        // Timers
        protected var _idleUpdateTime:Number = 0; // How often we change directions when in idle. Randomly generated each time updateGraphics() is called if STATE_IDLE.
        protected var _nextAttackTime:Number = 0; // The next time we can attack.
        protected var _nextRegenTime:Number = -1; // The next time we will regenerate some health. -1 means never.
		protected var _regenDelay:Number = .3; // s
        
        // Destination
        protected var _newDestination:Point = null; // A new destination, set by the TowerAPC if the rally point was changed. Will override _destination after finishing STATE_ATTACK.
        protected var _destination:Point = null; // Our current destination.
        public var rallyPointLocation:Point = null;
        
        // Infantry team
        protected var _infantryTeam:Vector.<Infantry> = null;
        
        // Stats
        public var regenRate:Number = 0;
        
        // Graphics
        public var model:MovieClip;
        
        public function Infantry( level:Level ) {
            super( level );
            
            // Initialize our teammate vector and start listening for unit death events.
            _infantryTeam = new Vector.<Infantry>();
            _level.addEventListener( EnemyEvent.UNIT_DIED, handleTeammateDiedEvent, false, 0, true );
            
            configStats();
        }
        
        override protected function configUI():void {
            super.configUI();
        }
        
        override protected function configStats():void {
            speed = TowerAPCData.INFANTRY_MAX_SPEED;
            attackSpeed = GameData.getTowerAttackSpeed( GameData.TOWER_TYPE_APC, grade );
            
            range = GameData.getTowerRange( GameData.TOWER_TYPE_APC, grade );
            maxHealth = GameData.getTowerHealth( GameData.TOWER_TYPE_APC, grade );
            
            minDamage = GameData.getTowerMinDamage( GameData.TOWER_TYPE_APC, grade );
            maxDamage = GameData.getTowerMaxDamage( GameData.TOWER_TYPE_APC, grade );
            
            regenRate = TowerAPCData.INFANTRY_REGEN_RATE;
        }
        
        override public function set eState( val:uint ):void {
            super.eState = val;
            if ( val != STATE_IDLE ) { _nextRegenTime = -1; }
        }
        
        override public function tick( lmode:uint ):void {
            super.tick( lmode );
            update(lmode);
            
            if (lmode != Level.LMODE_MOVABLE) return;
            
            updateGraphics();
        }
        
        protected function update(lmode:uint):void {
            var cTime:Number = _level.levelTimer;
            
            // If we have received a new destination, move there as soon as we finish attacking.
            if ( lmode == Level.LMODE_MOVABLE && _eState != STATE_ATTACK && _newDestination != null ) {
                eState = STATE_RUN;
                setDestination( _newDestination );
                _newDestination = null;
            }
            // If we're moving around, don't change anything.
            else if ( _destination ) {
                eState = STATE_RUN;

				updatePosition();
			
            }
            else {
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

                if (lmode != Level.LMODE_MOVABLE) return;
                
                // If we don't have a target, try to find one.
                if (_target == null) {
                    _target = findTarget();
                }
                
                // If we now have a valid target, attack him.
                if ( _target != null ) {
                    eState = STATE_ATTACK;
					_nextRegenTime = -1;
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
                    if ( _nextRegenTime > 0 && _nextRegenTime < cTime ) { 
                        health += regenRate;
                        _nextRegenTime = -1; // Set it so we don't end up here again until we have health < healthMax.
                    }
                    if ( _nextRegenTime == -1 && health < maxHealth ) {
                        _nextRegenTime = cTime + _regenDelay + firstRegenDelay;
                    }
                }
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
				reachedDestination();
			}
		}
        
        protected function attack( e:MoveEntity ):void {
            // trace( this + " :: attack( " + e + " ) ");
            _nextAttackTime = _level.levelTimer + attackSpeed;
            _target.addEventListener( EnemyEvent.UNIT_DIED, onTargetDeath, false, 0, true ); // Don't believe this needs to be cleaned up.
            dispatchEvent( new AttackEvent( AttackEvent.UNIT_ATTACK, true, true, _target, 0, minDamage ) );
            
            // @TODO: Move to attack().
            SoundSystem.playSound(SoundSystem.SOUND_FX, "Infantry_Attack.mp3");
        }
        
        public function addTeammateRef( inf:Infantry ):void {
            _infantryTeam.push( inf );
        }
        
        protected function handleTeammateDiedEvent( ee:EnemyEvent ):void {
            // Remove this teammate from the _infantryTeam array so we no longer look at his target.
            var numTeam:uint = _infantryTeam.length;
            var deadTeammate:Infantry = ee.target as Infantry;
            var idx:int = _infantryTeam.indexOf( deadTeammate );
            if ( idx >= 0 ) { 
                _infantryTeam.splice( idx, 1 );
            }
        }
        
        protected function onTargetDeath( e:EnemyEvent ):void {
            // trace(" >> " + this + " :: onTargetDeath()");
            if ( _target != null ) {
                _target.removeEventListener( EnemyEvent.UNIT_DIED, onTargetDeath, false );
                _target = null;
            }
            
            // NFM: I'm not sure this is necessary. Kind of looks strange to have them snap so quickly. A little delay is "humanizing".
            // Force an update immediately (switch to IDLE most of the time).
            // update(); 
            // updateGraphics(); 
        }
        
        protected function faceRandomDirection():void {
            var randDir:uint = Math.floor( Math.random() * 16 + 1 );
            _modelDirection.gotoAndPlay( randDir );
            _idleUpdateTime = _level.levelTimer + Math.floor( Math.random() * 10 + 4 );
        }
        
        override public function handleAttackEvent( e:AttackEvent ):void {
            // trace( this + " :: handleAttackEvent( " + e.damage + " )");
            health -= e.damage;
            
            //if (isCrit) {
            //    var combatText:FxCombatText = new FxCombatText( e.isCrit ? "CRIT" : String(e.damage), "#aa0000" );
            //    combatText.x = this.x;
            //    combatText.y = this.y;
            //    _level.addToFXLayer( combatText );
            //}            
            
            if ( health <= 0 ) {
                playDeathAnimation();
                dispatchEvent( new EnemyEvent( EnemyEvent.UNIT_DIED, true, true, 0 ) );
            }
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
        
        // Set a new destination that will interupt our current movement.
        public function setNewDestination( dest:Point ):void {
            _newDestination = dest;
            rallyPointLocation = dest;
        }
        
        public function setDestination( dest:Point ):void {
            // Cancel current movement.
            // Start moving toward new destination.
            _destination = dest;
            updateDestination();
        }
        
        override protected function updateDestination():void {
			var dist:Number = Utils.calcDistanceCoords( x, y, _destination.x, _destination.y );
			_dsteps = dist / (speed / Constants.SPEED_STEP);
			_dx = (_destination.x - x) / _dsteps;
			_dy = (_destination.y - y) / _dsteps;
			//trace("steps: " + _dsteps + ", (dx, dy): " + _dx + ", " + _dy);							  
        }
        
        protected function reachedDestination( ):void {
            _destination = null;
            
            // Force an update immediately since we've reached our destination.
            update(Level.LMODE_MOVABLE); 
            updateGraphics();
        }

        protected function findTarget():Enemy {
            // trace( this + " :: findTarget(), " + _level.levelTimer);
            var newTarget:Enemy = null;
            
            // OPTI - Track the distance of enemies, once they're out of range, skip em.
            // OPTI - Could check the number of enemies alive in tick() and never call findTarget().
            var enemies:Array = _level.getActiveEnemies(); // OPTI - Could be handed subset of enemies by level.
            if ( enemies.length == 0 ) { return newTarget; }
            
            var closestEnemyDist:Number = range + 1;
            var closestEnemy:Enemy = null;
            var closestEnemyWithoutTarget:Enemy = null;
            
            var numEnemies:uint = enemies.length;
            for ( var i:uint = 0; i < numEnemies; i++ ) {
                var enem:Enemy = enemies[i];
                var tdist:Number = Utils.calcDistanceEntity( this, enem );
                if (tdist < closestEnemyDist) {
                    closestEnemyDist = tdist;
                    closestEnemy = enem;
                    
                    // Infantry will always try to stop another enemy unless there are no enemies without
                    // a target in range, so we need to track which enemy is closest without a target.
                    if ( closestEnemy.getTarget() == null ) {
                        closestEnemyWithoutTarget = enem;
                    }
                }
            }
            
            newTarget = ( closestEnemyWithoutTarget == null ) ? closestEnemy : closestEnemyWithoutTarget;
            
            var numTeam:uint = _infantryTeam.length; 
            if ( newTarget == null && numTeam > 0 ) {
                for (var j:uint = 0; j < numTeam; j++ ) {
                    var teamTarget:MoveEntity = _infantryTeam[j].getTarget() as MoveEntity;
                    if ( teamTarget != null ) {
                        newTarget = teamTarget as Enemy;
                    }
                }
            }
            
            return newTarget;
        }
        
        override protected function updateGraphicRefs():void {
            _modelLevel = this;
            _modelState = model;
            _modelDirection = model.model;
        }
        
        override protected function updateGraphics():void {
            var gradeChanged:Boolean = false;
            var stateChanged:Boolean = false;
            var newFrame:String;
            
            if ( _gxGrade != grade ) { 
                _gxGrade = grade;
                gradeChanged = true;
                switch ( grade ) { 
                    case 1: 
                        newFrame = LABEL_GRADE_2; 
                        break;
                    case 2: 
                        newFrame = LABEL_GRADE_3; 
                        break;
                }
                _modelLevel.gotoAndStop( newFrame );
                updateGraphicRefs();
            }
            
            // trace(" >> GXS: " + _gxState + ", ES: " + _eState);
            if ( _gxState != _eState ) {
                _gxState = _eState;
                stateChanged = true;
                switch ( _eState ) { 
                    case STATE_IDLE: 
                        newFrame = LABEL_STATE_IDLE;
                        break;
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
            
            // If our state has changed, update the direction we're facing (IDLE -> ATTACK).
            var point:Point = new Point(0, 0);
            if ( _eState == STATE_ATTACK && _target != null ) { 
                point = new Point( _target.x, _target.y ); 
                updateDirection( point.x, point.y );
            }
            else if ( _eState == STATE_RUN && _destination != null) { 
                point = new Point( _destination.x, _destination.y); 
                updateDirection( point.x, point.y );
            }
            else if ( _eState == STATE_IDLE && _idleUpdateTime < _level.levelTimer ) {
                faceRandomDirection();
            }
        }
        
    }

}