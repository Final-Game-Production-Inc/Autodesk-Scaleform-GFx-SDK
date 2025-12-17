/**************************************************************************

Filename    :   TowerAPC.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.entities
{
    import com.scaleform.std.events.EnemyEvent;
    import flash.display.DisplayObject;
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.events.Event;
    import flash.geom.Point;
    import flash.text.TextField;
    
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.levels.Level;
    import com.scaleform.std.events.AttackEvent;
    import com.scaleform.std.data.towers.TowerAPCData;
    import com.scaleform.std.core.GameData;
    
    import com.scaleform.std.system.SoundSystem;
    
    public class TowerAPC extends Tower
    {
        
    // Public Properties
        public var rallyPoint:Point = null; // Coordinates on the Level where new units will head to.
        
    // Protected Properties
        // Timers
        protected var _lastSpawnTime:Number = 0; // The Level.levelTimer the last time we spawned a unit.
        protected var _nextSpawnTime:Number = -1; // The Level.levelTimer when we will next spawn a unit (if possible).
        protected var _respawnRate:Number = 0; // Seconds until the next unit will be spawned.
        
        // Infantry
        protected var _numActiveUnits:Number = 0; // The number of active units.
        protected var _maxActiveUnits:Number = 3; // The maximum number of units that can be  spawned at once.
        protected var _activeUnits:Vector.<AllyInfantry> = null; // Array of active units.
		
		protected static var DEPLOY_IDLE:uint = 0;
		protected static var DEPLOY_START:uint = 1;		
		protected static var DEPLOY_SPAWNED:uint = 2;
		protected var _deployMode:uint = DEPLOY_IDLE; 	// PPS: Stop deploy when upgrading
		
		protected var _firstSpawn:Boolean = true;
        
        // Raly Point
        protected var _coordPool:Array = null; // A pool of available coordinates around the rally point for the infantry to use.
        
        public function TowerAPC( level:Level = null ) {
            type = GameData.TOWER_TYPE_APC;
            
            super( level );
            
            _activeUnits = new Vector.<AllyInfantry>();
            _nextSpawnTime = _level.levelTimer + 2;// _respawnRate;
        }
        
        override protected function configUI():void {
            super.configUI();
            
            // Find the nearest path marker in the _level to use as our default Rally Point.
            findDefaultRallyPoint(); 
        }
        
        override public function cleanup():void {
            super.cleanup();
            while ( _activeUnits.length > 0 ) { 
                var unit:MoveEntity = _activeUnits.pop() as MoveEntity;
                _level.removeFromMainLayer( unit );
            }
        }
        
        override protected function configStats():void {
            range               = TowerAPCData.INFANTRY_SPAWN_RANGE;
            _respawnRate        = TowerAPCData.INFANTRY_RESPAWN_RATE;
            _maxActiveUnits     = TowerAPCData.INFANTRY_NUM_MAX_UNITS;
        }
        
        override public function upgrade():void {
            grade++;
            
            for ( var i:uint = 0; i < _activeUnits.length; i++ ) {
                _activeUnits[i].upgrade();
            }
            
            configStats();
            updateGraphics();
        }
        
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
                updateDirection();
                updateGraphicRefs();
            }
            
            //trace(" _gxState: " + _gxState )
            //trace(" _ _towerState: " +  _towerState);
            if ( stateChanged || _gxState != _towerState) {
                //trace("> " + this + " :: state changed!");
                _gxState = _towerState;
                switch (_towerState) { 
                    case STATE_IDLE: 
                        newFrame = LABEL_STATE_IDLE;
                        break;
                    case STATE_ATTACK: 
                        newFrame = LABEL_STATE_ATTACK;
                        break;
                    case STATE_DEPLOY:
						
						// PPS: Edge cases when state changes while deploying
						if (_deployMode == DEPLOY_SPAWNED) {
							// PPS: If we let this pass through, then we get an additional spawn; stop that
							//      But we need to invoke the appropriate callback
							deployAnimationComplete();
							return;
						}
						
                        newFrame = LABEL_STATE_DEPLOY;
                        break;
                    default:
                        break;
                }
                _modelState.gotoAndPlay( newFrame );
                updateGraphicRefs();
            }
            
        }		
		
        override protected function updateGraphicRefs():void {
            _modelLevel = this;
            _modelState = model;
        }
        
         // Special implementation for the APC Tower since it doesn't attack.
        override public function tick( lmode:uint):void {
            if ( _numActiveUnits > 0 ) {
                for (var i:uint = 0; i < _numActiveUnits; i++) { 
                    _activeUnits[i].tick( lmode );
                }
            } 
            
            if (lmode != Level.LMODE_TOWER) return;
            
            // If we have not set a nextSpawnTime, then we have the maximum number of units already.
            if ( _nextSpawnTime < 0 ) { 
                return; 
            }
            // If it's time to spawn another unit, do so immediately.
            else if ( _nextSpawnTime <= _level.levelTimer ) {
                deployUnit();
            }
        }
        
        public function deployAnimationReady():void {
			if (_firstSpawn) {
				for (var i:uint = 0; i < _maxActiveUnits; i++) {
					spawnUnit();
				}
				_nextSpawnTime = -1;
				_firstSpawn = false;
			} else {
				spawnUnit();
			}
        }
        
        public function deployAnimationComplete():void {
            towerState = eState = STATE_IDLE;
			_deployMode = DEPLOY_IDLE;
            // update();
            updateGraphics();
        }
        
        protected function deployUnit():void {
            towerState = eState = STATE_DEPLOY;
			_deployMode = DEPLOY_START;
            updateGraphics();
            
            // If we can spawn more units, schedule the next spawn.
            if ( _numActiveUnits + 1 < _maxActiveUnits ) {
                _nextSpawnTime = _level.levelTimer + _respawnRate;
            } else {
                _nextSpawnTime = -1; // Otherwise, prevent the tower from updating until the death of a unit.
            }
        }
        
        protected function spawnUnit():void {
			_deployMode = DEPLOY_SPAWNED;
            trace( "\n >> spawnUnit() ");
            var newUnit:AllyInfantry = new AllyInfantry( _level );
            newUnit.addEventListener( EnemyEvent.UNIT_DIED, handleUnitDeath, false, 0, true );
            
            // Adjust to move them toward the door of the APC before we add them to the stage.
            newUnit.x = x + 24;
            newUnit.y = y + 32;
            
            var coords:Point = _coordPool.shift() as Point
            trace(" \t coords: " + coords );
            trace(" \t coordsPool.length: " + _coordPool.length );
            newUnit.setNewDestination( coords );
            
            for ( var i:uint = 0; i < _activeUnits.length; i++ ) {
                var inf:AllyInfantry = _activeUnits[ i ] as AllyInfantry;
                inf.addTeammateRef( newUnit );
                newUnit.addTeammateRef( inf );
            }
            
            newUnit.setGrade( grade );
            
            _activeUnits.push( newUnit );
            _numActiveUnits = _activeUnits.length;
            trace(" \t _numActiveUnits: " + _numActiveUnits );
            trace(" \t _activeUnits.length: " + _activeUnits.length);
            
            _level.addToMainLayer( newUnit );
        }
        
        protected function generateCoordsForRally():void {
            if ( _coordPool == null ) {
                _coordPool = [];
            }
            
            _coordPool.length = 0;
            
            _coordPool.push( new Point(rallyPoint.x - 30, rallyPoint.y - 15 ) );
            _coordPool.push( new Point(rallyPoint.x + 30, rallyPoint.y - 15 ) );;
            _coordPool.push( new Point(rallyPoint.x, rallyPoint.y + 20 ) );
            
            // Update all the unit's destination to make them line up in formation.
            for ( var i:uint = 0; i < _numActiveUnits; i++ ) {
                var inf:AllyInfantry = _activeUnits[ i ] as AllyInfantry;
                inf.setNewDestination( _coordPool.shift() );
            }
        }
        
        protected function handleUnitDeath( e:EnemyEvent ) {
            trace( "\n >> handleUnitDeath() ");
            var ally:AllyInfantry = e.target as AllyInfantry;
            
            var idx:int = _activeUnits.indexOf( ally );
            trace("\t idx: " + idx );
            trace("\t _numActiveUnits: " + _numActiveUnits );
            
            if ( idx >= 0 && _activeUnits.length >= 1 ) {
                trace(" \t REMOVING: " + ally );
                _activeUnits.splice( idx, 1 );
                _numActiveUnits = _activeUnits.length;
                _coordPool.push( ally.rallyPointLocation );
                _level.removeFromMainLayer( ally );
                _nextSpawnTime = _level.levelTimer + _respawnRate;
            }
        }
        
        // Iterate over the available path coordinates and find the closest one.
        protected function findDefaultRallyPoint():void {
            var pathCoords:Array = _level.pathCoords;
            var length:uint = pathCoords.length;
            
            var bestPointIdx:uint = 0;
            var bestPointDist:Number = range + 1; // Arbitrary upper bound.
            for ( var i:uint = 0; i < length; i++ ) {
                var point:Point = pathCoords[ i ];
                var dist:Number = Utils.calcDistanceCoords( this.x, this.y, point.x, point.y );
                if ( dist < bestPointDist ) { 
                    bestPointDist = dist;
                    bestPointIdx = i;
                }
            }
            
            rallyPoint = pathCoords[ bestPointIdx ];
            generateCoordsForRally();
        }
        
        public function setRallyPoint( px:Number, py:Number ):void {
            rallyPoint = new Point( px, py );            
            generateCoordsForRally();
			
			var s:Number = Utils.generateRandomBetween(0, 3);
			var sound:String = "Infantry_Rally1.mp3";
			switch (s) {
				case 0: sound = "Infantry_Rally2.mp3"; break;
				case 1: sound = "Infantry_Rally3.mp3"; break;
			}
            SoundSystem.playSound(SoundSystem.SOUND_FX, sound);			
        }
        
        override public function toString():String {
            return "[TowerAPC" + this.name + "]";
        }
    }

}