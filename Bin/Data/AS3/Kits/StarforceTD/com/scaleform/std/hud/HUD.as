/**************************************************************************

Filename    :   HUD.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.hud
{
    import com.scaleform.std.data.levels.LevelWave;
    import flash.display.Bitmap;
    import flash.display.BitmapData;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.events.TextEvent;
    import flash.events.TouchEvent;
    import flash.geom.Point;
    import flash.geom.Rectangle;
    import flash.text.TextField;
	import flash.utils.Timer;
	import flash.events.TimerEvent;
    
    import com.scaleform.std.levels.Level;
    import com.scaleform.std.controls.SBButton;
    import com.scaleform.std.entities.Entity;
    import com.scaleform.std.entities.Tower;
    import com.scaleform.std.entities.TowerBuildSite;
    import com.scaleform.std.events.LevelEvent;
    import com.scaleform.std.events.TowerEvent;
    import com.scaleform.std.events.WaveEvent;
    import com.scaleform.std.utils.Constants;
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.controls.WaveButtonBag;
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.hud.VictoryDisplay;
    import com.scaleform.std.hud.SelectionInfo;
    import com.scaleform.std.controls.AbilityButton;
    import com.scaleform.std.controls.WaveButton;
    import com.scaleform.std.events.AbilityEvent;
    import com.scaleform.std.events.MapEvent;
    import com.scaleform.std.events.SystemEvent;
    
    import com.scaleform.std.events.GameEvent;
    
    import scaleform.clik.events.ButtonEvent;
    import com.scaleform.std.events.HUDEvent;
    import scaleform.clik.motion.Tween;
    
	import com.scaleform.std.events.EnemyEvent;
	
    import scaleform.gfx.Extensions;
	
	import fl.transitions.easing.*;
    
    public class HUD extends MovieClip
    {
        public static const LABEL_ANIMATION_OFF:String = "off";
        public static const LABEL_ANIMATION_OPEN:String = "open";
        public static const LABEL_ANIMATION_CLOSE:String = "close";
        
        protected var _level:Level = null;
        
        public var tfGold:TextField;
        public var tfTime:TextField;
        public var tfWave:TextField;
        public var tfLives:TextField;
        
        protected var _tweenStat1:Tween;
        protected var _tweenStat2:Tween;
        
        protected var _focusedTower:Entity = null; // Reference to the focused Tower or TowerBuildSite.
        
        protected var _upgradeUI:TowerUpgradeUI = null; // Reference to the Upgrade UI, if open, for the _focusedTower.
        protected var _showingUpgradeUI:Boolean = false; // true if we're showing the upgrade interface. false otherwise.
        
        //protected var _waveLaunchButtonBag:WaveButtonBag = null;
        //protected var _waveLaunchButton:WaveButton = null; // Reference to the active Launch button.
        
        protected var _tapClickInUse:Boolean = false;
        protected var _abilityIdInUse:int = -1;
        protected var _abilityBtnInUseRef:AbilityButton = null;
        
        protected var _pauseDisplay:PauseDisplay;
		        
        public var HudTop:HUDTop;
        public var HudBottom:MovieClip;
        public var PowerButton1:AbilityButton;
        public var PowerButton2:AbilityButton;
        
        public var BaseHealthText:TextField; // 87/100
        public var BaseHealth:MovieClip; // GotoAndStop %
        public var CreditsText:TextField; // 12,400
        
        public var WaveText:TextField; // 3/12
        public var WaveLabel:TextField; // WAVE
        
        public var SecLabel:TextField;
        public var NextWaveNum:TextField;
        public var NextWaveLabel:TextField;
        
        public var Stat1:TowerStatDisplay;
        public var Stat2:TowerStatDisplay;
        
        protected var _primarySelectionInfo:SelectionInfo = null;
        protected var _secondarySelectionInfo:SelectionInfo = null;
        
        protected var _waveEnemyIcons:Array = null;
        protected var _waveEnemyIconsPool:Array = null;
		protected var _waveIconIdx:uint;
		protected var _waveIconDur:Number;		
		protected var _waveIconTween:Tween;
        
        protected var _touchIndicator:RallyIndicator = null;
        
        public function HUD( level:Level ) {
            super();
            _level = level;
            
            PowerButton1 = HudBottom.PowerButton1;
            PowerButton2 = HudBottom.PowerButton2;
            
            BaseHealthText = HudBottom.BaseHealthText;
            BaseHealth = HudBottom.BaseHealth;
            CreditsText = HudBottom.CreditsText;
            WaveText = HudBottom.WaveText;
            WaveLabel = HudBottom.WaveLabel;
            SecLabel = HudBottom.SecLabel;
            NextWaveNum = HudBottom.NextWaveNum;
            NextWaveLabel = HudBottom.NextWaveLabel;
            
            Stat1 = HudBottom.Stat1;
            Stat2 = HudBottom.Stat2;
            
            Stat1.alpha = 0;
            Stat2.alpha = 0;
        }
        
        public function init():void {
            trace( this + " :: init()!");
            
            _touchIndicator = new RallyIndicator();
            
            CreditsText.htmlText = "<b>" + String( _level.gold ) + "</b>";
            //WaveText.text = String( _level.waveIdx );
            
            gotoAndPlay( LABEL_ANIMATION_OPEN );
            
            // General mouse / click listener for canceling.
            if ( Constants.isTouchEnabled() ) { 
                stage.addEventListener( TouchEvent.TOUCH_TAP, handleStageClick, false, 0, true );
            }
            else {
                stage.addEventListener( MouseEvent.CLICK, handleStageClick, false, 0, true );
            }
            
            // @TODO: Set data should take care of icon.
            PowerButton1.data = GameData.ABILITY_STRIKE;
            PowerButton1.icon.gotoAndPlay( AbilityButton.ICON_STRIKE );
            PowerButton1.addEventListener( ButtonEvent.CLICK, handleUserAbilityClick, false, 0, true );
            
            PowerButton2.data = GameData.ABILITY_DROPSHIP;
            PowerButton2.icon.gotoAndPlay( AbilityButton.ICON_DROPSHIP );
            PowerButton2.addEventListener( ButtonEvent.CLICK, handleUserAbilityClick, false, 0, true );
            
            HudTop.addEventListener( HUDEvent.OPEN_EXIT_MENU, handleOpenPauseMenu, false, 0, true );
            
            // Events that keep the HUD up to date on active towers/sites.
            _level.addEventListener( TowerEvent.TOWER_BUILT, handleTowerBuilt, false, 0, true );
            _level.addEventListener( TowerEvent.TOWER_DESTROYED, handleTowerDestroyed, false, 0, true );
            
            _level.addEventListener( LevelEvent.WAVE_CHANGE, handleWaveChange, false, 0, true );
            _level.addEventListener( LevelEvent.LIVES_CHANGE, handleLivesChange, false, 0, true );
            _level.addEventListener( LevelEvent.GOLD_CHANGE, handleGoldChange, false, 0, true );
            
            _level.addEventListener( LevelEvent.VICTORY, handleVictoryEvent, false, 0, true );
            _level.addEventListener( LevelEvent.DEFEAT, handleDefeatEvent, false, 0, true );
            
            _level.addEventListener( WaveEvent.WAVE_ALERT, handleWaveAlert, false, 0, true );
            _level.addEventListener( WaveEvent.WAVE_LAUNCH, handleWaveLaunched, false, 0, true );
            _level.addEventListener( WaveEvent.WAVE_COUNTDOWN, handleWaveCountdown, false, 0, true );
            
            _level.addEventListener( AbilityEvent.READY, handleAbilityReady, false, 0, true );
            _level.addEventListener( AbilityEvent.USED, handleAbilityUsed, false, 0, true );
            
            _level.addEventListener( SystemEvent.PAUSE_GAME, handlePauseGame, false, 0, true );
            _level.addEventListener( SystemEvent.RESUME_GAME, handleResumeGame, false, 0, true );
            
			stage.addEventListener( EnemyEvent.ENEMY_REACHED_DESTINATION, handleEnemyReachedDestination, false, 0, true );
			
			NextWaveNum.visible = false;
			SecLabel.visible = false;
			
            setupBuildSiteListeners();
        }
        
		protected function handleEnemyReachedDestination( e:EnemyEvent ):void {
            HudBottom.HudBottomBar.gotoAndPlay("on");
        }
		
        protected function handleOpenPauseMenu( he:HUDEvent ):void {
            if ( _pauseDisplay == null ) { 
                _pauseDisplay = new PauseDisplay();
            }
            _pauseDisplay.addEventListener( Event.COMPLETE, handlePauseMenuClose, false, 0, true );
            _pauseDisplay.x = (stage.stageWidth / 2) + ((1280 - stage.stageWidth) / 2);
            _pauseDisplay.y = (stage.stageHeight / 2) + ((720 - stage.stageHeight) / 2);
            _level.addChild( _pauseDisplay ); // Add it on top of everything.
			
            _pauseDisplay.open("hud");			
        }
        
        protected function handlePauseMenuClose( e:Event ):void {
            _pauseDisplay.removeEventListener( Event.COMPLETE, handlePauseMenuClose );
            _level.removeChild( _pauseDisplay );
            dispatchEvent( new SystemEvent( SystemEvent.RESUME_GAME ) );
        }
        
        protected function handlePauseGame(e:Event):void {
            PowerButton1.paused = true;
            PowerButton2.paused = true;
        }
        protected function handleResumeGame(e:Event):void {
            PowerButton1.paused = false;
            PowerButton2.paused = false;
        }
        
        protected function handleWaveCountdown( we:WaveEvent ):void {
            var secsUntilWave:Number = we.timeTilWave;
            NextWaveNum.text = String( secsUntilWave );
            // SecLabel.visible = true;
        }
        
        protected function handleAbilityUsed( ae:AbilityEvent ):void {
            var aId:uint = ae.abilityId;
            switch ( aId ) {
                case GameData.ABILITY_STRIKE:
                    PowerButton1.setCooldownTime( GameData.getAbilityCooldown( aId ) );
                    break;
                case GameData.ABILITY_DROPSHIP:
                    PowerButton2.setCooldownTime( GameData.getAbilityCooldown( aId ) );
                    break;
                default:
                    break;
            }
        }
        
        protected function handleVictoryEvent( le:LevelEvent ):void {
            var vd:VictoryDisplay = new VictoryDisplay();
            vd.mouseEnabled = true;
            vd.setData( le.stars );
            
            vd.x = (stage.stageWidth / 2) + ((1280 - stage.stageWidth) / 2);
            vd.y = (stage.stageHeight / 2) + ((720 - stage.stageHeight) / 2);            
            
            // Add it on top of everything
            _level.addChild( vd );
			
			var t:Timer = new Timer(2000, 1);
			t.addEventListener(TimerEvent.TIMER, function(e:TimerEvent) { vd.open(); } );
			t.start();
//            vd.open();			
        }
        protected function handleDefeatEvent( le:LevelEvent ):void {
            var dd:DefeatDisplay = new DefeatDisplay();
            dd.mouseEnabled = true;
            
            dd.x = (stage.stageWidth / 2) + ((1280 - stage.stageWidth) / 2);
            dd.y = (stage.stageHeight / 2) + ((720 - stage.stageHeight) / 2);            
            
            // Add it on top of everything
            _level.addChild( dd );
			
			var t:Timer = new Timer(2000, 1);
			t.addEventListener(TimerEvent.TIMER, function(e:TimerEvent) { dd.open(); } );
			t.start();
//            dd.open();			
        }
        
        protected function handleWaveChange( e:LevelEvent ):void {
            wave = e.wave;
        }
        protected function handleLivesChange( e:LevelEvent ):void {
            lives = e.lives;
        }
        protected function handleGoldChange( e:LevelEvent ):void {
            gold = e.gold;
        }
        
        public function set wave( val:uint ):void {
            WaveText.htmlText = "<b>" + String( val ) + "/" + String ( _level.waveData.length ) + "</b>";
        }
        public function set gold( val:Number ):void {
            CreditsText.htmlText = "<b> " + String( val ) + "</b>";
        }
        public function set time( val:Number ):void {
            if ( val >= 0 ) {
                tfTime.visible = true;
                var min:String = String( Math.floor(val / 60) );
                var sec:String = (val % 60 >= 10) ? "" + (uint)(val % 60) : "0" + (uint)(val % 60);
                tfTime.text = "Next Wave: " +  min + ":" + sec;
            } else {
                tfTime.visible = false;
            }
        }
        public function set lives( val:uint ):void {
            var maxLives:uint = Level.MAX_PLAYER_LIVES;
            var percBaseHealth:Number = (val / maxLives)
            BaseHealth.gotoAndStop( ( 100 - percBaseHealth * 100 ) );
            BaseHealthText.htmlText = "<b>" + String( val ) + "/" + maxLives + "</b>";
        }
        
        protected function handleAbilityReady( ae:AbilityEvent ):void {
            var abilityId:uint = ae.abilityId;
            switch (abilityId) {
                case GameData.ABILITY_STRIKE:
                    PowerButton1.enabled = true;
                    break;
                case GameData.ABILITY_DROPSHIP:
                    PowerButton2.enabled = true;
                    break;
                default:
                    break;
            }
        }
        
        protected function handleUserAbilityClick( be:ButtonEvent ):void {
			if (_abilityBtnInUseRef != null) {
				cleanupAbilityLaunchStart();
			}
			
            var btn:AbilityButton = be.target as AbilityButton;
			btn.selected = true;
            _abilityBtnInUseRef = btn;
            _abilityIdInUse = uint( btn.data );
            startUseAbility();
        }
        protected function startUseAbility():void {
            _tapClickInUse = true;
            var eventType:String = Constants.isTouchEnabled() ? TouchEvent.TOUCH_BEGIN : MouseEvent.MOUSE_DOWN;
            //stage.addEventListener( eventType, handleAbilityLaunch, false, 0, true );
            _level.abilityTouchCanvas.addEventListener( eventType, handleAbilityLaunch, false, 0, true );
            _level.abilityCanvasEnabled = true;
            _abilityBtnInUseRef.addEventListener( ButtonEvent.CLICK, cancelAbilityLaunch, false, 0, true );
            // @TODO: AbilityButton should display a Cancel state.
        }
        protected function handleAbilityLaunch( e:Event ):void {
            // Since the initial release (TAP, CLICK) will arrive frmo the Button we just clicked, ignore it.
            if ( e.target == _abilityBtnInUseRef ) { return; }
			
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
            
            // Implementations for the User Abilites
            var success:Boolean = false;
            switch ( _abilityIdInUse ) {
                case GameData.ABILITY_STRIKE:
                    success = _level.nav.hitTestPoint( ax, ay, true );
                    if (success) {
                        _abilityBtnInUseRef.enabled = false; // @TODO: AbilityButton should display a cooldown.
                        dispatchEvent( new AbilityEvent( AbilityEvent.USED, true, true, _abilityIdInUse, ax, ay ) ); 
                        cleanupAbilityLaunchStart();
                    } 
                    break;
                case GameData.ABILITY_DROPSHIP:
                    success = _level.nav.hitTestPoint( ax, ay, true );
                    if (success) {
                        _abilityBtnInUseRef.enabled = false; // @TODO: AbilityButton should display a cooldown.
                        dispatchEvent( new AbilityEvent( AbilityEvent.USED, true, true, _abilityIdInUse, ax, ay ) ); 
                        cleanupAbilityLaunchStart();
                    }
                    break;
                default:
                    break;
            }
            
            _touchIndicator.x = ax;
            _touchIndicator.y = ay;
            _touchIndicator.gotoAndPlay( success ? "rallyOff" : "dont" );
            _level.addToBottomLayer( _touchIndicator );
        }
        protected function cleanupAbilityLaunchStart():void {
            var eventType:String = Constants.isTouchEnabled() ? TouchEvent.TOUCH_BEGIN : MouseEvent.MOUSE_DOWN;
            //stage.removeEventListener( eventType, handleAbilityLaunch, false );
            _level.abilityTouchCanvas.removeEventListener( eventType, handleAbilityLaunch, false );
            _level.abilityCanvasEnabled = false;
            _abilityBtnInUseRef.removeEventListener( ButtonEvent.CLICK, cancelAbilityLaunch, false );
            
			_abilityBtnInUseRef.selected = false;
			
            _abilityIdInUse = -1;
            _abilityBtnInUseRef = null;
            _tapClickInUse = false;
        }
        protected function cancelAbilityLaunch( e:Event ):void {
            // @TODO: AbilityButton should return to it's Okay state.
            cleanupAbilityLaunchStart();
        }
        
        protected function setupEnemyWaveIcons( levelWave:LevelWave ):void {
            if ( levelWave != null ) {
                if ( _waveEnemyIcons == null ) { 
                    _waveEnemyIcons = []; 
                }
                if ( _waveEnemyIconsPool == null ) { 
                    _waveEnemyIconsPool = []; 
                }
                
                var numEnemies:Object = {};
                var numEnemySubWaves:uint = levelWave.data.length;
                for (var i:uint = 0; i < numEnemySubWaves; i++) {
                    var enemyType:uint = levelWave.data[i].enemyType;
                    var subWaveNumEnemies:uint = levelWave.data[i].numEnemies;
                    trace("adding " + subWaveNumEnemies + " to " + enemyType + " type");
                    if (numEnemies[enemyType] == undefined)
                        numEnemies[enemyType] = subWaveNumEnemies;
                    else
                        numEnemies[enemyType] += subWaveNumEnemies;
                }
                
                var idx:uint = 0;
                for (var a in numEnemies) {
                    var eIcon:WaveEnemyIcon;
                    if ( _waveEnemyIconsPool.length >= 1 ) { 
                        eIcon = _waveEnemyIconsPool.shift();
                    }
                    else {
                        eIcon = new WaveEnemyIcon();
                    }
                    
                    eIcon.alpha = 0;
                    eIcon.scaleX = 0; 
                    eIcon.scaleY = 0;
                    eIcon.x = -250 - (idx * 30);
                    eIcon.y = -60;
                    idx++;
                    trace("created icon for type " + a + " (num = " + numEnemies[a] + ")");
                    eIcon.setValues( a, numEnemies[a] );
                    HudBottom.addChild( eIcon );
                    _waveEnemyIcons.push( eIcon );
                }
            }
        }
        protected function cleanupEnemyWaveIcons():void {
            if ( _waveEnemyIcons != null ) {
                var numIcons:uint = _waveEnemyIcons.length;
                while ( _waveEnemyIcons.length >= 1 ) {
                    var eIcon:WaveEnemyIcon = _waveEnemyIcons.shift();
                    HudBottom.removeChild( eIcon );
                    _waveEnemyIconsPool.push( eIcon );
                }
            }
        }
        
        // Logic for displaying the Wave timer / force Wave button.
        protected function handleWaveAlert( we:WaveEvent ):void {
				
			var notFirstWave:Boolean = ( we.waveIdx != 0 );
                
			NextWaveNum.text = String( Math.floor( we.timeTilWave ) );
			NextWaveNum.visible = notFirstWave;
			SecLabel.visible = notFirstWave;
			
			setupEnemyWaveIcons( we.levelWave );
			
			_waveIconIdx = 0;
			_waveIconDur = (we.timeTilWave * 1000) / 6;
			//startWaveIconFadeIn();
            startWaveIconsFadeIn();
            
            showWaveLaunchButton( we.waveIdx, we.timeTilWave, we.waveCoords, we.direction );
        }
        protected function startWaveIconsFadeIn():void {
            var wlen:int = _waveEnemyIcons.length;
            for (var i:int = 0; i < wlen; i++) {
                var icon:WaveEnemyIcon = _waveEnemyIcons[i];
                var t:Tween = new Tween(1000, icon, { alpha:1, scaleX:1, scaleY:1 }, { ease:Strong.easeIn } );
            }
        }
        /*
		protected function startWaveIconFadeIn():void {
			var icon:WaveEnemyIcon = _waveEnemyIcons[_waveIconIdx];
			icon.scaleX = 0; icon.scaleY = 0; icon.alpha = 0;
			_waveIconTween = new Tween(500, icon, { alpha:1, scaleX:1, scaleY:1 }, { ease:Back.easeIn, onComplete:onWaveIconFadeInTweenComplete } );
		}
		protected function onWaveIconFadeInTweenComplete( t:Tween ):void {
			var icon:WaveEnemyIcon = _waveEnemyIcons[_waveIconIdx];
			var nx:Number = -250 - (_waveIconIdx * 30);
			var da:Number = ( -450 - nx ) / ( -450 - (-250) );
			_waveIconTween = new Tween( _waveIconDur * da, icon, { x: nx }, { onComplete:onWaveIconHorizTweenComplete } );
		}
        protected function onWaveIconHorizTweenComplete( t:Tween ):void {						
			_waveIconIdx++
			if (_waveIconIdx < _waveEnemyIcons.length) {
				startWaveIconFadeIn();
			}
        }
        */
        protected function onWaveIconFadeOutTweenComplete( t:Tween ):void {
            cleanupEnemyWaveIcons();
        }
        protected function handleWaveLaunched( we:WaveEvent ):void {
			
			if (_waveIconTween != null) {
				_waveIconTween.paused = true;
			}
			if (_waveEnemyIcons != null) {
				for (var i:uint = 0; i < _waveEnemyIcons.length; i++) {
					var icon:WaveEnemyIcon = _waveEnemyIcons[i];
					var t:Tween = new Tween( 1000, icon, { alpha: 0, scaleX:0, scaleY:0 }, { ease:Strong.easeOut, onComplete:(i == 0 ? onWaveIconFadeOutTweenComplete : null), delay:500 } );
				}
			}
			
            hideWaveLaunchButton( we.waveIdx );
            
            wave = we.waveIdx + 1;
            NextWaveNum.visible = false;
            SecLabel.visible = false;
        }
        protected function showWaveLaunchButton( waveIdx:uint, timeTilWave:Number, waveCoords:Point, direction:uint ):void {
			trace("showWaveLaunchButton : " + waveIdx + ", " + timeTilWave + ", " + waveCoords + ", " + direction);
			
			for (var i:uint = 0; i < _level.waveBtns.length; i++) {
				var waveBtnBag:WaveButtonBag = (_level.waveBtns[i] as WaveButtonBag);
				var waveBtn:WaveButton = waveBtnBag.WaveButton as WaveButton;
				waveBtn.enabled = true;
				waveBtn.data = waveIdx;
				waveBtnBag.gotoAndPlay("open");
				waveBtn.WaveButtonProgress.gotoAndPlay( (waveIdx == 0) ? "wait" : "start" );
				waveBtn.addEventListener( ButtonEvent.CLICK, handleWaveLaunchButtonClick, false, 0, true );
				waveBtnBag.mouseChildren = waveBtnBag.mouseEnabled = true;
			}
        }
        protected function hideWaveLaunchButton( waveIdx:uint ):void {
				for (var i:uint = 0; i < _level.waveBtns.length; i++) {
					var waveBtnBag:WaveButtonBag = (_level.waveBtns[i] as WaveButtonBag);
					var waveBtn:WaveButton = waveBtnBag.WaveButton as WaveButton;	
					waveBtnBag.gotoAndPlay("close");
					waveBtn.removeEventListener( ButtonEvent.CLICK, handleWaveLaunchButtonClick, false );
					waveBtn.enabled = false;
					waveBtnBag.mouseChildren = waveBtnBag.mouseEnabled = false;
				}
        }
        protected function handleWaveLaunchButtonClick( be:ButtonEvent ):void {
			var waveIdx:uint = uint( be.target.data );
            hideWaveLaunchButton( waveIdx );
            
            // Launch a WaveEvent.FORCE_WAVE
            dispatchEvent( new WaveEvent( WaveEvent.FORCE_WAVE, true, true, waveIdx ) );
        }
        
        protected function setupBuildSiteListeners():void {
            // Setup the initial listeners on active tower build sites.
            var sites:Array = _level.getActiveBuildSites();
            for (var i:uint = 0; i < sites.length; i++) {
                var site:TowerBuildSite = sites[i] as TowerBuildSite;
                if ( Constants.isTouchEnabled() ) { 
                    site.addEventListener( TouchEvent.TOUCH_TAP, handleTowerClick, false, 0, true );
                }
                else {
                    site.addEventListener( MouseEvent.CLICK, handleTowerClick, false, 0, true );
                }
            }
        }
        
        protected function handleTowerBuilt( e:TowerEvent ):void {
            var tower:Tower = e.tower;
            if ( Constants.isTouchEnabled() ) { 
                tower.addEventListener( TouchEvent.TOUCH_TAP, handleTowerClick, false, 0, true );
            }
            else {
                tower.addEventListener( MouseEvent.CLICK, handleTowerClick, false, 0, true );
            }
        }
        
        protected function handleTowerDestroyed( e:TowerEvent ):void {
            var site:TowerBuildSite = e.site;
            if ( Constants.isTouchEnabled() ) { 
                site.addEventListener( TouchEvent.TOUCH_TAP, handleTowerClick, false, 0, true );
            }
            else {
                site.addEventListener( MouseEvent.CLICK, handleTowerClick, false, 0, true );
            }
        }
        
        protected function handleTowerClick( e:Event ):void {
            if ( _tapClickInUse ) { return; } 
            
            // If we're clicking on the same tower, do nothing.
            if ( e.target == _focusedTower ) { 
                return;
            }
            
            // If we're clicking on a new tower but we're showing a UI already, clean up the existing one.
            if ( _showingUpgradeUI && e.target != _focusedTower ) { 
                closeUpgradeUI();
            }
            
            // Create the new tower upgrade UI.
            _showingUpgradeUI = true;
            
            _focusedTower = e.target as Entity;
            _upgradeUI = new TowerUpgradeUI( _focusedTower, _level );
            _upgradeUI.addEventListener( Event.COMPLETE, handleUpgradeUIComplete, false, 0, true );
            
            _upgradeUI.x = _focusedTower.x;
            _upgradeUI.y = _focusedTower.y;
            _level.addToUILayer( _upgradeUI );
            
            _upgradeUI.open();
            updateTowerStats();
            
            showSelectionInfo( _focusedTower as Tower );
        }
        
        protected function updateTowerStats():void {
            if ( _tweenStat1 != null ) { 
                _tweenStat1.paused = true;
                _tweenStat1 = null;
            }
            
            if ( _tweenStat2 != null ) { 
                _tweenStat2.paused = true;
                _tweenStat2 = null;
            }
            
            if ( _focusedTower is Tower ) { 
                _tweenStat1 = new Tween( 500, Stat1, { alpha:0 }, { onComplete:onStatFadeOutComplete } );
                _tweenStat2 = new Tween( 500, Stat2, { alpha:0 }, { onComplete:onStatFadeOutComplete } );
            }
            else if ( Stat1.alpha != 0 ) {
                _tweenStat1 = new Tween( 500, Stat1, { alpha:0 } );
                _tweenStat2 = new Tween( 500, Stat2, { alpha:0 } );
            }
        }
        protected function onStatFadeOutComplete( t:Tween ):void {			
            var tsd:TowerStatDisplay = t.target as TowerStatDisplay;
            var isStat1:Boolean = tsd == Stat1;
            var towerRef:Tower = _focusedTower as Tower;
            var tType:Number = towerRef.type;
            var tGrade:Number = isStat1 ? towerRef.grade : towerRef.grade + 1;
            var dmgString:String; 
            var spdString:String;
            var rngString:String;
            var spcString:String;
            dmgString = GameData.getTowerMinDamage( tType, tGrade ) + "-" + GameData.getTowerMaxDamage( tType, tGrade );
            spdString = String( GameData.getTowerAttackSpeed( tType, tGrade ) );
            rngString = String( GameData.getTowerRange( tType, tGrade ) );
            spcString = "";
            tsd.setData( tType, tGrade, dmgString, spdString, rngString, spcString );
            
            if ( isStat1 ) {
                _tweenStat1 = new Tween( 250, Stat1, { alpha:1 }, { onComplete:onStatFadeInComplete } );
            }
            else {
				Stat2.visible = (tGrade <= 2);
                _tweenStat2 = new Tween( 250, Stat2, { alpha:1 }, { onComplete:onStatFadeInComplete } );
            }
        }
        protected function onStatFadeInComplete( t:Tween ):void {
            
        }
        
        protected function showSelectionInfo( t:Tower ):void {
            return;
            
            var upgradeAvailable:Boolean = t.grade < 2;
            
            var type:uint = t.type;
            var grade:uint = t.grade;
            var minDamage:Number = t.minDamage;
            var maxDamage:Number = t.maxDamage;
            var attackSpeed:Number = t.attackSpeed;
            var effectType:uint = t.effectType;
            var effectValue:Number = t.effectValue;
            
            _primarySelectionInfo.setData( type, grade, minDamage, maxDamage, attackSpeed, effectType, effectValue );
            _primarySelectionInfo.visible = true;
            
            if ( upgradeAvailable ) {
                grade++;
                minDamage =     GameData.getTowerMinDamage( type, grade );
                maxDamage=      GameData.getTowerMaxDamage( type, grade );
                attackSpeed =   GameData.getTowerAttackSpeed( type, grade );
                effectType =    GameData.getTowerEffectType( type, grade );
                effectValue =   GameData.getTowerEffectValue( type, grade );
            
                _secondarySelectionInfo.setData( type, grade, minDamage, maxDamage, attackSpeed, effectType, effectValue );
                _secondarySelectionInfo.visible = true;
            }
        }
        
        protected function hideSelectionInfo():void {
            return; 
            _primarySelectionInfo.visible = false;
            _secondarySelectionInfo.visible = false;
        }
        
        protected function handleStageClick( e:Event ):void {
            if ( _tapClickInUse ) { return; }
            if ( _showingUpgradeUI && e.target != _focusedTower && e.target != _upgradeUI ) {
                closeUpgradeUI();
                updateTowerStats();
                hideSelectionInfo();
            }
        }
        
        protected function handleUpgradeUIComplete( e:Event ):void {
            closeUpgradeUI();
            updateTowerStats();
        }
        
        protected function closeUpgradeUI():void {
            _upgradeUI.close();
            _upgradeUI = null;
            _focusedTower = null;
            _showingUpgradeUI = false;
        }
    
    }

}