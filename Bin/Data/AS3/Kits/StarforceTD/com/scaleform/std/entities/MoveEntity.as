/**************************************************************************

Filename    :   MoveEntity.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.entities
{
    import com.scaleform.std.events.AttackEvent;
    import com.scaleform.std.events.EnemyEvent;
    import com.scaleform.std.levels.Level;
    import flash.display.MovieClip;
    import flash.display.Sprite;
    import flash.geom.Point;
    import flash.events.Event;
    import fl.motion.easing.Linear;
    
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.hud.EntityHealthBar
    
    import scaleform.clik.motion.Tween;
    
    public class MoveEntity extends Entity
    {
        public var speed:Number         = 0;
        public var maxSpeed:Number      = 0;
        public var attackSpeed:Number   = 0;
        public var maxHealth:Number     = 0;
        public var goldValue:uint       = 0;
        public var range:uint           = 0;
        public var playerDamage:uint    = 0;
        
        protected var _health:Number = 0; // Current health of the Unit
        protected var _target:MoveEntity = null; // Reference to our current target, if valid.
        
        // Pathing data
        protected var _curDest:Point;
        protected var _isMoving:Boolean = false;
        protected var _pathIdx:uint = 1; // Index for the current Level.pathCoord that we're headed to.
        protected var _healthBarOffsetY:Number = -20;
		
		protected var _dx:Number;
		protected var _dy:Number;
		protected var _dsteps:int;
        
        // UI Elements
        protected var _healthBar:MovieClip = null;
        
        // PPS: -180 <-> 180 map with 10deg ranges
        protected static var _flookup:Vector.<uint> = Vector.<uint>([5, 5, 7, 7, 7, 7, 7, 9, 9, 9, 9, 11, 11, 11, 11, 11, 13, 13, 13, 13, 15, 15, 15, 15, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 5, 5]);
        
        public function MoveEntity( level:Level ) {
            super();
            _level = level;
        }

        public function get health():Number { return _health; }
        public function set health( val:Number ):void {
            if ( val > maxHealth ) { val = maxHealth; }
            if ( val < 0 ) { isAlive = false; }
            _health = val;
            updateHealthBarGraphic();
        }
        
        protected function updateHealthBarGraphic():void {
            if ( _healthBar != null ) { 
                _healthBar.gotoAndStop( 100 - Math.floor( _health / maxHealth * 100 ) );    
            }
        }
        
        protected function createHealthBarGraphic():void {
            _healthBar = new EntityHealthBar();
            _healthBar.y = _healthBarOffsetY;
            addChild( _healthBar );
            updateHealthBarGraphic();
        }
        
        override protected function configUI():void {
            super.configUI();
            
            createHealthBarGraphic();
            enabled = false; // Disable the SBButton functionality.
            health = maxHealth; // health setter won't be initialized til now.
        }
        
        public function getTarget():MoveEntity { return _target; }
        
        protected function configStats():void {
            // To be implemented by subclasses.
        }
        
        public function handleAttackEvent( e:AttackEvent ):void {
            // To be implemented by subclasses.
        }
        
        protected function updateDestination():void {
            // To be implemented by subclasses.
        }
        
        protected function playDeathAnimation():void {
            // to be implemented by subclasses.
        }
        
        protected function updateDirection( px:Number, py:Number ):void {
            var frameIdx:uint = 0;
            var angle:Number = Utils.calcDirectionCoords( x, y, px, py );
            // trace("angle: " + angle);

            //*
            // S
            if      ( angle > 60   &&   angle <= 95 )     { frameIdx = 1; }
            // SE
            else if ( angle > 20   &&   angle <= 60 )     { frameIdx = 15; }
            // E
            else if ( angle > -15  &&   angle <= 20 )     { frameIdx = 13; }
            // NE
            else if ( angle > -65  &&   angle <= -15 )     { frameIdx = 11; }
            // N
            else if ( angle > -115 &&   angle <= -65 )     { frameIdx = 9; }
            // NW
            else if ( angle > -160 &&   angle <= -115 )    { frameIdx = 7; }
            // W
            else if ( ( angle >= -180   &&    angle <= -160 ) || 
                      ( angle > 160     &&    angle <= 180  ) ) {
                frameIdx = 5; 
            }
            // SW
            else if ( angle > 95 && angle <= 160 )     { frameIdx = 3; }
            /*/
            var anorm:int = Math.floor((angle / 10) + 17.99);
            frameIdx = _flookup[anorm];
            //*/
            
            _modelDirection.gotoAndPlay( frameIdx );
            updateGraphicRefs();
        }
        
    }

}