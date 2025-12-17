/**************************************************************************

Filename    :   AbilityDropship.as

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
    
    import fl.transitions.easing.*;
    
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.entities.Enemy;
    import com.scaleform.std.fx.FxExplosion;
    import com.scaleform.std.levels.Level;
    import com.scaleform.std.data.abilities.AbilityData;
    import com.scaleform.std.events.AttackEvent;
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.entities.AllyInfantry;
    import com.scaleform.std.events.EnemyEvent;
    import com.scaleform.std.entities.DropshipInfantry;
    import com.scaleform.std.events.TowerEvent;
    import com.scaleform.std.fx.FxBeam;
    
    import scaleform.clik.motion.Tween;
    
    import com.scaleform.std.system.SoundSystem;
    
    public class AbilityDropship extends Ability 
    {
    // Public Properties:
        
    // Protected Properties:
        protected var _ally1:DropshipInfantry = null;
        protected var _ally2:DropshipInfantry = null;
        
    // Initialization
        public function AbilityDropship( level:Level ) {
            super( level );
        }
        
        override protected function configStats():void {
            _duration = GameData.getAbilityDuration( GameData.ABILITY_DROPSHIP );
        }
        
        override public function start( ax:Number, ay:Number ):void {
            super.start( ax, ay );
            
            _ally1 = new DropshipInfantry( _level );
            _ally2 = new DropshipInfantry( _level );
            
            _ally1.addEventListener( EnemyEvent.UNIT_DIED, handleInfantryDied, false, 0, true );
            _ally2.addEventListener( EnemyEvent.UNIT_DIED, handleInfantryDied, false, 0, true );
            
            _ally1.addTeammateRef( _ally2 );
            _ally2.addTeammateRef( _ally1 );
            
            _ally1.x = ax - 30;
            _ally2.x = ax + 30;
            
            _ally1.y = _ally2.y = ay;
            
            var fxBeam1:FxBeam = new FxBeam();
            fxBeam1.x = _ally1.x;
            fxBeam1.y = _ally1.y;
            fxBeam1.scaleX = fxBeam1.scaleY = .5;
            _level.addToFXLayer( fxBeam1 );
            fxBeam1.gotoAndPlay("open");
            
            var fxBeam2:FxBeam = new FxBeam();
            fxBeam2.x = _ally2.x;
            fxBeam2.y = _ally2.y;
            fxBeam2.scaleX = fxBeam2.scaleY = .5;
            _level.addToFXLayer( fxBeam2 );
            fxBeam2.gotoAndPlay("open");
            
            _level.addToMainLayer( _ally1 );
            _level.addToMainLayer( _ally2 );
        
            _ally1.alpha = 0;
            _ally2.alpha = 0;
            
            _ally1.isAlive = false;
            _ally2.isAlive = false;
            
            var t1:Tween = new Tween( 1000, _ally1, { alpha:1 }, { data:{ fx:fxBeam1 }, onComplete:onBeamInAnimComplete } );
            var t2:Tween = new Tween( 1000, _ally2, { alpha:1 }, { data: { fx:fxBeam2 }, onComplete:onBeamInAnimComplete } );
			
            SoundSystem.playSound(SoundSystem.SOUND_FX, "UnitTeleport.mp3");
        }
        
        protected function onBeamInAnimComplete( t:Tween ):void {
            t.data.fx.gotoAndPlay("close");
            t.target.isAlive = true;
            t.data = null;
        }
        
        override public function end():void {
            super.end();
            
            if ( _ally1 != null ) { 
                _ally1.removeEventListener( EnemyEvent.UNIT_DIED, handleInfantryDied, false );
                _ally1.dispatchEvent( new EnemyEvent( EnemyEvent.UNIT_DIED ) );
                _ally1.isAlive = false;
                
                var fxBeam1:FxBeam = new FxBeam();
                fxBeam1.x = _ally1.x;
                fxBeam1.y = _ally1.y;
                fxBeam1.scaleX = fxBeam1.scaleY = .5;
                _level.addToFXLayer( fxBeam1 );
                fxBeam1.gotoAndPlay("open");
                
                var t1:Tween = new Tween( 1000, _ally1, { alpha:0 }, { data:{ fx:fxBeam1 }, onComplete:onBeamOutAnimComplete } );
            }
            
            if ( _ally2 != null ) { 
                _ally2.removeEventListener( EnemyEvent.UNIT_DIED, handleInfantryDied, false );
                _ally2.dispatchEvent( new EnemyEvent( EnemyEvent.UNIT_DIED ) );
                _ally2.isAlive = false;
                
                var fxBeam2:FxBeam = new FxBeam();
                fxBeam2.x = _ally2.x;
                fxBeam2.y = _ally2.y;
                fxBeam2.scaleX = fxBeam2.scaleY = .5;
                _level.addToFXLayer( fxBeam2 );
                fxBeam2.gotoAndPlay("open");
                
                var t2:Tween = new Tween( 1000, _ally2, { alpha:0 }, { data:{ fx:fxBeam2 }, onComplete:onBeamOutAnimComplete } );
            }
        }
        
        protected function onBeamOutAnimComplete( t:Tween ):void {
            t.data.fx.gotoAndPlay("close");
            cleanupAlly( t.target as DropshipInfantry, true );
        }
        
        protected function cleanupAlly( ally:DropshipInfantry, dispatchDeathEvent:Boolean ):void {
            if ( ally != null ) {
                ally.removeEventListener( EnemyEvent.UNIT_DIED, handleInfantryDied, false );
                _level.removeFromMainLayer( ally );
                if ( dispatchDeathEvent ) {
                    ally.destroy();
                }
                
                // Drop refs.
                if ( ally == _ally1 ) { _ally1 = null; } 
                else if ( ally == _ally2 ) { _ally2 = null; }
            }
        }
        
        override public function tick( lmode:uint ):void {
            if ( _ally1 != null ) {
                _ally1.tick( lmode );
            }
            if ( _ally2 != null ) {
                _ally2.tick( lmode );
            }

            if (lmode != Level.LMODE_ABILITY) return;
            
            if ( _destroyTime < _level.levelTimer ) {
                end();
                return;
            }
            
        }
        
        protected function handleInfantryDied( ee:EnemyEvent ):void {
            var ally:DropshipInfantry = ee.target as DropshipInfantry;
            cleanupAlly( ally, false );
            if ( ally == _ally1 ) { _ally1 = null; } 
            else if ( ally == _ally2 ) { _ally2 = null; }
        }
        
    }

}