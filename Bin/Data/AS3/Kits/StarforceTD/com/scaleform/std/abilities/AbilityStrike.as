/**************************************************************************

Filename    :   AbilityStrike.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.abilities
{
    import flash.events.Event;
    import flash.display.MovieClip;
    import flash.geom.Point;
    import flash.events.TouchEvent;
    import flash.events.MouseEvent;
    
    import fl.motion.easing.*;
    
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.entities.Enemy;
    import com.scaleform.std.levels.Level;
    import com.scaleform.std.data.abilities.AbilityData;
    import com.scaleform.std.events.AttackEvent;
    import com.scaleform.std.fx.FxExplosion;
    import com.scaleform.std.fx.FxAbilityStrike;
    import com.scaleform.std.fx.FxAbilityStrike;
    import com.scaleform.std.fx.FxAbilityStrikeScreen;
    import com.scaleform.std.fx.FxOrbitalScar;
    import com.scaleform.std.hud.RallyIndicator;
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.utils.Constants;
    
    import scaleform.clik.motion.Tween;
    
    import com.scaleform.std.system.SoundSystem;
    
    public class AbilityStrike extends Ability 
    {
        
    // Public Properties:
        protected var _minDamage:Number = 0;
        protected var _maxDamage:Number = 0;
        protected var _armorPen:Number = 0;
        protected var _attackSpeed:Number = 0;
        protected var _splashRadius:Number = 0;
        
    // Protected Properties:
        protected var _fxRef:MovieClip = null; // Reference to the Fx symbol for the AbilityStrike
        protected var _nextAttackTime:Number = 0; // The time when our next attack will be processed.
        
        protected var _pathInputCoords:Vector.<Point> = null;
        protected var _inputPathTrackDuration:Number = 3;
        protected var _timeToStopTrackingInput:Number = 0;
        protected var _followPathSpeed:Number = 40;
        
        protected var _currentPathCoords:Point = null;
        protected var _currentPathCoordsIdx:int = 0;
        protected var _tween:Tween = null;
        protected var _isTrackingInput:Boolean = false;
        protected var _inputTrackDelay:Number = 0.05;
        protected var _timeNextInputTrack:Number = 0;
        
        protected var _isComplete:Boolean = false;
        
        protected var _fxScreenRef:MovieClip = null;
        
        protected var _targetIndicator:RallyIndicator = null;
        
        protected var _timeNextOrbitalScar:Number = 0;
        protected var _orbitalScarDelay:Number = .5;
        
        public function AbilityStrike( level:Level ) {
            super( level );
            _pathInputCoords = new Vector.<Point>();
        }
        
        override protected function configStats():void {
            _duration =     GameData.getAbilityDuration( GameData.ABILITY_STRIKE );
            
            _minDamage =    GameData.getAbilityMinDamage( GameData.ABILITY_STRIKE );
            _maxDamage =    GameData.getAbilityMaxDamage( GameData.ABILITY_STRIKE );
            _attackSpeed =  GameData.getAbilityAttackSpeed( GameData.ABILITY_STRIKE );
            _armorPen =     GameData.getAbilityArmorPen( GameData.ABILITY_STRIKE );
            _splashRadius = GameData.getAbilitySplashRadius( GameData.ABILITY_STRIKE );
        }
        
        override public function start( ax:Number, ay:Number ):void {
            super.start( ax, ay );
            
            var startCoords:Point = new Point( ax, ay );
            _pathInputCoords.push( startCoords );
            _currentPathCoords = startCoords;
            
            // @TODO: Delay this until after the inital blast.
            // updateDestination();
            
            _fxRef = new FxAbilityStrike();
            _level.addToFXLayer( _fxRef );
            _fxRef.x = ax;
            _fxRef.y = ay;
            _fxRef.gotoAndPlay("attack");
            
            _targetIndicator = new RallyIndicator();
            _targetIndicator.x = ax;
            _targetIndicator.y = ay;
            _level.addToBottomLayer( _targetIndicator );
            _targetIndicator.mouseEnabled = false;
            _targetIndicator.gotoAndPlay("on");
            _targetIndicator.addEventListener( "onRallyOffComplete", handleTargetIndicCloseComplete, false, 0, true );
            
            _fxScreenRef = new FxAbilityStrikeScreen();
            _level.addToFXLayer( _fxScreenRef );
            
            var eventType:String = Constants.isTouchEnabled() ? TouchEvent.TOUCH_MOVE : MouseEvent.MOUSE_MOVE;
            stage.addEventListener( eventType, handleInputPathMove, false, 0, true );
            
            eventType = Constants.isTouchEnabled() ? TouchEvent.TOUCH_END : MouseEvent.MOUSE_UP;
            stage.addEventListener( eventType, handleInputPathEnd, false, 0, true );
            
            _isTrackingInput = true;
            _timeToStopTrackingInput = _level.levelTimer + _inputPathTrackDuration;
            
            SoundSystem.playSound(SoundSystem.SOUND_FX, "PowerOrbitalStrike.mp3");
        }
        
        protected function handleTargetIndicCloseComplete( e:Event ):void {
            _level.removeFromBottomLayer( _targetIndicator );
            _targetIndicator = null;
        }
        
        protected function handleInputPathMove( e:Event ):void {
            var ax:Number; 
            var ay:Number;
            if ( e is MouseEvent ) { 
                var me:MouseEvent = e as MouseEvent;
                ax = me.stageX;
                ay = me.stageY;
            }
            else {
                var te:TouchEvent = e as TouchEvent;
                ax = te.stageX;
                ay = te.stageY;
            }
            
            _targetIndicator.x = ax;
            _targetIndicator.y = ay;
               
            if ( _timeNextInputTrack <= _level.levelTimer ) {
                var startCoords:Point = _pathInputCoords[0];
                var dist:Number = Utils.calcDistanceCoords( startCoords.x, startCoords.y, ax, ay );
                _pathInputCoords.push( new Point( ax, ay ) );
                if ( _pathInputCoords.length >= 2 && (_tween == null || _tween.paused )) { updateDestination(); }
                
                _timeNextInputTrack = _inputTrackDelay + _level.levelTimer;
                if ( ( dist / (_followPathSpeed) ) >= _duration ) {
                    stopTrackingInput();
                }
            }
        }
        
        protected function handleInputPathEnd( e:Event ):void {
            stopTrackingInput();
        }
        
        protected function stopTrackingInput():void {
            _isTrackingInput = false;
            
            var eventType:String = Constants.isTouchEnabled() ? TouchEvent.TOUCH_MOVE : MouseEvent.MOUSE_MOVE;
            stage.removeEventListener( eventType, handleInputPathMove, false );
            eventType = Constants.isTouchEnabled() ? TouchEvent.TOUCH_END : MouseEvent.MOUSE_UP;
            stage.removeEventListener( eventType, handleInputPathEnd, false );
        }
        
        override public function end():void {
            super.end();
            
            generateOrbitalScar(); // One last scar for good measure.
            
            _fxRef.gotoAndPlay("close");
            _targetIndicator.gotoAndPlay("rallyOff");
            
            _pathInputCoords = null;
            _isComplete = true;
            _fxRef = null;
            _fxScreenRef = null;
        }
        
        override public function notifyPaused( value:Boolean ):void {
            if (_tween) {
                _tween.paused = value;
            }
        }
        
        protected function updateDestination():void {
            if ( _tween != null ) { _tween.paused = true; }
            if ( _currentPathCoordsIdx + 1 < _pathInputCoords.length ) {
                var pPathCoords:Point = _currentPathCoords;
                _currentPathCoords = _pathInputCoords[ ++_currentPathCoordsIdx ];
                
                // Calculate the speed that it will take us to reach the next waypoint.
                var dist:Number = Utils.calcDistanceCoords( pPathCoords.x, pPathCoords.y, _currentPathCoords.x, _currentPathCoords.y );
                var tweenTime:Number = (dist / _followPathSpeed) * 1000;
                
                _tween = new Tween(  tweenTime, 
                                    _fxRef, 
                                    {
                                        x:_currentPathCoords.x, 
                                        y:_currentPathCoords.y
                                    }, 
                                    {
                                        ease:Linear.easeOut, 
                                        onComplete:reachedPathCoord
                                    }
                                 );
            }
        }
        
        protected function reachedPathCoord( t:Tween ):void {
            if ( !_isComplete ) {
                updateDestination();
            }
        }
        
        override public function tick( lmode:uint):void {
            if (lmode != Level.LMODE_ABILITY) return;
            
            if ( _timeToStopTrackingInput <= _level.levelTimer && _isTrackingInput ) {
                stopTrackingInput();
            }
            
            if ( _destroyTime < _level.levelTimer ) {
                end();
                return;
            }
            
            if ( _timeNextOrbitalScar < _level.levelTimer ) {
                generateOrbitalScar();
                _timeNextOrbitalScar = _level.levelTimer + _orbitalScarDelay;
            }
            
            if ( _nextAttackTime < _level.levelTimer ) {
                attack();
            }
        }
        
        protected function generateOrbitalScar():void {
            var fxOrbitalScar = new FxOrbitalScar();
            
            fxOrbitalScar.x = _fxRef.x;
            fxOrbitalScar.y = _fxRef.y;
            
            _level.addToBottomLayer( fxOrbitalScar );
            fxOrbitalScar.gotoAndPlay( "on" )
        }
        
        protected function attack():void { 
            _nextAttackTime = _level.levelTimer + _attackSpeed; // Set the next time we'll attack.
            
            var enemies:Array = _level.getActiveEnemies();
            var numEnemies:uint = enemies.length;
            for ( var i:uint = 0; i < numEnemies; i++ ) {
                var en:Enemy = enemies[ i ];
                if ( en != null && Utils.calcDistanceCoords( _currentPathCoords.x, _currentPathCoords.y, en.x, en.y ) < _splashRadius ) {
                    var dmg:Number = Utils.generateRandomBetween( _minDamage, _maxDamage );
                    dispatchEvent( new AttackEvent( AttackEvent.ABILITY_ATTACK, true, true, en, 0, dmg ) );
                }
            }
        }
        
    }

}