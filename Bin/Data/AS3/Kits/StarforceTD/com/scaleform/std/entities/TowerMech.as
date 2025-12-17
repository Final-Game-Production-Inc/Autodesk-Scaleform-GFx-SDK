/**************************************************************************

Filename    :   TowerMech.as

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
    import com.scaleform.std.utils.Constants;
    import com.scaleform.std.fx.FxExplosion;
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.data.towers.TowerData;
    
    import com.scaleform.std.system.SoundSystem;
    
    public class TowerMech extends Tower
    {
        protected var _modelGun:MovieClip;
        protected var _modelBounce:MovieClip;
        protected var _modelLeg:MovieClip;
        
        protected var _currentDirection:int = -1;
        protected var _newDirection:int = -1;
        
        protected var _rotationLocked = false;
        protected var _rotationLockCounter = 0;
        
        public function TowerMech( level:Level = null ) {
            type = GameData.TOWER_TYPE_MECH;
            effectType = TowerData.EFFECT_DOT;
            super( level );
        }
        
        //protected var _playingAttackAnimation:Boolean = false;
        //protected var _attackAnimFrameCounter = 0;
        //protected const ATTACK_ANIM_FRAME_LENGTH = 19;
        
        /*
        protected function trackAttackAnimFrames( e:Event ):void {
            if ( _attackAnimFrameCounter++ >= ATTACK_ANIM_FRAME_LENGTH ) {
                _playingAttackAnimation = false;
                removeEventListener( Event.ENTER_FRAME, trackAttackAnimFrames, false );
            }
            
        }*/
        
        override protected function updateGraphicRefs():void {
            // trace( this + " :: updateGraphicRefs()");
           
            _modelDirection = model;
            _modelState = model.dir;
            
            _modelAttack = model.dir; // Bounce. "attack"
            _modelGun = model.gun;
            _modelLeg = model.Leg;
            
            if ( _modelGun != null && _modelGun.g ) {
                _modelFx1 = _modelGun.g.fx1;
            }
            
            /*
            trace("\t _modelLevel: " + _modelLevel);
            trace("\t _modelState: " + _modelState);
            trace("\t _modelDirection: " + _modelDirection);
            trace("\t _modelAttack: " + _modelAttack);
            trace("\t _modelGun: "  + _modelGun);
            trace("\t _modelLeg: " + _modelLeg);
            trace("\t _modelFx1: " + _modelFx1);
            */
        }
        
        override protected function playAttackAnimation( ent:Entity ):void {
            if ( target == null ) { return; }
            
            updateGraphicRefs();
            
            if ( _modelAttack != null ) {
                _modelAttack.gotoAndPlay( "attack" );
            }
            if ( _modelGun != null ) {
                _modelGun.gotoAndPlay( "attack" );
            }
            if ( _modelFx1 != null ) {
                _modelFx1.gotoAndPlay( "attack" );
            }
            
			if (target != null)
			{
				var fx:MovieClip = new FxExplosion();
				fx.x = target.x;
				fx.y = target.y;
				_level.addToFXLayer( fx );
			}
            
            //_playingAttackAnimation = true;
            //_attackAnimFrameCounter = 0;
            //addEventListener( Event.ENTER_FRAME, trackAttackAnimFrames, false, 0, true );
            
            // @TODO: Move to attack().
            SoundSystem.playSound(SoundSystem.SOUND_FX, "Mech_Attack.mp3");
        }
        
        
        protected function tickRotationLock( e:Event ):void {
            _rotationLockCounter++;
            if ( _rotationLockCounter++ >= 15 ) {
                removeEventListener( Event.ENTER_FRAME, tickRotationLock, false );
                _rotationLocked = false;
            }
        }
        
        
        override protected function updateDirection():void {
            // trace(this + " :: updateDirection()");
            //if ( target == null ) { return; }
            var frameIdx:uint = 0;
            var angle:Number = Utils.calcDirectionEntity( this, target ); //  + 270; // Corrects the offset.
            
            // S
            if  ( angle > 75  &&  angle <= 92 ) { 
                frameIdx = 1; 
                _newDirection = 8;
            }
            // S - SE
            else if ( angle > 47  &&  angle <= 75 ) { 
                frameIdx = 31; 
                _newDirection = 7;
            }
            // SE
            else if ( angle > 15  &&  angle <= 47 ) {
                frameIdx = 29; 
                _newDirection = 6;
            }
            // SE - E
            else if ( angle > 0 && angle <= 15 )  { 
                frameIdx = 27; 
                _newDirection = 5;
            }
            // E
            else if ( angle > -15 && angle <= 0 ) { 
                frameIdx = 25;
                _newDirection = 4;
            }
            // E - NE
            else if ( angle > -38 && angle <= -15 ) { 
                frameIdx = 23;
                _newDirection = 3;
            }
            // NE
            else if ( angle > -58 && angle <= -38 ) {
                frameIdx = 21;
                _newDirection = 2;
            }
            // NE - N
            else if ( angle > -78 && angle <= -58 ) { 
                frameIdx = 19;
                _newDirection = 1;
            }
            // N
            else if ( angle > -103 && angle <= -78 ) {
                frameIdx = 17;
                _newDirection = 0;
            }
            // N - NW
            else if ( angle > -115 && angle <= -103 ) { 
                frameIdx = 15;
                _newDirection = 15;
            }
            // NW
            else if ( angle > -153 && angle <= -115 ) {
                frameIdx = 13;
                _newDirection = 14;
            }
            // NW - W
            else if ( angle > -175 && angle <= -153 ) {
                frameIdx = 11;
                _newDirection = 13;
            }
            // W
            else if ( ( angle >= -180 && angle <= -175 ) || 
                      ( angle > 165   && angle <= 180 ) ) {
                frameIdx = 9; 
                _newDirection = 12;
            }
            // W - SW
            else if ( angle > 140 && angle <= 165 ) { 
                frameIdx = 7; 
                _newDirection = 11;
            }
            // SW
            else if ( angle > 115 && angle <= 140 ) {
                frameIdx = 5; 
                _newDirection = 10;
            }
            // SW -S
            else if ( angle > 92 && angle <= 115 ) {
                frameIdx = 3;
                _newDirection = 9;
            }
            
            // trace(" _newDirection: " + _newDirection );
            // trace(" _currentDiretion: " + _currentDirection );
            
            // If we don't need to update our direction, do nothing.
            if ( _newDirection == _currentDirection ) { 
                return;
            }
            
            var framePrefix:String = "ccw";
            
            // Clockwise
            if ( _currentDirection <= 8 && ( _newDirection <= _currentDirection ) ) {
                framePrefix = "ccw";
            }
            else if ( _currentDirection > 8 && ( _newDirection <= _currentDirection ) ) {
                framePrefix = "ccw";
            }
            
            // Counter Clockwise.
            if ( _currentDirection <= 8 && ( _newDirection >= _currentDirection ) ) {
                framePrefix = "cw";
            }
            else if ( _currentDirection > 8 && ( _newDirection >= _currentDirection ) ) {
                framePrefix = "cw";
            }
            
            if ( _currentDirection == 15 && _newDirection == 0 ) { 
                framePrefix = "cw";
            }
            else if ( _currentDirection == 0 && _newDirection == 15 ) {
                framePrefix = "ccw";
            }
            
            // var framePrefix:String = ( _newDirection > _currentDirection )  ? "cw" : "ccw";
            var legFrame:int = 1;
            if ( framePrefix == "ccw" ) {
                // South - New Direction
                if ( _newDirection <= 8 ) {
                    legFrame = 8 - _newDirection;
                }
                else {
                    switch( _newDirection ) {
                        case 15: 
                            legFrame = 9;
                            break;
                        case 14: 
                            legFrame = 10;
                            break;
                        case 13: 
                            legFrame = 11;
                            break;
                        case 12: 
                            legFrame = 12;
                            break;
                        case 11:
                            legFrame = 13;
                            break;
                        case 10:
                            legFrame = 14;
                            break;
                        case 9:
                            legFrame = 15;
                            break;
                    }
                }
            }
            else {
                switch( _newDirection ) {
                    case 15: 
                        legFrame = 7;
                        break;
                    case 14: 
                        legFrame = 6;
                        break;
                    case 13: 
                        legFrame = 5;
                        break;
                    case 12: 
                        legFrame = 4;
                        break;
                    case 11:
                        legFrame = 3;
                        break;
                    case 10:
                        legFrame = 2;
                        break;
                    case 9:
                        legFrame = 1;
                        break;
                    case 8: 
                        legFrame = 16;
                        break;
                    case 7: 
                        legFrame = 15;
                        break;
                    case 6: 
                        legFrame = 14;
                        break;
                    case 5: 
                        legFrame = 13;
                        break;
                    case 4:
                        legFrame = 12;
                        break;
                    case 3:
                        legFrame = 11;
                        break;
                    case 2:
                        legFrame = 10;
                        break;
                    case 1:
                        legFrame = 9;
                        break;
                    case 0:
                        legFrame = 8;
                        break;
                }
            }
            
            
            // S - 8
            // S-SE - 7
            // ccw: 1
            
            // S-SE - 7
            // SE - 6
            // ccw: 2
            
            // SE - 6
            // SE-E - 5
            // ccw: 3
            
            // SE-E - 5
            // E - 4
            // ccw: 4
            
            // NE-E - 1
            // N - 0
            // ccw: 8
            
            // N - 0
            // N-NW - 16
            // ccw: 9   // 7
            
            // N-NW - 16
            // NW - 15
            // ccw: 10  // 5
            
            // NW - 15
            // NW-W - 14
            // ccw: 11  // 3
            
            // NW-W - 14
            // W - 13
            // ccw: 12  // 1

            // W - 13
            // W-SW - 12
            // ccw: 13  // -1
            
            // N - 0
            // N-NE - 1 
            // cw: 9
            
            
            // trace(" _modelDirection.gotoAndPlay( " + frameIdx + " )");
            _modelDirection.gotoAndPlay( frameIdx );
            
            // Legs
            if ( !_rotationLocked ) {
                var frameName:String = framePrefix + legFrame;
                // trace(" frameName: " + frameName );
                _modelLeg = _modelDirection.Leg;
                _modelLeg.gotoAndPlay( frameName );
            }
            
            if ( legFrame % 4 == 0 ) {
                _rotationLocked = true;
                _rotationLockCounter = 0;
                addEventListener( Event.ENTER_FRAME, tickRotationLock, false, 0, true );
            }
            
            // trace("Updating _newDirection: " + _newDirection );
            _currentDirection = _newDirection;
            updateGraphicRefs(); // @TODO: May be unnecessary.
        }

        
        override public function toString():String {
            return "[TowerMech " + this.name + "]";
        }
    }

}