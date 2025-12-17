/**************************************************************************

Filename    :   Entity.as

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

package com.scaleform.std.entities
{
    import flash.display.MovieClip;
    import flash.events.Event;
    import flash.geom.Point;
    
    import com.scaleform.std.controls.SBButton;
    import com.scaleform.std.utils.Utils;
    import com.scaleform.std.levels.Level;

    public class Entity extends SBButton
    {
    // Public Properties
        public var id:uint = 0; // Global unique identifier for this Entity.
        
        // Stats
        public var isAlive:Boolean = true; // true if the Entity's health > 0.
        
        public var grade:uint = 0; // Grade (level) of the unit.        
        public var minDamage:Number = 0; // Minimum damage for the entity.
        public var maxDamage:Number = 0; // Maximum damage for the entity.
        
    // Protected Properties
        // Data
        protected var _level:Level = null; // Reference to the Level that created this tower. Access info about enemies.
        
        // State
        protected var _eState:uint = 0; // The state of the Entity (Idle, Attack, Run, etc...).
        
        // Graphics
        protected var _gxState:uint = 0; // The state that the graphics are currently displaying.
        protected var _gxGrade:uint = 0; // The grade that the graphics are currently displaying.
        
        protected var _modelLevel:MovieClip = null; // Reference to the MovieClip that controls level of the Tower.
        protected var _modelState:MovieClip = null; // Reference to the MovieClip that controls state of the Tower.
        protected var _modelDirection:MovieClip = null; // Reference to the MovieClip that controls direction of the Tower.
        protected var _modelAttack:MovieClip = null; // Reference to the MovieClip that controls the attack animation of the Tower.
        
        public function Entity() {
            super();
            id = Utils.getNewId();
            updateGraphicRefs();
        }
        
        override protected function configUI():void {
            // trace( this + " :: cfgui." );
            super.configUI();
        }
        
        public function get eState():uint { return _eState;  }
        public function set eState( val:uint ):void {
            if ( _eState == val ) { return; }
            _eState = val;
        }
        
        public function tick( lmode:uint ):void {
            // To be implemented by subclasses.
        }
        
        protected function updateGraphics():void {
            // To be implemented by subclasses.
        }
        
        protected function updateGraphicRefs():void {
            // To be implemented by subclasses.
        }
        
    }

}