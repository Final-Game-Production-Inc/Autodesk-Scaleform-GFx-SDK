/**************************************************************************

Filename    :   TowerOrbital.as

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
    import flash.display.CapsStyle;
    import flash.events.Event;
    import flash.geom.Point;
    import flash.text.TextField;
    
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.levels.Level;
    import com.scaleform.std.events.AttackEvent;
    import com.scaleform.std.fx.FxOrbitalStrike;
    import com.scaleform.std.fx.FxOrbitalScar;
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.data.towers.TowerOrbitalData;
    import com.scaleform.std.data.towers.TowerData;
    import com.scaleform.std.fx.FxExplosion;
    
    import com.scaleform.std.system.SoundSystem;
    
    public class TowerOrbital extends Tower
    {
        
    // Protected Properties
        // Splash radius for the OST's attack.
        protected var _splashRadius:Number = 0;
        
    // Graphics
        // Reference to the Orbial Strike FX.
        protected var _fxOrbitalStrike:FxOrbitalStrike = null;
        
    // Initialization
        public function TowerOrbital( level:Level = null ) {
            type = GameData.TOWER_TYPE_OST;
            effectType = TowerData.EFFECT_AMP_DMG;
            super( level );
        }
        
        override protected function configStats():void {
            super.configStats();
            _splashRadius = GameData.getTowerSplashRadius( GameData.TOWER_TYPE_OST, grade );
        }

    // Protected Functions
        // Logic
        override protected function attack( ent:Entity ):void {
            if ( ent == null ) { return; }
            updateGraphics(); // Update the graphics, since we want the attack to be as up to date as possible.
            playAttackAnimation( ent ); // Play the attack animation.
            _nextAttackTime = _level.levelTimer + attackSpeed; // Clock the time so we don't attack again until we can.
        }
        
        protected function applyDamage():void { 
            var enemies:Array = _level.getActiveEnemies();
            var numEnemies:uint = enemies.length;
            for ( var i:uint = 0; i < numEnemies; i++ ) {
                var en:Enemy = enemies[ i ];
                if ( en != null && Utils.calcDistanceEntity( target, en ) < _splashRadius ) {
                    var dmg:Number = Utils.generateRandomBetween( minDamage, maxDamage ) * ( 1 + (Utils.generateRandomBetween( -15, 15 ) / 100 ) );
                    var isCrit:Boolean = Utils.generateRandomBetween(0, 100) < criticalChance;
                    if ( isCrit ) {
                        dmg = dmg * 2; 
                    }
                    dispatchEvent( new AttackEvent( AttackEvent.TOWER_ATTACK, true, true, en, type, dmg, id, effectType, effectValue, effectDuration, effectTickSpeed, isCrit) );
                }
            }
        }
        
        // Graphics
        override protected function updateGraphicRefs():void {
            _modelState = model;
        }
        
        override protected function playAttackAnimation( ent:Entity ):void {
            if ( target == null  ) { return; }
            if ( _fxOrbitalStrike == null ) { 
                _fxOrbitalStrike = new FxOrbitalStrike();
            }
            
            _fxOrbitalStrike.x = target.x;
            _fxOrbitalStrike.y = target.y;
            _level.addToFXLayer( _fxOrbitalStrike );
            _fxOrbitalStrike.gotoAndPlay("attack");
            
            _fxOrbitalStrike.addEventListener( "onStartAnimComplete", onFxStrikeStartAnimComplete, false, 0, true );
            _fxOrbitalStrike.addEventListener( "onAttackAnimComplete", onFxStrikeAttackAnimComplete, false, 0, true );
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "OST_Attack.mp3");
        }
        
        // Fx and Animations
        protected function onFxStrikeStartAnimComplete( e:Event ):void {
            applyDamage();
        }
        
        protected function onFxStrikeAttackAnimComplete( e:Event ):void {
            var fxOrbitalScar:FxOrbitalScar = new FxOrbitalScar();
            fxOrbitalScar.x = _fxOrbitalStrike.x;
            fxOrbitalScar.y = _fxOrbitalStrike.y;
            _level.addToBottomLayer( fxOrbitalScar );
            fxOrbitalScar.gotoAndPlay( "on" );
        }
        
        /** @exclude **/
        override public function toString():String {
            return "[SB TowerOribtal " + this.name + "]";
        }
    }

}