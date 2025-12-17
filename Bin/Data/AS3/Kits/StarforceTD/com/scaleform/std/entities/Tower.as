/**************************************************************************

Filename    :   Tower.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.entities
{
    import flash.display.DisplayObject;
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.events.Event;
    import flash.geom.Point;
    import flash.display.CapsStyle;
    
    import fl.motion.easing.Linear;
    
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.levels.Level;
    import com.scaleform.std.events.AttackEvent;
    import com.scaleform.std.utils.Constants;
    import com.scaleform.std.fx.FxExplosion;
    import com.scaleform.std.events.EnemyEvent;
    
    import scaleform.clik.motion.Tween;
    
    public class Tower extends Entity
    {
    // Static Constants
        // State definitions for the Tower's state.
        public static const STATE_IDLE:uint     = 0;
        public static const STATE_ATTACK:uint   = 1;
        public static const STATE_DEPLOY:uint   = 2;
        
        // Labels for grade keyframes of _modelLevel.
        public static const LABEL_GRADE_0:String = "level1";
        public static const LABEL_GRADE_1:String = "level2";
        public static const LABEL_GRADE_2:String = "level3";
        
        // Labels for state keyframes of _modelState.
        public static const LABEL_STATE_IDLE:String     = "idle";
        public static const LABEL_STATE_ATTACK:String   = "attack";
        public static const LABEL_STATE_DEPLOY:String   = "deploy";
        
        // Labels for the keyframes of _modelAttack.
        public static const LABEL_ATTACK_PLAY:String    = "play";
        
    // Public Properties
        // Type (APC, Mech, Tank, OST... Defined in GameData)
        public var type:uint = GameData.TOWER_TYPE_NONE; 
        // Range of the tower (px).
        public var range:Number = 200;
        // The delay between the tower's attacks.
        public var attackSpeed:Number = 0;
        // The percentage chance of a critical hit (2x).
        public var criticalChance:Number = 0;
        
        // Type of effect (DOT, Amp Damage, Slow, None... Defined in GameData).
        public var effectType:uint = 0;
        // Grade of the Tower's meta ability (slow, DOT, amplify damage...).
        public var effectGrade:uint = 0;
        // The value of the effect applied to targets when hit.
        public var effectValue:Number = 0;
        public var effectDuration:Number = 0;
        public var effectTickSpeed:Number = 0;
        
        // The tower's current target.
        public var target:Enemy = null;
        // The last time the tower attacked.
        public var lastAttackTime:Number = 0;
        // The last time the tower searched for a new target.
        public var lastSearchTime:Number = 0;
        // The delay between searching for a new target.
        public var searchDelay:Number = .5;
        // A randomly generated update offset so that not all towers are updated in the same frame.
        public var updateOffset:Number = 0;
        
    // Protected Properties
        // The tower's current state. // @TODO: Replace with Entity.eState.
        protected var _towerState:uint = 0;
        // The next time that this tower can attack it's target.
        protected var _nextAttackTime:Number = 0;
        
        // The next time the tower should update its direction (the way it's currently facing).
        protected var _nextDirectionUpdateTime:Number = 0;
        // The time between directional updates.
        protected var _nextDirectionUpdateDelay:Number = .25;
        
    // UI References
        // MovieClip that contains all levels, states, and animations for Tower.
        public var model:MovieClip; 
         // Reference to the MovieClip that controls the attack animation of the Tower.
        protected var _modelFx1:MovieClip = null;
        // Reference to the MovieClip that controls the 2nd attack animation of the Tower.
        protected var _modelFx2:MovieClip = null; 
        // Reference to the MovieClip that controls the 3rd attack animation of the Tower.
        protected var _modelFx3:MovieClip = null; 
        // Reference to the attack graphic used for debugging.
        protected var _attackGraphic:DisplayObject;
        
    // Initialization
        public function Tower( level:Level = null ) {
            isAlive = false; // Will be set to true once the Level finishes building us.
            _modelLevel = this; // Do this once pre-constructor so we never need to update it in updateGraphicRefs().
            
            super();
            
            _level = level;
            configStats(); // Set the initial stats for the tower (rng, dmg, spd) based on grade arrays.
        }
        
        public function cleanup():void {
            // To be implemented by subclasses.
        }
        
        protected function configGradeArrays():void {
            // To be implemented by subclasses.
        }
        
        protected function configStats():void {
            range       = GameData.getTowerRange( type, grade );
            attackSpeed = GameData.getTowerAttackSpeed( type, grade );
            
            minDamage   = GameData.getTowerMinDamage( type, grade );
            maxDamage   = GameData.getTowerMaxDamage( type, grade );
            
            criticalChance = GameData.getTowerCritical( type, grade );
        }
        
        protected function updateEffectStats():void {
            effectValue = GameData.getTowerEffectValue( type, effectGrade );
            effectDuration = GameData.getTowerEffectDuration( type, effectGrade );
            effectTickSpeed = GameData.getTowerEffectTickSpeed( type, effectGrade );
        }
        
        override protected function configUI():void {
            super.configUI();
            tabChildren = mouseChildren = false; // Prevent our children from being interacted with.
        }
        
        public function get towerState():uint { return _towerState;  }
        public function set towerState( val:uint ):void {
            if ( _towerState == val ) { return; }
            _towerState = val;
            switch ( val ) {
                case STATE_ATTACK:
                    // 
                    break;
                case STATE_IDLE:
                    //updateGraphics();
                    break;
                default:
                    break;
            }
        }
        
        override public function tick( lmode:uint ):void {
            
            if (lmode != Level.LMODE_TOWER) return;
            
            super.tick( lmode );
            
            if ( !isAlive ) { return }
            
            // Cache the time since almost everything will use it.
            var ctime:Number = _level.levelTimer;
            
            // @TODO: Remove post-testing.
            // if ( target ) { updateDirection(); }
            
            // If we have a target, we'll update the direction more frequently than attacking and/or searching for a target.
            if ( target != null && _nextDirectionUpdateTime < ctime ) { 
                updateDirection();
                _nextDirectionUpdateTime = ctime + _nextDirectionUpdateDelay
            }
            
            // No need to tick the tower if we can't attack. // OPTI - Never call this function if no need to tick.
            if ( _nextAttackTime > ctime ) { 
                return; 
            }
            
            // If our target is dead, it's no longer valid.
            if (target != null) {
                if (target.health <= 0) { target = null; }
            }
            
            // TODO: Only check for a target if the last action we performed < delay.
            // Ensure that we have a target
            if (target == null) {
                // TODO: Revisit. Tower should probably search more often, but maybe we don't need so many enemies.
                //       Simple algorithm could give a region based sort.
                if ( ( ctime - (lastSearchTime + updateOffset)) < searchDelay ) { 
                    return; 
                }
                // OPTI -- Only check for a new target every second or so.
                target = findTarget();
                towerState = (target != null) ? STATE_ATTACK : STATE_IDLE;
            }
            
            if (target != null) {
                // Ensure that our target is still in range
                var tdist:Number = Utils.calcDistanceEntity( this, target );
                if (tdist > range) { target = null; }
                
                // If we have a target in range, attack it.
                if ( target != null ) { 
                    towerState = STATE_ATTACK;
                    attack( target );
                }
            }
            
            if (target == null) { 
                towerState = STATE_IDLE;
               
            }
        }
        
        public function getUpgradeCost():int {
            return (grade < 2) ? GameData.getTowerUpgradeCost( type, grade + 1 ) : -1;
        }
        public function getRefundValue():int {
            return GameData.getTowerRefund( type, grade );
        }
        
        public function upgrade():void {
            grade++;
            
            configStats();
            updateGraphics();
        }
        
        public function upgradeEffect():void {
            effectGrade++;
            updateEffectStats();
        }
        
        override protected function updateGraphicRefs():void {
            _modelState = model;
            _modelDirection = model.model;
            
            if ( _modelDirection.dir != null ) {
                _modelFx1 = _modelDirection.dir.fx1;
                _modelFx2 = _modelDirection.dir.fx2;
                _modelFx3 = _modelDirection.dir.fx3;
            }
        }
        
        protected function attack( ent:Entity ):void {
            if ( ent == null) { return; } // Saftey catch all in-case something has gone wrong.
			
            //trace( this + " :: attack( " + ent + " )" );
            updateGraphics(); // Update the graphics, since we want the attack to be as up to date as possible.
            playAttackAnimation( ent ); // Play the attack animation.
            _nextAttackTime = _level.levelTimer + attackSpeed; // Clock the time so we don't attack again until we can.
                   
			
			
            // PPS: Why are we continuously adding listeners????
			if ( target == null) { return; }
				target.addEventListener( EnemyEvent.ENEMY_DIED, handleTargetDied, false, 0, true );
			
			
            var dmg:Number = Utils.generateRandomBetween( minDamage, maxDamage ) * ( 1 + (Utils.generateRandomBetween( -15, 15 ) / 100 ) );
            var isCrit:Boolean = Utils.generateRandomBetween(0, 100) < criticalChance;
            if ( isCrit ) {
                dmg = dmg * 2; 
            }
            dispatchEvent( new AttackEvent( AttackEvent.TOWER_ATTACK, true, true, target, type, dmg, id, effectType, effectValue, effectDuration, effectTickSpeed, isCrit ) ); // @TODO: No modifier used here.
            if (_attackGraphic != null) { removeAttackAnimation(); } // @TODO: put this on some sort of removal timer.
        }
        
        
        protected function handleTargetDied( e:EnemyEvent ):void {
            if ( target != null) { 
				
                target.removeEventListener( EnemyEvent.ENEMY_DIED, handleTargetDied, false );
                target = null;
            }
        }
        
        protected function findTarget():Enemy {
            lastSearchTime = _level.levelTimer;
            var newTarget:Enemy = null;
            
            // OPTI - Track the distance of enemies, once they're out of range, skip em.
            // OPTI - Could check the number of enemies alive in tick() and never call findTarget().
            var enemies:Array = _level.getActiveEnemies(); // OPTI - Could be handed subset of enemies by level.
            if (enemies.length == 0) { return newTarget; }
            
            var closestEnemyIdx:uint = 0;
            var closestEnemyDist:Number = range + 1;
            
            var highestPriorityVal:Number = (_level.priorityProperty == Level.PRIORITIZE_POS_X || _level.priorityProperty == Level.PRIORITIZE_POS_Y) ? 0 : stage.stageWidth;
            var levelPriority:uint = _level.priorityProperty;
            for ( var i:uint = 0; i < enemies.length; i++ ) {
                var enem:Enemy = enemies[i];
                var tdist:Number = Utils.calcDistanceEntity( this, enem );           
                if ( tdist < range ) {
                    switch ( levelPriority ) {
                        case Level.PRIORITIZE_POS_X:
                            if ( enem.x > highestPriorityVal ) {
                                newTarget = enem;
                                highestPriorityVal = enem.x;
                            }
                            break;
                        case Level.PRIORITIZE_NEG_X:
                            if ( enem.x < highestPriorityVal ) {
                                newTarget = enem;
                                highestPriorityVal = enem.x;
                            }
                            break;
                        case Level.PRIORITIZE_POS_Y:
                            if ( enem.y > highestPriorityVal ) {
                                newTarget = enem;
                                highestPriorityVal = enem.y;
                            }
                            break;
                        case Level.PRIORITIZE_NEG_Y:
                            if ( enem.y < highestPriorityVal ) {
                                newTarget = enem;
                                highestPriorityVal = enem.y;
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            
            return newTarget;
        }
        
    // Graphics Functions
        // @TODO: No need to call updateGraphicsRefs() repeatedly here... optimize inline maybe?
        override protected function updateGraphics():void {
            // trace(this + " :: updateGraphics()");
            var newFrame:String;
            var stateChanged:Boolean = false;
            
            if ( _gxGrade != grade ) { 
                // trace("> " + this + " :: grade changed: " + grade);
                _gxGrade = grade;
                switch (grade) { 
                    case 1: 
                        newFrame = LABEL_GRADE_1; 
                        break;
                    case 2: 
                        newFrame = LABEL_GRADE_2; 
                        break;
                    default:
                        break;
                }
                
                // trace("newFrame: " + newFrame);
                stateChanged = true;
                _modelLevel.gotoAndStop( newFrame );
                updateGraphicRefs();
            }
            
            if (  _towerState == STATE_ATTACK ) { 
                updateGraphicRefs();
                updateDirection();
            }
            
            // trace(" _gxState: " + _gxState )
            // trace(" _ _towerState: " +  _towerState);
            if ( stateChanged || _gxState != _towerState) {
                // trace("> " + this + " :: state changed!");
                _gxState = _towerState;
                switch (_towerState) { 
                    case STATE_IDLE: 
                        newFrame = LABEL_STATE_IDLE;
                        break;
                    case STATE_ATTACK: 
                        newFrame = LABEL_STATE_ATTACK;
                        break;
                    case STATE_DEPLOY:
                        newFrame = LABEL_STATE_DEPLOY;
                        break;
                    default:
                        break;
                }
                _modelState.gotoAndPlay( newFrame );
                updateGraphicRefs();
            }
            
        }
        
        protected function updateDirection():void {
            //
        }
        
        protected function playAttackAnimation( ent:Entity ):void {
			
			//return;
			
            if (_modelFx1 != null)  {
                _modelFx1.gotoAndPlay( "attack" );
            }
            if (_modelFx2 != null)  {
                _modelFx2.gotoAndPlay( "attack" );
            }
            if (_modelFx3 != null)  {
                _modelFx3.gotoAndPlay( "attack" );
            }
            
			if (target != null)
			{
				var fx:MovieClip = new FxExplosion();
				fx.x = target.x;
				fx.y = target.y;
				_level.addToFXLayer( fx );
			}
            
            // Debugging.
            // Draw a line from the tower to the enemy that we just attacked.
			
            if (Constants.isDebugMode) {
                var dx:Number = ent.x - x;
                var dy:Number = ent.y - y;
                
                var radians:Number = Math.atan2(dy, dx);
                var angle:Number = (radians/Math.PI)*180;
                var leng:Number = Math.sqrt(dx * dx + dy * dy); 
                
                var line_mc:Sprite = new Sprite();
                line_mc.graphics.lineStyle(3, 0x990000, 1, true, "none", CapsStyle.ROUND); 
                line_mc.graphics.lineTo(leng, 0);
                line_mc.x = x;
                line_mc.y = y;
                line_mc.rotation = angle;
                
                _attackGraphic = line_mc;
                _level.addToFXLayer( _attackGraphic );
            }
			
        }
        
        // This is debugging for now.
        protected function removeAttackAnimation():void {
           if ( _attackGraphic != null ) {
                _level.removeFromFXLayer( _attackGraphic );
                _attackGraphic = null;
           }
        }
        
        override public function toString():String {
            return "[Tower " + this.name + "]";
        }
    }

}