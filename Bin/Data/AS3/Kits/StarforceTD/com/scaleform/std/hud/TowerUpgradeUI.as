/**************************************************************************

Filename    :   TowerUpgradeUI.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.hud
{
    import flash.display.BitmapData;
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.events.MouseEvent;
    import flash.events.TouchEvent;
    import flash.geom.Point;
    import flash.utils.getDefinitionByName;
    import flash.display.Bitmap
    
    import scaleform.clik.controls.Button;
    import scaleform.clik.events.ButtonEvent;
    
    import com.scaleform.std.controls.SBButton;
    import com.scaleform.std.entities.Entity;
    import com.scaleform.std.entities.Tower;
    import com.scaleform.std.entities.TowerTank;
    import com.scaleform.std.entities.TowerOrbital;
    import com.scaleform.std.entities.TowerMech;
    import com.scaleform.std.entities.TowerAPC;
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.controls.TowerInterfaceButton;
    import com.scaleform.std.utils.Constants;
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.data.towers.TowerAPCData;
    import com.scaleform.std.data.towers.TowerData;
    import com.scaleform.std.events.LevelEvent;
    import com.scaleform.std.entities.TowerBuildSite;
    import com.scaleform.std.levels.Level;

    import com.scaleform.std.system.SoundSystem;
    
    public class TowerUpgradeUI extends MovieClip
    {
    // Static Constants
        public static const ANIM_HIDDEN:String  = "hidden";
        public static const ANIM_SHOWN:String   = "shown";
        public static const ANIM_OPEN:String    = "open";
        public static const ANIM_CLOSE:String   = "close";
        
        public static const BUTTON_ICON_APC:String          = "apc";
        public static const BUTTON_ICON_MECH:String         = "mech";
        public static const BUTTON_ICON_TANK:String         = "tank";
        public static const BUTTON_ICON_ORBITAL:String      = "ost";
        public static const BUTTON_ICON_UPGRADE:String      = "upgrade";
        public static const BUTTON_ICON_EFFECT_UPGRADE:String = "effectUpgrade";
        public static const BUTTON_ICON_REFUND:String       = "sell";
        public static const BUTTON_ICON_RALLY:String        = "rally";
        
        public static const OPTION_UPGRADE:uint = 0;
        public static const OPTION_REFUND:uint  = 1;
        public static const OPTION_RALLY:uint   = 2;
        public static const OPTION_UPGRADE_EFFECT:uint = 3;
        
    // Protected Properties
        protected var _level:Level;
        protected var _target:Entity;
        protected var _tower:Tower;
        protected var _site:TowerBuildSite;
        
        protected var _forBuildSite:Boolean = false;
        protected var _forTower:Boolean = false;
        
        protected var _buttons:Array = null;
        protected var _buttonPool:Array = null;
        
        protected var _rangeIndicator:MovieClip = null; // Reference to the range indicator, if we're displaying one.
        
        protected var _rallyPoint:RallyPoint = null;
        protected var _rallyIndicator:RallyIndicator = null;
        protected var _setRallyBtn:Button = null;
        
        protected var _isSettingRally:Boolean = false;
        protected var _openAnimComplete:Boolean = false;
        
        public var tRing:MovieClip;
        
        // OPTI - Create a Button pool rather than recreating them each time.
        
        public function TowerUpgradeUI( target:Entity, level:Level ) {
            super();
            
            _target = target;
            if ( _target is Tower ) {
                _tower = target as Tower;
                _forTower = true;
            }
            else if ( _target is TowerBuildSite ) {
                _site = target as TowerBuildSite;
                _forBuildSite = true;
            }
            
            _level = level;
            _level.addEventListener( LevelEvent.GOLD_CHANGE, handleLevelGoldChange, false, 0, true );
            _buttons = [];
            _buttonPool = [];
        }
        
        public function open():void {
            _openAnimComplete = false;
            gotoAndPlay( ANIM_OPEN );
            tRing.mouseEnabled = tRing.mouseChildren = false;
            
            createButtons();
            configureButtons();
            reflowLayout();
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "TowerButtonOpen.mp3");
        }
        
        public function close():void {
            _openAnimComplete = false;
            
            if ( !_isSettingRally ) { 
                hideRangeIndicator();
            }
            
            if ( _forTower ) {
                hideTowerExtras();
            }
            
            gotoAndPlay( ANIM_CLOSE );
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "TowerButtonClose.mp3");
        }
        
        protected function onCompleteOpenAnimation():void {
            _openAnimComplete = true;
            tRing.mouseEnabled = tRing.mouseChildren = true;
            
            if ( _forTower ) {
                // Show the range indicator for the tower, scaling it down. 130 is arbitrary approximation of indicator size. May need to change.
                showRangeIndicator( (_tower.range / 100), x, y );  // Stupid indicator isn't a proper circle so radian is jacked.
                showTowerExtras(); // Display extra data related to the tower (eg. rally points).
            }
        }
        
        protected function onCompleteCloseAnim():void {
            cleanupButtons();
            // 
        }
        
        protected function handleLevelGoldChange( le:LevelEvent ):void {
            if ( _buttons.length > 0 ) { 
                if ( _site != null ) {
                    for ( var i:uint = 0; i < _buttons.length; i++ ) { 
                        var btnRef:TowerInterfaceButton = _buttons[i] as TowerInterfaceButton;
                        if ( btnRef.enabled ) { continue; }
                        var tType:uint = uint( btnRef.data );
                        var cost:Number = GameData.getTowerUpgradeCost( tType, 0 )
                        if ( cost <= _level.gold ) { 
                            btnRef.enabled = true;
                        }
                    }
                }
                else if ( _tower != null ) {
                    for ( var j:uint = 0; j < _buttons.length; j++ ) { 
                        var ubtnRef:TowerInterfaceButton = _buttons[j] as TowerInterfaceButton;
                        if ( ubtnRef.enabled ) { continue; }
                        var uType:uint = uint( ubtnRef.data );
                        if ( uType == OPTION_UPGRADE ) {
                            var uCost:Number = _tower.getUpgradeCost();
                            if ( uCost <= _level.gold ) {
                                ubtnRef.enabled = true;
                            }
                        }
                    }
                }
            }
        }
        
        protected function createButtons():void {
            // Building tower...
            if ( _site != null ) {
                
                var towerCost:Number = GameData.getTowerUpgradeCost( GameData.TOWER_TYPE_APC, 0 );
                // Add the four buttons for building now towers.    
                _buttons.push( createButton( 
                                            TowerInterfaceButton, 
                                            GameData.TOWER_TYPE_APC, 
                                            ( _level.gold >= towerCost),
                                            String( towerCost ) ,
                                            BUTTON_ICON_APC
                                          )
                            ); 
                            
                towerCost = GameData.getTowerUpgradeCost( GameData.TOWER_TYPE_MECH, 0 )
                _buttons.push( createButton( 
                                            TowerInterfaceButton, 
                                            GameData.TOWER_TYPE_MECH, 
                                            ( _level.gold >= towerCost ),
                                            String( towerCost ),
                                            BUTTON_ICON_MECH
                                          ) 
                            );
                towerCost = GameData.getTowerUpgradeCost( GameData.TOWER_TYPE_TANK, 0 )
                _buttons.push( createButton( 
                                            TowerInterfaceButton, 
                                            GameData.TOWER_TYPE_TANK, 
                                            ( _level.gold >= towerCost ), 
                                            String( towerCost ),
                                            BUTTON_ICON_TANK
                                          ) 
                            );
                            
                towerCost = GameData.getTowerUpgradeCost( GameData.TOWER_TYPE_OST, 0 )
                _buttons.push( createButton( 
                                            TowerInterfaceButton, 
                                            GameData.TOWER_TYPE_OST, 
                                            ( _level.gold >= towerCost ), 
                                            String( towerCost ) ,
                                            BUTTON_ICON_ORBITAL
                                          )
                            );
            }
            
            // Upgrading tower...
            else if ( _tower != null ) {
                // Can the tower's main grade be upgraded?
                var tgrade:uint = _tower.grade;
                if ( tgrade != TowerData.GRADE_MAX ) { // Don't show the upgrade Button if the tower is at the max grade.
                    var cost:uint = _tower.getUpgradeCost();
                    _buttons.push( createButton( TowerInterfaceButton, OPTION_UPGRADE, ( _level.gold >= cost ), String( cost ), BUTTON_ICON_UPGRADE ) );
                }
                
				/*
                var eGrade:uint = _tower.effectGrade;
                var eType:uint = _tower.effectType;
                if ( eGrade != TowerData.GRADE_EFFECT_MAX && eType != TowerData.EFFECT_NONE ) {
                    var eCost:uint = GameData.getTowerEffectUpgradeCost( _tower.type, eGrade + 1 );
                    _buttons.push( createButton( TowerInterfaceButton, OPTION_UPGRADE_EFFECT, ( _level.gold >= eCost ), String( eCost ), BUTTON_ICON_EFFECT_UPGRADE ) );
                }
				*/
                
                if ( _tower.type == GameData.TOWER_TYPE_APC ) { 
                    _buttons.push( createButton( TowerInterfaceButton, OPTION_RALLY, true, "Rally", BUTTON_ICON_RALLY ) );
                }
                
                _buttons.push( createButton( TowerInterfaceButton, OPTION_REFUND, true, String( _tower.getRefundValue() ), BUTTON_ICON_REFUND ) );
                
                // TODO: Set Rally Point button?
                // TODO: Tower cooldown button?
            }
        }
        
        protected function createButton( buttonClass:Class, data:Object, enabled:Boolean, label:String = "", iconFrameLabel:String = "" ):Button {
            var btn:SBButton;
            if ( _buttonPool.length > 0 ) { 
                for ( var i:uint = 0; i < _buttons.length; i++ ) {
                    if ( _buttonPool[i] is buttonClass ) { 
                        btn = _buttonPool[i];
                        _buttonPool.splice( i, 1 );
                        break;
                    }
                }
            } 
            if ( btn == null ) { // If there was no usable Button in the pool.
                btn = new buttonClass();
            }
            
            btn.data = data;
            btn.enabled = enabled;
            btn.label = label;
            if ( iconFrameLabel != "" ) {
                btn.icon.gotoAndPlay( iconFrameLabel );
            }
            
            return btn;
        }
        
        protected function configureButtons():void {
            for ( var i:uint = 0; i < _buttons.length; i++ ) {
                var btn:Button = _buttons[i] as Button;
                btn.addEventListener( ButtonEvent.CLICK, handleButtonClick, false, 0, true );
                
                // Display the range indicator on a roll over.
                if (Constants.isTouchEnabled()) {
                    btn.addEventListener( TouchEvent.TOUCH_OVER, handleButtonRollOver, false, 0, true );
                    btn.addEventListener( TouchEvent.TOUCH_OUT, handleButtonRollOut, false, 0, true );    
                }
                else {
                    btn.addEventListener( MouseEvent.ROLL_OVER, handleButtonRollOver, false, 0, true );
                    btn.addEventListener( MouseEvent.ROLL_OUT, handleButtonRollOut, false, 0, true );
                }
                
            }
        }
        
        protected function cleanupButtons():void {
            while ( _buttons.length > 0 ) {
                var btn:Button = _buttons.pop() as Button;
                // This should definitely be null at this point, but just in case.
                if ( tRing ) {
                    tRing.removeChild( btn );
                }
                
                btn.removeEventListener( ButtonEvent.CLICK, handleButtonClick, false );
                
                btn.removeEventListener( MouseEvent.ROLL_OVER, handleButtonRollOver, false );
                btn.removeEventListener( MouseEvent.ROLL_OUT, handleButtonRollOut, false );
                
                btn.removeEventListener( TouchEvent.TOUCH_OVER, handleButtonRollOver, false );
                btn.removeEventListener( TouchEvent.TOUCH_OUT, handleButtonRollOut, false );
                
                _buttonPool.push( btn );
            }
        }
        
        protected function handleButtonClick( e:ButtonEvent ):void {
            if ( !_openAnimComplete ) { return; }
            
            var actionCompleted:Boolean = true;
            var data:Object = e.target.data;
            
            // Building a tower...
            if ( _site != null ) {
                var towerType:uint = uint( data );
                _level.buildTower( _site, towerType );
                hideRangeIndicator();
            }
            
            // Modifiying a tower...
            else if ( _tower != null ) {
                var option:uint = uint( data );
                switch (option) {
                    case OPTION_UPGRADE:
                        _level.upgradeTower( _tower );
                        hideRangeIndicator();
                        break;
                    case OPTION_UPGRADE_EFFECT:
                        _level.upgradeTowerEffect( _tower );
                        break;
                    case OPTION_REFUND:
                        _level.destroyTower( _tower );
                        hideRangeIndicator();
                        break;
                    case OPTION_RALLY:
                        _setRallyBtn = e.target as Button;
                        startSetRallyPoint( _tower as TowerAPC );
                        actionCompleted = false;
                        break;
                    default:
                        break;
                }
            }
            
            // Dispatch a Complete event so that the HUD knows to close us.
            if ( actionCompleted ) {
                dispatchEvent( new Event( Event.COMPLETE ) );
            }
        }
        
        protected function handleButtonRollOver( e:Event ):void {
            if ( !_openAnimComplete ) { return; }
            var data:uint = e.target.data;
            var tRange:Number = 0;
            if ( _forBuildSite ) {
                var range:Number = 0;
                switch ( data ) {
                    case GameData.TOWER_TYPE_APC:
                        tRange = TowerAPCData.INFANTRY_SPAWN_RANGE;
                        break;
                    default:
                        tRange = GameData.getTowerRange( data, 0 );
                        break;
                }
            }
            
            if ( _forTower ) {
                if ( data == OPTION_UPGRADE ) {
                    tRange = GameData.getTowerRange( _tower.type, _tower.grade + 1 );
                }
                else if ( data == OPTION_RALLY ) {
                    tRange = _tower.range;
                }
            }
            
            showRangeIndicator( tRange / 100, _target.x, _target.y );
        }
        
        protected function handleButtonRollOut( e:Event ):void {
            if ( !_openAnimComplete ) { return; }
            
            // If it's a build site, hide the range indicator since a build site has no range.
            if ( _forBuildSite ) {
                hideRangeIndicator();
            }
            
            if ( _forTower ) {
                showRangeIndicator( (_tower.range / 100) , _tower.x, _tower.y );
            }
        }
        
        protected function setRangeIndicatorScale( scale:Number ):void {
            _rangeIndicator.scaleX = _rangeIndicator.scaleY = scale;
            _rangeIndicator.x = x - (_rangeIndicator.width / 2);
            _rangeIndicator.y = y - (_rangeIndicator.height / 2);
        }
        
        protected function showRangeIndicator( scale:Number, px:Number, py:Number ):void {
            if ( _rangeIndicator == null ) {
                _rangeIndicator = new RangeIndicator();
                _rangeIndicator.alpha = .65;
                // _rangeIndicator.rotationX = 45;
                _rangeIndicator.height = _rangeIndicator.height - 20
            }
            
            _rangeIndicator.scaleX = _rangeIndicator.scaleY = scale;
            
            _rangeIndicator.x = px - (_rangeIndicator.width / 2);
            _rangeIndicator.y = py - (_rangeIndicator.height / 2);
            
            _level.addToBottomLayer( _rangeIndicator ); // Might cause the indicator to blink?
        }
        protected function hideRangeIndicator():void {
            if ( _rangeIndicator != null ) {
                _level.removeFromBottomLayer( _rangeIndicator );
            }
        }
        
        protected function showTowerExtras():void {
            // Tower APC
            if ( _tower.type == GameData.TOWER_TYPE_APC ) {
                // Show the current rally point.
                var cRally:Point = (_tower as TowerAPC).rallyPoint;
                if ( _rallyPoint == null ) {
                    _rallyPoint = new RallyPoint();
                }
                
                _rallyPoint.x = cRally.x;
                _rallyPoint.y = cRally.y;
                _level.addToFXLayer( _rallyPoint );
            }
        }
        protected function hideTowerExtras():void {
            if ( _tower.type == GameData.TOWER_TYPE_APC ) {
                // Hide the current rally point.
                if ( _rallyPoint != null ) { // @TODO: Is contains faster than RemoveChild? Probably not.
                    _level.removeFromFXLayer( _rallyPoint );
                }
            }
        }
        
        protected function disableButtons():void {
            for (var i:uint = 0; i < _buttons.length; i++) {
                var btn:Button = _buttons[i] as Button;
            }
        }
        
        public function startSetRallyPoint( tower:TowerAPC ):void {
            _isSettingRally = true;
            close();
            
            if (Constants.isTouchEnabled()) {
                stage.addEventListener( TouchEvent.TOUCH_BEGIN, onSetRallyClick, false, 0, true );
            }
            else {
                stage.addEventListener( MouseEvent.MOUSE_DOWN, onSetRallyClick, false, 0, true );
            }
        }
        public function endSetRallyPoint():void {
            stage.removeEventListener( MouseEvent.MOUSE_DOWN, onSetRallyClick, false );
            stage.removeEventListener( TouchEvent.TOUCH_BEGIN, onSetRallyClick, false );
            _isSettingRally = false;
            _setRallyBtn.selected = false;
            _setRallyBtn = null;
            hideRangeIndicator();
        }
        
        protected function onSetRallyClick( e:Event ):void {
            var validPoint:Boolean = false;
            
            var point:Point;
            if (e is MouseEvent) { 
                var me:MouseEvent = e as MouseEvent;
                point = new Point( me.stageX, me.stageY );
            }
            else {
                var te:TouchEvent = e as TouchEvent;
                point = new Point( te.stageX, te.stageY );
            }
            
            var pointOnNav:Boolean = _level.nav.hitTestPoint( point.x, point.y, true );
            if ( pointOnNav ) {
                var dist:Number = Utils.calcDistanceCoords( _tower.x, _tower.y, point.x, point.y );
                if ( dist <= _tower.range ) { 
                    validPoint = true;
                    (_tower as TowerAPC).setRallyPoint( point.x, point.y );
                }
            }
            
            if ( _rallyIndicator == null ) { 
                _rallyIndicator = new RallyIndicator();
            }
            
            _rallyIndicator.x = point.x;
            _rallyIndicator.y = point.y;
            _level.addToFXLayer( _rallyIndicator );
            
            
            if ( validPoint ) {
                _rallyIndicator.gotoAndPlay("rally");
                endSetRallyPoint();
            }
            else {
                _rallyIndicator.gotoAndPlay("dont");
            }
        }
        
        protected function reflowLayout():void {
            var leftX:Number = -68;
            var rightX:Number = 68;
            var topY:Number = -68;
            var bottomY:Number = 68;
            
            var length:uint = _buttons.length;
            var btnWidth:Number = (length > 0) ? _buttons[0].width : 0;
            switch( length )
            {
                case 4:
                    _buttons[0].x = 0;
                    _buttons[0].y = topY;
                    
                    _buttons[1].x = rightX;
                    _buttons[1].y = 0;
                    
                    _buttons[2].x = 0;
                    _buttons[2].y = bottomY;
                    
                    _buttons[3].x = leftX;
                    _buttons[3].y = 0;
                    break;
                
                case 3:
                    _buttons[0].x = 0;
                    _buttons[0].y = topY;
                    
                    _buttons[1].x = leftX;
                    _buttons[1].y = 0;
                    
                    _buttons[2].x = 0;
                    _buttons[2].y = bottomY;
                    break;
                    
                case 2:
                    _buttons[0].x = 0;
                    _buttons[0].y = topY;
                    
                    _buttons[1].x = 0;
                    _buttons[1].y = bottomY;
                    break;
                    
                case 1:
                    _buttons[0].x = 0;
                    _buttons[0].y = bottomY;
                    break;
                    
                default:
                    break;
            }
            
            for ( var i:uint = 0; i < _buttons.length; i++ ) {
                tRing.addChild( _buttons[i] );
            }
        }
        
        protected function onCompleteCloseAnimation():void {
            parent.removeChild( this );
        }
        
    }

}