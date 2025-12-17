/**************************************************************************

Filename    :   TowerTank.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.entities
{
    import com.scaleform.std.fx.FxTankExplosion;
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
    import com.scaleform.std.core.GameData;
    import com.scaleform.std.data.towers.TowerData;
    
    import com.scaleform.std.system.SoundSystem;
	
    import com.scaleform.std.fx.FxExplosion;
    
    public class TowerTank extends Tower
    {
        
        //protected var _playingAttackAnimation:Boolean = false;
        //protected var _attackAnimFrameCounter = 0;
        //protected const ATTACK_ANIM_FRAME_LENGTH = 19;
        
        public function TowerTank( level:Level = null ) {
            type = GameData.TOWER_TYPE_TANK;
            effectType = TowerData.EFFECT_SLOW;
            super( level );
        }
        
        /*
        protected function trackAttackAnimFrames( e:Event ):void {
            if ( _attackAnimFrameCounter++ >= ATTACK_ANIM_FRAME_LENGTH ) {
                _playingAttackAnimation = false;
                removeEventListener( Event.ENTER_FRAME, trackAttackAnimFrames, false );
            }
        }
        */
        
        override protected function playAttackAnimation( ent:Entity ):void {
            // Graphics should be up to date by this point ( Tower.attack() );
            if ( _modelAttack != null ) {
                _modelAttack.gotoAndPlay( "attack" );
            }
              
            if ( _modelFx1 != null ) {
                _modelFx1.gotoAndPlay( "attack" );
            }
            if (_modelFx2 != null)  {
                _modelFx2.gotoAndPlay( "attack" );
            }
            if (_modelFx3 != null)  {
                _modelFx3.gotoAndPlay( "attack" );
            }            
            
            var fx:MovieClip = new FxTankExplosion();
            fx.x = target.x;
            fx.y = target.y;
            _level.addToFXLayer( fx );
            
            //_playingAttackAnimation = true;
            //_attackAnimFrameCounter = 0;
            //addEventListener( Event.ENTER_FRAME, trackAttackAnimFrames, false, 0, true );
            
            
            // @TODO: Move to attack().
            SoundSystem.playSound(SoundSystem.SOUND_FX, "Tank_Attack.mp3");
        }
        
        override protected function updateGraphicRefs():void {
            _modelDirection = model;
            _modelState = model.dir;
            _modelAttack = _modelState;
            
            if ( _modelAttack != null && _modelAttack.g ) {
                _modelFx1 = _modelAttack.g.fx1;
                _modelFx2 = _modelAttack.g.fx2;
                _modelFx3 = _modelAttack.g.fx3;
            }
            
            /*
            trace( this + " :: updateGraphicRefs()");
            trace("\t _modelLevel: " + _modelLevel);
            trace("\t _modelState: " + _modelState);
            trace("\t _modelDirection: " + _modelDirection);
            trace("\t _modelAttack: " + _modelAttack);
            trace("\t _modelFx1: "  + _modelFx1);
            trace("\t _modelFx2: " + _modelFx2);
            trace("\t _modelFx3: " + _modelFx3);
            */
        }
        
        override protected function updateGraphics():void {
            var newFrame:String;
            var stateChanged:Boolean = false;
            
            if (_gxGrade != grade) { 
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
                // trace("\t newFrame: " + newFrame);
                stateChanged = true;
                _modelLevel.gotoAndStop( newFrame );
                updateGraphicRefs();
            }
            
            if (  _towerState == STATE_ATTACK ) { 
                // trace( this + " :: updateDirection() ");
                updateGraphicRefs();
                updateDirection();                
            }
            
            if ( stateChanged || _gxState != _towerState ) {
                // trace("\t > " + this + " :: state changed!");
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
        
        override protected function updateDirection():void {
            //if ( _playingAttackAnimation ) { return; }

            var frameIdx:uint = 0;
            
            var angle:Number = Utils.calcDirectionEntity( this, target ); //  + 270; // Corrects the offset.
            // trace(" anAngle: " + angle );
            
            // S        // 80 - 70
            // S-SW     // 70 - 50
            // SW       // 50 - 20
            // SW - E   // 20 - -5
            
            // S
            if      ( angle > 75  &&  angle <= 92 ) { frameIdx = 1;  }
            // S - SE
            else if ( angle > 47  &&  angle <= 75 ) { frameIdx = 31; }
            // SE
            else if ( angle > 15  &&  angle <= 47 ) { frameIdx = 29; }
            // SW - E
            else if ( angle > 0 && angle <= 15 )  { frameIdx = 27; }
            // E
            else if ( angle > -15 && angle <= 0 ) { frameIdx = 25;  }
            // E - NE
            else if ( angle > -38 && angle <= -15 ) { frameIdx = 23;  }
            // NE
            else if ( angle > -58 && angle <= -38 ) { frameIdx = 21;  }
            // NE - N
            else if ( angle > -78 && angle <= -58 ) { frameIdx = 19;  }
            // N
            else if ( angle > -103 && angle <= -78 ) { frameIdx = 17;  }
            // N - NW
            else if ( angle > -115 && angle <= -103 ) { frameIdx = 15;  }
            // NW
            else if ( angle > -153 && angle <= -115 ) { frameIdx = 13;  }
            // NW - W
            else if ( angle > -175 && angle <= -153 ) { frameIdx = 11;  }
            // W
            else if ( ( angle >= -180 && angle <= -175 ) || 
                      ( angle > 165   && angle <= 180 ) ) {
                frameIdx = 9; 
            }
            // W - SW
            else if ( angle > 140 && angle <= 165 )     { frameIdx = 7; }
            // SW
            else if ( angle > 115 && angle <= 140 )     { frameIdx = 5; }
            // SW -S
            else if ( angle > 92 && angle <= 115 )     { frameIdx = 3; }
            
            _modelDirection.gotoAndPlay( frameIdx );
            updateGraphicRefs(); // @TODO: this may be 
            
            /*
            // Update these since our references are now bad.
            if ( _modelDirection.dir != null) {
                _modelFx1 = _modelDirection.dir.fx1;
                _modelFx2 = _modelDirection.dir.fx2;
                _modelFx3 = _modelDirection.dir.fx3;
            }
            */
        }
        
        override public function toString():String {
            return "[TankTower " + this.name + "]";
        }
    }

}