/**************************************************************************

Filename    :   Level.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
 
package com.scaleform.std.levels
{
    import com.scaleform.std.abilities.Ability;
    import com.scaleform.std.data.levels.LevelWave;
    import com.scaleform.std.data.levels.LevelWaveData;
    import com.scaleform.std.fx.FxCombatText;
	import com.scaleform.std.test.TestEnemy;
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.display.DisplayObject;
    import flash.events.Event;
    import flash.events.MouseEvent
    import flash.events.TouchEvent;
    import flash.geom.Point;
    import flash.geom.Rectangle;
	import flash.net.URLLoader;
    
    import flash.media.Sound;
    import flash.media.SoundChannel;
    import flash.net.URLRequest;
    
    import scaleform.clik.motion.Tween;
    import fl.transitions.easing.*;
    
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.entities.Entity;
    import com.scaleform.std.entities.MoveEntity;
    import com.scaleform.std.entities.Enemy;
    import com.scaleform.std.entities.Tower;
    import com.scaleform.std.entities.TowerBuildSite;
    import com.scaleform.std.entities.TowerAPC;
    import com.scaleform.std.entities.TowerMech;
    import com.scaleform.std.entities.TowerOrbital;
    import com.scaleform.std.entities.TowerTank;
    import com.scaleform.std.events.AttackEvent;
    import com.scaleform.std.events.EnemyEvent;
    import com.scaleform.std.events.FxEvent;
    import com.scaleform.std.events.LevelEvent;
    import com.scaleform.std.events.AbilityEvent;
    import com.scaleform.std.events.LoadingEvent;
    import com.scaleform.std.events.TowerEvent;
    import com.scaleform.std.events.WaveEvent;
    import com.scaleform.std.fx.FxBase;
    import com.scaleform.std.fx.FxExplosion;
    import com.scaleform.std.hud.HUD;
    import com.scaleform.std.utils.Constants;
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.utils.Layer;
    import com.scaleform.std.data.PlayerAchievements;
    
    import com.scaleform.std.events.SystemEvent;
    
    import com.scaleform.std.entities.EnemyInfantry;
    import com.scaleform.std.entities.EnemyArmoredInfantry;
    import com.scaleform.std.entities.EnemyDrone;
    import com.scaleform.std.entities.EnemyMech;
    import com.scaleform.std.entities.EnemyTank;
    
    import com.scaleform.std.abilities.AbilityDropship;
    import com.scaleform.std.abilities.AbilityStrike;
    import com.scaleform.std.fx.FxBeam;
    import com.scaleform.std.utils.LaneGenerator;
    
    import scaleform.gfx.Extensions;
    
    import com.scaleform.std.system.SoundSystem;
    
    public class Level extends MovieClip
    {
        
        public static const COUNTDOWN_DURATION:uint     = 10; // seconds
        
        public static const MAX_PLAYER_LIVES:uint       = 100;
        
        // Phase definitions
        public static const PHASE_START:uint        = 0;
        public static const PHASE_WAVE:uint         = 1;
        public static const PHASE_INTERMISSION:uint = 2;
        public static const PHASE_END:uint          = 3;
        public static const PHASE_COUNTDOWN:uint    = 4;    
        
        // Level prioritization definitions
        public static const PRIORITIZE_POS_X:uint = 0;
        public static const PRIORITIZE_NEG_X:uint = 1;
        public static const PRIORITIZE_POS_Y:uint = 2;
        public static const PRIORITIZE_NEG_Y:uint = 3;
        
        // Declarations for path/tower markers so that they can be added to the stage.
        public var path0, path1, path2, path3, path4, path5, path6, path7, path8, path9:MovieClip;
        public var path10, path11, path12, path13, path14, path15, path16, path17, path18, path19:MovieClip;
        public var path20, path21, path22, path23, path24, path25, path26, path27, path28, path29:MovieClip;
		public var path30, path31, path32, path33, path34, path35, path36;
        
        public var tower0, tower1, tower2, tower3, tower4, tower5, tower6, tower7, tower8, tower9:MovieClip;
		public var tower10, tower11, tower12, tower13, tower14, tower15, tower16, tower17, tower18, tower19:MovieClip;
        
		public var waveBtn0, waveBtn1, waveBtn2:MovieClip;		
        
        public var end16x9, end4x3:MovieClip;
		
        // Start paused to delay tick() processing; asynch wave data loading from XML
        public var paused:Boolean = true;
        	
        // Map data
		public var Background:MovieClip;	// The background image and effects (used for ability hit testing)
        public var nav:MovieClip; // Reference to the nav mesh for the Level. Can be hit-tested.
        public var towerCoords:Array = null; // Array of Points. Coordinates where towers should be placed on the map.
        public var pathCoords:Array = null; // Array of Points. Coordinates for the map's path.
        public var priorityProperty:uint = 0; // The property Tower's should use to prioritize targets.
        
        // Wave data
        public var waveData:Vector.<LevelWave>;
        public var enemyTypes:Array; // Array of Classes. The types of enemies to be spawned this wave.
        public var enemyCount:Array; // Array of uints. The number of enemies to be spawned this wave.
        public var enemyStaggerDelay:Array; // Array of Numbers. Speed at which enemies should be spawned for this wave.
        
        protected var _waveIdx:uint = 0; // The index of the wave that we're on.
        protected var _nextPhaseTime:Number = Number.MAX_VALUE;
        protected var _waveSourceDirection:Number = 0; // The direction the wave is coming from.
        protected var _spawnIdx:uint = 0;   // Index of the next spawn. This is accumulated through level lifetime
        
		protected var pathDefns:Array = null; 	// Array of arrays (points)
		
        public var paths:Array = []; // An Array of paths;
        // protected var _lanes:Array = []; // An array of lanes.
        
        // Player data
        protected var _gold:Number = 0; // The amount of gold the player currently has.
        protected var _livesRemaining:int = MAX_PLAYER_LIVES; // The number of lives the player currently has left.
        protected var _starScore:uint = 0; // The player's score (in stars [0-3]) for the level.
        
        // Player abilities
        protected var _activeAbilities:Array = null;
        
        protected var _abilityStrikeAvailable:Boolean = false;
        protected var _abilityStrikeCooldown:Number = 0;
        protected var _nextAbilityStrikeTime:Number = 0;
        
        protected var _abilityDropshipAvailable:Boolean = false;
        protected var _abilityDropshipCooldoown:Number = 0;
        protected var _nextAbilityDropshipTime:Number = 0;
        
        // Level data
        public var waveSpawn:Vector.<Spawn>;
        public var phase:uint = PHASE_START; // The current phase of the level.
        public var levelTimer:Number = 0; // Seconds since we began to tick the level.
        
        protected var _levelComplete:Boolean = false;       
        protected var _nextPhaseStartTrigger:Boolean = false;
        
		public var waveBtns:Array; 	// Array of wave start buttons
		
        // Game entities
        public var towers:Array; // Array of Towers. Active towers for the level.
        public var enemies:Array; // Array of Enemies. Active enemies left alive.
        public var towerSites:Array; // Array of TowerBuildSites. Active sites that haven't been used.
        
        // Graphic Layers
        public var layer10:MovieClip = null;    // Top most layer. Reserved for UI.
        public var layer7:MovieClip  = null;    // Layer for effects
        public var layer5:MovieClip  = null;    // Layer for entities and towers.
        public var layer1:MovieClip  = null;    // Layer just above the background for UI elements like range indicator.
        
        public var abilityTouchCanvas:MovieClip;
        
        // Graphic data
        protected var _zSortTime:Number = 0; // The next time we'll perform a z-sort.
        protected var _zSortDelay:Number = 10; // The delay between z-sorting.
        
        // UI
        public var hud:HUD = null;
        protected var _alertedPlayerOfFirstWave:Boolean = false;
        protected var _lastWaveCountDown:Number = 0;
		
		protected var _xmlPath:String;
        
        public static const LMODE_MOVABLE:uint = 0;
        public static const LMODE_TOWER:uint = 1;
        public static const LMODE_ABILITY:uint = 2;
        protected var lmode:uint = 0;
        
        
        public function Level(pathToXml:String) {
            super();
            
			_xmlPath = pathToXml;
			
            Utils.resetIds();
            
            _activeAbilities = [];
            
            // Init level data arrays
            pathCoords = [];
            towerCoords = [];
            enemyTypes = [];
            enemyCount = [];
            enemyStaggerDelay = [];
            
			waveBtns = [];
			
            // Init entity arrays
            towers = [];
            enemies = [];
            towerSites = [];
           
            // Hide the navmesh used for path hitesting..
            nav.visible = false;
            
            createLayers();
            
            // trace(" Utils.isTouchEnabled: " + Constants.isTouchEnabled() );
            
            addEventListener( Event.ENTER_FRAME, configUI, false, 0, true );
            
            waveSpawn = new Vector.<Spawn>;
        }
        
        public function set abilityCanvasEnabled(val:Boolean) { 
            abilityTouchCanvas.visible = val;
        }
        
		protected function getAtmosphere():MovieClip { return null; }
		
        protected function createLayers():void {
            // Create a new layer for the range indicator so it's just above the background.
            layer1 = new Layer();
            layer5 = new Layer();
            layer7 = new Layer();
            layer10 = new Layer();
            
            // Order the layers
            addChildAt( layer1, 1 ); // Bottom. // 0 is bottom. 1 is just above background.
            addChild( layer5 );
            if (abilityTouchCanvas) {
                removeChild(abilityTouchCanvas);
                addChild(abilityTouchCanvas);
                abilityCanvasEnabled = false;
            }
            addChild( layer7 );
			var atmo:MovieClip = getAtmosphere();
			if (atmo) {
				removeChild(atmo);
				atmo.mouseEnabled = false;
				atmo.mouseChildren = false;
				addChild(atmo);
			}
            addChild( layer10 ); // Top
        }
        
        public function configUI( e:Event ):void {
            removeEventListener( Event.ENTER_FRAME, configUI, false );
            
            phase = PHASE_START;
            
            // Setup our persistant event listeners.
            stage.addEventListener( EnemyEvent.ENEMY_DIED, handleEnemyDeathEvent, false, 0, true );
            stage.addEventListener( EnemyEvent.ENEMY_REACHED_DESTINATION, handleEnemyReachedDestination, false, 0, true );
            
            stage.addEventListener( EnemyEvent.REACHED_WP, zSortHandler, false, 0, true );
            stage.addEventListener( EnemyEvent.STOPPED_MOVING, zSortHandler, false, 0, true );
            
            stage.addEventListener( AttackEvent.TOWER_ATTACK, handleTowerAttack, false, 0, true );
            stage.addEventListener( AttackEvent.UNIT_ATTACK, handleUnitAttack, false, 0, true );
            stage.addEventListener( AttackEvent.ABILITY_ATTACK, handleTowerAttack, false, 0, true );
            
            stage.addEventListener( WaveEvent.FORCE_WAVE, handleForceWaveEvent, false, 0, true );
            
            stage.addEventListener( AbilityEvent.USED, handleAbilityUsedEvent, false, 0, true );
            
            stage.addEventListener( FxEvent.COMPLETE, handleFxAnimationComplete, false, 0, true );
            
            stage.addEventListener( SystemEvent.PAUSE_GAME, handlePauseGame, false, 0, true );
            stage.addEventListener( SystemEvent.RESUME_GAME, handleResumeGame, false, 0, true );
            
            // Load the unique level data.
            loadLevelData();
        }
  
		protected function loadLevelData():void {
			var xmlLoader:URLLoader = new URLLoader();
			xmlLoader.addEventListener(Event.COMPLETE, processLevelData);
			xmlLoader.load(new URLRequest(_xmlPath));
		}
		
		protected function processLevelData(e:Event):void {
			var xml:XML = new XML(e.target.data);
			// Gold
			//trace("gold: " + xml.gold);
			gold = xml.gold;
			// Priority
			//trace("priority: " + xml.priority);			
			switch (xml.priority) {
				case "pos-x":	priorityProperty = PRIORITIZE_POS_X; break;
				case "neg-x":	priorityProperty = PRIORITIZE_NEG_X; break;
				case "pos-y":	priorityProperty = PRIORITIZE_POS_Y; break;
				case "neg-y":	priorityProperty = PRIORITIZE_NEG_Y; break;
			}
			// Wave source direction
			//trace("source direction: " + xml["source-direction"]);
			_waveSourceDirection = xml["source-direction"];
			// Path definitions
			pathDefns = [];
			var navPaths:XMLList = xml.navpaths.navpath;
			for each (var navpath:XML in navPaths) {
				var n = [];
				var id:uint = navpath.attribute("id");
				var navPts:XMLList = navpath.navpt;
				for each (var navpt:XML in navPts) {
					//trace("navpt: " + navpt.attribute("name"));
					n.push(this[navpt.attribute("name")]);
				}
				pathDefns[id] = n;
			}
			// Waves
			waveData = new Vector.<LevelWave>;
			var waves:XMLList = xml.waves.wave;
			for each (var wave:XML in waves) {
				var w = new LevelWave;
				var wid:uint = wave.attribute("id");
                
                var startTime:Number = wave.attribute("startTime");
                if (!startTime) {
                    startTime = 5;
                }
                w.startTime = startTime;
                
                var startBonus:uint = wave.attribute("startBonus");                
                w.startBonus = startBonus;
                
				var wavedatas:XMLList = wave.wavedata;
				for each (var wavedata:XML in wavedatas) {
					var type:uint = GameData.ENEMY_TEST;
					var spawnDelay:Number = 0;
					var relSpawnTime:Number = 1;
					var pathIdx:uint = 0;
					var numEnemies:uint = 1;

					var xtype:String = wavedata.attribute("type");					
					//trace("wavedata: " + xtype);
					switch (xtype) {
						case "infantry":	type = GameData.ENEMY_INFANTRY;	break;
						case "armored-infantry":	type = GameData.ENEMY_ARMORED_INFANTRY;	break;
						case "drone": 	type = GameData.ENEMY_DRONE;	break;
						case "tank": 	type = GameData.ENEMY_TANK;	break;
						case "mech": 	type = GameData.ENEMY_MECH;	break;
						case "test": 	type = GameData.ENEMY_TEST;	break;
					default:
						throw new Error("No such unit type - '" + xtype + "'");
					}
					var sd:XMLList = wavedata.attribute("spawndelay");
					spawnDelay = (sd.length() > 0) ? Number(sd) : 0;
					relSpawnTime = wavedata.attribute("spawntime");
					pathIdx = wavedata.attribute("navpath");
					numEnemies = wavedata.attribute("num");
					//trace("spawndelay: " + spawnDelay + ", relSpawnTime: " + relSpawnTime + ", pathIdx: " + pathIdx + ", numEnemies: " + numEnemies);
					
					w.addWaveData(new LevelWaveData(type, spawnDelay, relSpawnTime, pathIdx, numEnemies));
				}               
				waveData[wid] = w;
			}
			
			discoverPaths();
            discoverTowers();
			
            // Configure the level using the new data.
            configLevel();			
		}
		
        public function configLevel():void {
            // Create map
            createTowerBuildSites(); // Generate the towers for the map based on the level data.
            drawPaths(); // This will only happen if Constants.isDebugMode == true.
            
            // Create hud
            hud = new HUD( this );
            addToUILayer( hud ); // Hud should be added last.
            hud.init(); // addChild before init() so it has stage ref.
            
            _abilityStrikeCooldown = GameData.getAbilityCooldown( GameData.ABILITY_STRIKE );
            _abilityDropshipCooldoown = GameData.getAbilityCooldown( GameData.ABILITY_DROPSHIP );
            
            // Init Player Stats
            waveIdx = 0;
            livesRemaining = MAX_PLAYER_LIVES;
            
            // Start music
            SoundSystem.playSound(SoundSystem.SOUND_MUSIC, "BGM_InGame.mp3", true);
            
            paused = false;
            
            // Notify the System that we've completed loading and that the loading screen can be closed.
            dispatchEvent( new LoadingEvent( LoadingEvent.LOAD_COMPLETE ) );
        }
        
        // Loads the data for the level.
//        public function loadLevelData():void {}
        
        public function getActiveEnemies():Array        { return enemies; }
        public function getActiveTowers():Array         { return towers; }
        public function getActiveBuildSites():Array     { return towerSites; }
        
        public function get waveIdx():uint { return _waveIdx; }
        public function set waveIdx( val:uint ):void {
            _waveIdx = val;
            dispatchEvent( new LevelEvent( LevelEvent.WAVE_CHANGE, true, true, _gold, livesRemaining, waveIdx ) );
        }
        
        public function get gold():Number { return _gold; }
        public function set gold( val:Number ):void {
            _gold = val;
            dispatchEvent( new LevelEvent( LevelEvent.GOLD_CHANGE, true, true, _gold, livesRemaining, waveIdx ) );
        }
        
        public function get livesRemaining():int { return _livesRemaining; }
        public function set livesRemaining( val:int ):void {
            _livesRemaining = val;
            if ( _livesRemaining < 0 ) { return; }
            dispatchEvent( new LevelEvent( LevelEvent.LIVES_CHANGE, true, true, _gold, livesRemaining, waveIdx ) );
        }
        
        public function tick():void {
            if ( paused ) { return; }
            
            levelTimer += Constants.FRAME_DELTA;
            
            var lm:uint = lmode % 3;
            
            // Tick towers. This pretty much always need to happen for respawning Infantry, idle states, etc...
            for (var i:uint = 0; i < towers.length; i++) {
				if(towers[i]!=null)
					towers[i].tick( lm );
                //cleanupEnemies();
            }                        
            
            // PPS: This should be a linked list..
            //trace("\nTick : " + levelTimer);
            for (var ei:uint = 0; ei < enemies.length; ei++) {
                //trace(ei + ") " + enemies[ ei ]);
				if(enemies[ei]!=null)
					enemies[ ei ].tick( lm );
                //cleanupEnemies();                
            }
            //trace("-------------");
            
			if (_activeAbilities != null)
			{
				var al:uint = _activeAbilities.length;
				for (var ai:uint = 0; ai < al; ai++) {
					if(_activeAbilities[ai]!=null)
						_activeAbilities[ ai ].tick( lm );
					//cleanupEnemies();
				}
			}

            lmode++;

            // Spawn enemies if possible
            if (phase == PHASE_WAVE) {
                var wslen:int = waveSpawn.length;
                if (_spawnIdx < wslen) {
                    var spawning:Boolean = true;
                    while ((_spawnIdx < wslen) && spawning) {
                        var spawn:Spawn = waveSpawn[_spawnIdx];
                        if (spawn.time <= levelTimer) {
                            //trace("spawning " + _spawnIdx + "..");
                            spawnEnemy(spawn);
                            _spawnIdx++;
                        } else {
                            spawning = false;
                        }
                    }
                } else {
                    _nextPhaseStartTrigger = true;
                    phase = PHASE_INTERMISSION;
                }
                                
            } 
            else if (phase == PHASE_INTERMISSION) {                
                
                if (_nextPhaseStartTrigger) {
                    if (_waveIdx < waveData.length) {
                        //trace("NEXT WAVE TRIGGERED " + _waveIdx);                        
                        _nextPhaseTime = levelTimer + waveData[_waveIdx].startTime;
                    }
                    _nextPhaseStartTrigger = false;
                }
                
                // Check for next wave time
                if (_nextPhaseTime <= levelTimer) {
                    alertPlayerOfWave(_waveIdx);
                    _nextPhaseStartTrigger = true;
                    phase = PHASE_COUNTDOWN;
                }
            }
            else if (phase == PHASE_COUNTDOWN) {
                
                if (_nextPhaseStartTrigger) {
                    if (_waveIdx < waveData.length) {
                        _nextPhaseTime = levelTimer + COUNTDOWN_DURATION;    // 10s is animation length of wave countdown
                    }
                    _nextPhaseStartTrigger = false;
                }
                
                var waveCountdown:Number = Math.ceil( _nextPhaseTime - levelTimer );
                if ( waveCountdown != _lastWaveCountDown ) {
                    dispatchEvent( new WaveEvent( WaveEvent.WAVE_COUNTDOWN, true, true, _waveIdx, waveCountdown ) );
                    _lastWaveCountDown = waveCountdown;
                }
                
                // Check for next wave time
                if (_nextPhaseTime <= levelTimer) {
                    beginWave(_waveIdx);
                }
            }
                
            if ( phase == PHASE_START && !_alertedPlayerOfFirstWave && hud != null && levelTimer > 3 ) {                
                alertPlayerOfWave( 0 );
                _alertedPlayerOfFirstWave = true;
            }
            else if ( phase == PHASE_END && !_levelComplete ) {
                handleLevelComplete();
                _waveIdx = 0;                
            }
            
            if ( !_abilityStrikeAvailable && _nextAbilityStrikeTime < levelTimer ) {
                dispatchEvent( new AbilityEvent( AbilityEvent.READY, true, true, GameData.ABILITY_STRIKE ) );
                _abilityStrikeAvailable = true;
            }
            if ( !_abilityDropshipAvailable && _nextAbilityDropshipTime < levelTimer ) {
                dispatchEvent( new AbilityEvent( AbilityEvent.READY, true, true, GameData.ABILITY_DROPSHIP ) );
                _abilityDropshipAvailable = true;
            }            
            
            if (phase != PHASE_COUNTDOWN && enemies.length == 0 && _waveIdx == waveData.length) {
                phase = PHASE_END;
            }            
            
            // Every 12 seconds, z-sort everything on the stage.
            // @TODO: This can definitely be optimized by moving all MovableEntities on to a single layer.
            /*
            if ( _zSortTime < levelTimer ) {
                // printMainLayer();    
                zsort();
                _zSortTime = levelTimer + _zSortDelay;
            }
            */
        }
        
         public function spawnEnemy(data:Spawn):void {
            // Spawn a new enmy based on the Spawner's current settings.
            var enemyClass:Class = null;
            switch ( data.waveDefn.enemyType ) {
                case GameData.ENEMY_INFANTRY:
                    enemyClass = EnemyInfantry;
                    break;
                case GameData.ENEMY_ARMORED_INFANTRY:
                    enemyClass = EnemyArmoredInfantry;
                    break;
                case GameData.ENEMY_DRONE:
                    enemyClass = EnemyDrone;
                    break;
                case GameData.ENEMY_MECH:
                    enemyClass = EnemyMech;
                    break;
                case GameData.ENEMY_TANK:
                    enemyClass = EnemyTank;
                    break;
				case GameData.ENEMY_TEST:
					enemyClass = TestEnemy;
					break;
                default:
                    throw( new Error("Error :: Level.as, Enemy class associated with type " + data.waveDefn.enemyType + " not found!") );
                    break;
            }
            
            // Pick a random lane for the new enemy.
            var randLaneIdx:uint = Utils.generateRandomBetween( 0, paths[0].length );
            
            var enem:Enemy = new enemyClass( this, data.waveDefn.pathIdx, randLaneIdx );
            enemies.push( enem );
            
            // Graphically add the new enemy to the stage.
            addToMainLayer( enem );
            zsort();
        }
        
        protected function handleLevelComplete():void {
            if ( _livesRemaining > 0 ) {
                _starScore = 1;
                if ( _livesRemaining == MAX_PLAYER_LIVES ) { _starScore = 3; }
                else if ( _livesRemaining >= (MAX_PLAYER_LIVES / 2) ) { _starScore = 2; }
                dispatchEvent( new LevelEvent( LevelEvent.VICTORY, true, true, _gold, _livesRemaining, _waveIdx, _starScore ) );
                
                // Writes the data out to the SharedObject file.
                GameData.setMissionNumStars( GameData.getCurrentMissionId(), _starScore );
            }
            else { 
                dispatchEvent( new LevelEvent( LevelEvent.DEFEAT, true, true, _gold, _livesRemaining, _waveIdx, _starScore ) ) ;
            }
            _levelComplete = true;
            PlayerAchievements.saveAchievementProgress();
        }
        
        protected function printMainLayer():void {
            trace(" >> printMainLayer()! ");
            var numChild:uint = layer5.numChildren;
            for (var i:uint = 0; i < numChild; i++ ) { 
                var mc:DisplayObject = layer5.getChildAt(i);
                trace("\t [" + i + "]: " + mc);
            }
        }
        
        protected function alertPlayerOfWave( idx:uint ) {
            dispatchEvent( new WaveEvent( WaveEvent.WAVE_ALERT, true, true, idx, 10, pathCoords[0], 0, 0, null, waveData[ idx ], _waveSourceDirection) ); // pathCoords[0] is always the starting location for now.
        }
        protected function handleForceWaveEvent( we:WaveEvent ):void {
            // Process early start cash bonus
            var bonus:Number = Math.round(((_nextPhaseTime - levelTimer) / COUNTDOWN_DURATION) * waveData[_waveIdx].startBonus);
            gold += bonus;
            
            beginWave(_waveIdx);
        }
        
        protected function beginWave(waveIdx:uint):void {
            // Populate spawn list
            if (_spawnIdx < waveSpawn.length) {
                throw new Error("Next wave started before the previous wave had spawned completely!");
            }
            waveSpawn.length = 0;
            _spawnIdx = 0;
            
            //trace("starting wave " + waveIdx);
            var wave:Vector.<LevelWaveData> = waveData[waveIdx].data;
            for (var i:uint = 0; i < wave.length; i++) {
                //trace("processing wave data " + i);
                var data:LevelWaveData = wave[i];
                var accumSpawnTime:Number = levelTimer + data.spawnDelay;
                for (var j:uint = 0; j < data.numEnemies; j++) {
                    //trace("creating spawn entry " + j);
                    var spawn:Spawn = new Spawn;
                    spawn.waveDefn = data;
                    spawn.time = accumSpawnTime;
                    accumSpawnTime = accumSpawnTime + data.relSpawnTime;                    
                    waveSpawn.push(spawn);
                }
            }
            
            // Sort spawn array by time
            waveSpawn.sort(spawnSortCompFn);
            
            // Wait until all spawns for current wave has been processed
            _nextPhaseTime = Number.MAX_VALUE;

            dispatchEvent( new WaveEvent( WaveEvent.WAVE_LAUNCH, true, true, waveIdx, 0, null, 0, 0, null, waveData[ waveIdx ] ) );
            
            phase = PHASE_WAVE;
            
            // This is really the next wave index..
            _waveIdx++;            
        }
        
        private function spawnSortCompFn(a:Spawn, b:Spawn):int
        {
            var d:Number = a.time - b.time;
            if (d < 0) return -1;
            else if (d > 0) return 1;
            return 0;
        }

        
        protected function handleAbilityUsedEvent( ae:AbilityEvent ):void {
            useAbility( ae.abilityId, ae.ax, ae.ay );
        }        
        protected function useAbility( abilityId:uint, ax:Number, ay:Number ):void {
            switch ( abilityId ) {
                case GameData.ABILITY_STRIKE:
                    useAbilityStrike( ax, ay );
                    break;
                case GameData.ABILITY_DROPSHIP:
                    useAbilityDropship( ax, ay );
                    break;
                default:
                    break;
            }
        }
        protected function useAbilityStrike( ax:Number, ay:Number ):void {
            var abilityStrike:AbilityStrike = new AbilityStrike( this );
            addChild( abilityStrike );
            abilityStrike.start( ax, ay );
            _nextAbilityStrikeTime = levelTimer + _abilityStrikeCooldown;
            _abilityStrikeAvailable = false;
            
            abilityStrike.addEventListener(Event.COMPLETE, handleAbilityCompleted, false, 0, true);
            _activeAbilities.push(abilityStrike);
        }
        protected function useAbilityDropship( ax:Number, ay:Number ):void {
            var abilityDropship:AbilityDropship = new AbilityDropship( this );
            addChild( abilityDropship );
            abilityDropship.start( ax, ay );
            _nextAbilityDropshipTime = levelTimer + _abilityDropshipCooldoown;
            _abilityDropshipAvailable = false;
            
            abilityDropship.addEventListener(Event.COMPLETE, handleAbilityCompleted, false, 0, true);
            _activeAbilities.push(abilityDropship);
        }      
        protected function handleAbilityCompleted(e:Event):void {
            var ability:Object = e.target;
            Utils.removeFromArray(_activeAbilities, ability);
        }

        protected function handleTowerAttack( e:AttackEvent ):void {
            var enemy:Enemy = e.attackTarget as Enemy;
            if ( enemy != null ) {
                enemy.handleAttackEvent( e );
            }
        }
        
        protected function handleUnitAttack( e:AttackEvent ):void {
            var target:MoveEntity = e.attackTarget as MoveEntity;
            if ( target != null ) {
                target.handleAttackEvent( e );
            }
        }
        
        protected function handleAllyDeathEvent( e:EnemyEvent ):void {
            // @TODO: Attempt at fixing broken guys on stage.
            /*
            var ally:MoveEntity = e.target as MoveEntity;
            if ( layer5.contains( ally ) { 
                removeChild( ally );
            }
            */
        }
        
        protected function handleEnemyDeathEvent( e:EnemyEvent ):void {
            var enem:Enemy = e.target as Enemy;
            cleanupEnemy( enem );
            var gv:Number = e.goldValue; 
            gold += gv;
            zsort();
            
            PlayerAchievements.setAchievementProgress( PlayerAchievements.ACHV_STRATEGIST, PlayerAchievements.getAchievementProgress( PlayerAchievements.ACHV_STRATEGIST ) + 1 );
            PlayerAchievements.setAchievementProgress( PlayerAchievements.ACHV_TACTICIAN, PlayerAchievements.getAchievementProgress( PlayerAchievements.ACHV_TACTICIAN ) + 1 );
            PlayerAchievements.saveAchievementProgress();
        }
        
        protected function handleEnemyReachedDestination( e:EnemyEvent ):void {
            var enem:Enemy = e.target as Enemy;
            cleanupEnemy( enem );
            
            livesRemaining -= enem.playerDamage;
            if (livesRemaining < 0) livesRemaining = 0;
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "BaseDamage.mp3");
			
            if ( _livesRemaining == 0 ) { 
                phase = PHASE_END;
            }
        }
        
        protected function cleanupEnemy(e:Enemy):void {
            // Remove enemy from enemies array.
            var idx:uint = enemies.indexOf( e );
            //trace("CLEANING UP IDX (" + idx + ")");
            enemies.splice( idx, 1 );
            removeFromMainLayer( e );
        
            // Also change their coordinates so Towers can't find them and drop ref.
            e.x = -100; e.y = -100; 
        }
        
        protected function createTowerBuildSites():void {
            for ( var i:uint = 0; i < towerCoords.length; i++ ) {
                var tbs:TowerBuildSite = new TowerBuildSite( this );
                var tpoint:Point = towerCoords[i] as Point;
                tbs.x = tpoint.x;
                tbs.y = tpoint.y;
                addToMainLayer( tbs );
                towerSites.push( tbs );
            }
        }
        
        public function upgradeTower( tower:Tower ):void {
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "TowerUpgrade.mp3");	
			
            var upgradeCost:Number = tower.getUpgradeCost();
            gold -= upgradeCost;
            
            tower.upgrade();
            dispatchEvent( new TowerEvent( TowerEvent.TOWER_UPGRADED, true, true, tower, null ) );
        }
        
        public function upgradeTowerEffect( tower:Tower ):void {
            var upgradeCost:Number = GameData.getTowerEffectUpgradeCost( tower.type, tower.grade + 1 );
            gold -= upgradeCost;
            tower.upgradeEffect();
        }
        
        public function buildTower( tbs:TowerBuildSite, towerType:uint ):void {
            var idx:int = towerSites.indexOf( tbs );
            var tower:Tower;
            var cost:uint = 0;
            
            // Setup the new tower.
            switch ( towerType ) {
                case GameData.TOWER_TYPE_APC:
                    tower = new TowerAPC( this );
                    break;
                case GameData.TOWER_TYPE_MECH:
                    tower = new TowerMech( this );
                    break;
                case GameData.TOWER_TYPE_TANK:
                    tower = new TowerTank( this );
                    break;
                case GameData.TOWER_TYPE_OST:
                    tower = new TowerOrbital( this );
                    break;
                
                default:
                    break;
            }
            
            cost =  GameData.getTowerUpgradeCost( towerType, 0 );
            
            tower.x = tbs.x;
            tower.y = tbs.y;
            
            gold -= cost;
            dispatchEvent( new TowerEvent( TowerEvent.TOWER_BUILT, true, true, tower, tbs ) );
            
            var fxBeam:FxBeam = new FxBeam();
            fxBeam.x = tbs.x;
            fxBeam.y = tbs.y;
            addToFXLayer( fxBeam );
            fxBeam.gotoAndPlay("open");
            
            SoundSystem.playSound(SoundSystem.SOUND_FX, "UnitTeleport.mp3");
			
            tower.alpha = 0;
            
            var t:Tween = new Tween( 2000, tower, { alpha:1 }, { data:{fx:fxBeam}, onComplete:onTowerBuildTweenComplete } );
            
            addToMainLayer( tower );
            towers.push( tower );
            
            // Clean up the build site:
            removeFromMainLayer( tbs );
            towerSites.splice( idx, 1 );
        }
        
        protected function onTowerBuildTweenComplete( t:Tween ):void {
            t.data.fx.gotoAndPlay("close");
            t.target.isAlive = true;
        }
        
        public function destroyTower( tower:Tower ):void {
            var idx:int = towers.indexOf( tower );
            
            // Setup the new tower.
            var tbs:TowerBuildSite = new TowerBuildSite( this );
            tbs.x = tower.x;
            tbs.y = tower.y;
            
            gold += tower.getRefundValue();
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "UnitSell.mp3");
			
            dispatchEvent( new TowerEvent( TowerEvent.TOWER_DESTROYED, true, true, tower, tbs ) );
            
            // Clean up the build site:
            removeFromMainLayer( tower );
            tower.cleanup();
            towers.splice( idx, 1 );
            
            addToMainLayer( tbs );
            towerSites.push( tbs );
        }
        
        protected function drawPaths():void {
            if (Constants.isDebugMode) {
                // trace( this + " :: drawPaths(), pathCoords.length: " + pathCoords.length );
                if (paths.length == 0) { return; }				
                var lineGraphic:Sprite = new Sprite();
				lineGraphic.graphics.lineStyle( 3, 0, .5);				
				for (var pidx:uint = 0; pidx < paths.length; pidx++) {
					var pc:Array = paths[pidx][0];
					lineGraphic.graphics.moveTo( pc[0].x, pc[0].y );
					for (var i:uint = 1; i < pc.length; i++) {
						lineGraphic.graphics.lineTo( pc[i].x, pc[i].y  );
					}
				}
                addChild( lineGraphic );
            }
        }
        
        protected function discoverTowers():void {
            for (var i:uint = 0; i < numChildren; i++) {
                var tower:DisplayObject = getChildByName( ("tower" + i) );
                if ( tower != null ) {
                    tower.visible = false;
                    towerCoords[i] = new Point( tower.x, tower.y );
                }
                else {
                    break;
                }
            }
        }
		
		protected function discoverWaveButtons():void {
            // If the indicator is East (far right) shift it back on if it's offscreen.
            var visibleRect:Rectangle = Extensions.visibleRect;            
			var deltaX:Number = 0;
            if ( Extensions.isScaleform ) {
                if ( _waveSourceDirection == 1 ) {
                    if ( visibleRect.width < 1280 ) { // 1280x720 is the default resolution.
                        deltaX = 1280 - visibleRect.width - visibleRect.x;
                    }
                }
            }            
			for (var i:uint = 0; i < numChildren; i++) {
                var waveBtn:DisplayObject = getChildByName( ("waveBtn" + i) );
                if ( waveBtn != null ) {
                    waveBtn.x -= deltaX;
					addToUILayer(waveBtn);
                    waveBtns.push(waveBtn);
                }
                else {
                    break;
                }
            }
            
            // We piggyback this method to configure the base end point mc
            var is4x3:Boolean = ( visibleRect.width < 1280 );
            end16x9.visible = !is4x3;
            end4x3.visible = is4x3;
		}		
        
        protected function discoverPaths():void {
            
            // For now, there will only be one path in the array: path0.
            // The index holds an array of Point arrays, one for each lane.
            paths = [ ];
            
			for (var pathIdx:uint = 0; pathIdx < pathDefns.length; pathIdx++) {
				var _lane0Coords:Array = [];
				var _lane1Coords:Array = [];
				var _lane2Coords:Array = [];
                var _lane3Coords:Array = [];
				var _lane4Coords:Array = [];
				
				var parr:Array = pathDefns[pathIdx];
				var plen:Number = parr.length;
				for (var i:uint = 0; i < plen; i++) {
					var pmark:DisplayObject = parr[i];
					if ( !Constants.isDebugMode ) { pmark.visible = false; } // Hide these for the real game.
					pathCoords.push(new Point( pmark.x, pmark.y ));
					_lane0Coords.push( new Point( pmark.x, pmark.y ) );
					
					var nmark:DisplayObject = (i + 1) == plen ? null : parr[i + 1];
					var direction:Number = 0;
					if ( nmark != null ) {
						direction = Utils.calcDirectionCoords( pmark.x, pmark.y, nmark.x, nmark.y );
						
					}
					var lg:LaneGenerator = new LaneGenerator();
						
					lg.x = pmark.x;
					lg.y = pmark.y;
					lg.rotation = direction;
					
					var l1:Point = new Point( lg.m0.x, lg.m0.y );
					var gL1:Point = lg.localToGlobal( l1 );
					_lane1Coords.push( new Point( gL1.x, gL1.y ) );
					
					var l2:Point = new Point( lg.m1.x, lg.m1.y );
					var gL2:Point = lg.localToGlobal( l2 );
					_lane2Coords.push( new Point( gL2.x, gL2.y ) );
                    
                    var l3:Point = new Point( lg.m2.x, lg.m2.y );
					var gL3:Point = lg.localToGlobal( l3 );
					_lane3Coords.push( new Point( gL3.x, gL3.y ) );
                    
                    var l4:Point = new Point( lg.m3.x, lg.m3.y );
					var gL4:Point = lg.localToGlobal( l4 );
					_lane4Coords.push( new Point( gL4.x, gL4.y ) );
					
					if ( Constants.isDebugMode ) { addChild( lg ); }
					else { 
						lg = null;
					}
				}
				
				paths[ pathIdx ] = [ _lane0Coords, _lane1Coords, _lane2Coords, _lane3Coords, _lane4Coords ];
			}
			
			discoverWaveButtons();
        }
        
        // UI Layer -- layer10
        public function addToUILayer( s:DisplayObject, depth:int = -1 ):void {
            if ( depth > -1 ) { 
                layer10.addChildAt( s, depth );
            }
            else {
                layer10.addChild( s );
            }
        }
        public function removeFromUILayer( s:DisplayObject ):void {
            layer10.removeChild( s );
        }
        
        // FX Layer -- layer7
        public function addToFXLayer( s:DisplayObject, depth:int = -1 ):void {
            if ( depth > -1 ) { 
                layer7.addChildAt( s, depth );
            }
            else {
                layer7.addChild( s );
            }
        }
        public function removeFromFXLayer( s:DisplayObject ):void {
            // trace(" Level :: removeFromFxLayer(), " + s );
            layer7.removeChild( s );
        }
        
        // Entity Layer (Primary) -- layer5
        public function addToMainLayer( s:DisplayObject, depth:int = -1 ):void {
            if ( depth > -1 ) { 
                layer5.addChildAt( s, depth );
            }
            else {
                layer5.addChild( s );
            }
        }
        public function removeFromMainLayer( s:DisplayObject ):void {
            layer5.removeChild( s );
        }
        
        // Bottom Layer, just above Background -- layer1
        public function addToBottomLayer( s:DisplayObject, depth:int = -1 ):void {
            if ( depth > -1 ) { 
                layer1.addChildAt( s, depth );
            }
            else {
                layer1.addChild( s );
            }
        }
        public function removeFromBottomLayer( s:DisplayObject ):void {
            if ( layer1.contains( s ) ) {
                layer1.removeChild( s );
            }
        }
        
        protected function handleFxAnimationComplete( fxe:FxEvent ):void {
            var fx:FxBase = fxe.target as FxBase;
            var fxLayer:uint = fxe.layer;
            switch ( fxLayer ) {
                case FxEvent.LAYER_FX:
                    removeFromFXLayer( fx );
                    break;
                case FxEvent.LAYER_BOTTOM:
                    removeFromBottomLayer( fx );
                    break;
                default:
                    throw( new Error( "Error :: Level.handleFxAnimationComplete(), Completed Fx's layer index is undefined! Unable to remove from stage. " ) );
                    break;
            }
            // trace(" Level :: handleFxAnimationComplete(), fx: " + fx );
        }
        
        protected function handlePauseGame( se:SystemEvent ):void {
            paused = true;
            var el:uint = enemies.length;
            for (var ei:uint = 0; ei < el; ei++) {
                enemies[ ei ].notifyPaused( true );
            }
            var al:uint = _activeAbilities.length;            
            for (var ai:uint = 0; ai < al; ai++) {
                _activeAbilities[ ai ].notifyPaused( true );
            }
        }
        protected function handleResumeGame( se:SystemEvent ):void {
            paused = false;
            for (var ei:uint = 0; ei < enemies.length; ei++) {
                enemies[ ei ].notifyPaused( false );
            }
            var al:uint = _activeAbilities.length;            
            for (var ai:uint = 0; ai < al; ai++) {
                _activeAbilities[ ai ].notifyPaused( false );
            }
        }
        
        protected function zSortHandler( e:Event ):void {
            zsort();
        }
        
        protected function zsort():void {
            var sortArr:Array = [];
            var arrLength:uint = layer5.numChildren;
            for ( var j:uint = 0; j < arrLength; j++ ) { 
                // Copy the array.
                var mc:Entity = layer5.getChildAt( j ) as Entity;
                if ( mc != null ) { // Only need to add entities.
                    sortArr.push( mc );
                }
            }
            
            sortArr.sort( sortOnY ); // Sort the array by the y coordinate of all Entities.
            
            var length:uint = sortArr.length;
            for ( var k:uint = 0; k < length; k++ ) { 
                // Readd the children to the stage in the correct order.
                var ent:Entity = sortArr[ k ];
                layer5.addChildAt( sortArr[ k ], layer5.numChildren - 1 );
            } 
        }
        
        protected function sortOnY( a:Entity, b:Entity ):Number {
            var ay:Number = a.y;
            var by:Number = b.y;
            
            if (ay < by) { return -1; } 
            else if (ay > by) { return 1; } 
            else { return 0; }
        }
        
    }

}
import com.scaleform.std.data.levels.LevelWaveData;

final class Spawn {
    public var waveDefn:LevelWaveData;
    public var time:Number;
}
